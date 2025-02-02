/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"

World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds)
	:m_target(output_target)
	,m_camera(output_target.getDefaultView())
	,m_textures()
	,m_fonts(font)
	,m_sounds(sounds)
	,m_scenegraph(ReceiverCategories::kNone)
	,m_scene_layers()
	,m_world_bounds(0.f,0.f, m_camera.getSize().x, 3000.f)
	,m_spawn_position(m_camera.getSize().x/2.f, m_world_bounds.height - m_camera.getSize().y/2.f)
	,m_scrollspeed(0.f)
	, m_enemySpawnTimer(sf::Time::Zero)
	, m_enemySpawnInterval(sf::seconds(2.f)) // Initial spawn interval; will be randomized after each spawn.
{
	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);
	// === Create Player 1 ===
	auto player1 = std::make_unique<Aircraft>(AircraftType::kEagle, m_textures, m_fonts);
	player1->SetCategory(ReceiverCategories::kPlayerAircraft);
	player1->setPosition(m_spawn_position.x - 50.f, m_spawn_position.y); // Offset left
	m_player_aircrafts.push_back(player1.get());
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(player1));

	// === Create Player 2 ===
	auto player2 = std::make_unique<Aircraft>(AircraftType::kEagle, m_textures, m_fonts);
	player2->SetCategory(ReceiverCategories::kAlliedAircraft);
	player2->setPosition(m_spawn_position.x + 50.f, m_spawn_position.y); // Offset right
	m_player_aircrafts.push_back(player2.get());
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(player2));
}

void World::Update(sf::Time dt)
{
	//Scroll the world
	m_camera.move(0, m_scrollspeed * dt.asSeconds());
	
	//m_player_aircraft->SetVelocity(0.f, 0.f);
	m_enemySpawnTimer += dt;
	if (m_enemySpawnTimer >= m_enemySpawnInterval)
	{
		SpawnEnemy();
		m_enemySpawnTimer = sf::Time::Zero;
		// Randomize the next spawn interval (for example, between 1.5 and 3 seconds):
		float nextInterval = 1.5f + static_cast<float>(Utility::RandomInt(1500)) / 1000.f; // 1.5 to 3.0 seconds
		m_enemySpawnInterval = sf::seconds(nextInterval);
	}

	DestroyEntitiesOutsideView();
	GuideMissiles();
	GuideEnemies(dt);

	//Forward commands to the scenegraph
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayerVelocity();

	HandleCollisions();
	// Remove dead players from m_player_aircrafts
	m_player_aircrafts.erase(
		std::remove_if(m_player_aircrafts.begin(), m_player_aircrafts.end(),
			[](Aircraft* player) { return player->IsMarkedForRemoval(); }),
		m_player_aircrafts.end());

	m_scenegraph.RemoveWrecks();


	m_scenegraph.Update(dt, m_command_queue);
	AdaptPlayerPosition();
}

void World::Draw()
{
	if (PostEffect::IsSupported())
	{
		m_scene_texture.clear();
		m_scene_texture.setView(m_camera);
		m_scene_texture.draw(m_scenegraph);
		m_scene_texture.display();
		m_bloom_effect.Apply(m_scene_texture, m_target);
	}
	else
	{
		m_target.setView(m_camera);
		m_target.draw(m_scenegraph);
	}
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

bool World::HasAlivePlayer() const
{
	if (!m_player_aircrafts.empty())
	{
		return true;
	}
	else
	{
		return false;
	}

}

//bool World::HasPlayerReachedEnd() const
//{
//	return !m_world_bounds.contains(m_player_aircraft->getPosition());
//}

void World::LoadTextures()
{
	m_textures.Load(TextureID::kEagle, "Media/Textures/Eagle.png");
	m_textures.Load(TextureID::kRaptor, "Media/Textures/Raptor.png");
	m_textures.Load(TextureID::kAvenger, "Media/Textures/Avenger.png");
	m_textures.Load(TextureID::kLandscape, "Media/Textures/Desert.png");
	m_textures.Load(TextureID::kBullet, "Media/Textures/Bullet.png");
	m_textures.Load(TextureID::kMissile, "Media/Textures/Missile.png");

	m_textures.Load(TextureID::kHealthRefill, "Media/Textures/HealthRefill.png");
	m_textures.Load(TextureID::kMissileRefill, "Media/Textures/MissileRefill.png");
	m_textures.Load(TextureID::kFireSpread, "Media/Textures/FireSpread.png");
	m_textures.Load(TextureID::kFireRate, "Media/Textures/FireRate.png");
	m_textures.Load(TextureID::kFinishLine, "Media/Textures/FinishLine.png");

	m_textures.Load(TextureID::kEntities, "Media/Textures/Entities.png");
	m_textures.Load(TextureID::kJungle, "Media/Textures/Jungle.png");
	m_textures.Load(TextureID::kExplosion, "Media/Textures/Explosion.png");
	m_textures.Load(TextureID::kParticle, "Media/Textures/Particle.png");

	//Added by Albert
	m_textures.Load(TextureID::kInvincibility, "Media/Textures/Invincibility.png");
	//Added by Albert
	m_textures.Load(TextureID::kMinigun, "Media/Textures/Minigun.png");


}

void World::BuildScene()
{
	//Initialize the different layers
	for (std::size_t i = 0; i < static_cast<int>(SceneLayers::kLayerCount); ++i)
	{
		ReceiverCategories category = (i == static_cast<int>(SceneLayers::kLowerAir)) ? ReceiverCategories::kScene : ReceiverCategories::kNone;
		SceneNode::Ptr layer(new SceneNode(category));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& texture = m_textures.Get(TextureID::kJungle);
	sf::IntRect textureRect(m_world_bounds);
	texture.setRepeated(true);

	//Add the background sprite to the world
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(m_world_bounds.left, m_world_bounds.top);
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(background_sprite));

	//Add the finish line
	sf::Texture& finish_texture = m_textures.Get(TextureID::kFinishLine);
	std::unique_ptr<SpriteNode> finish_sprite(new SpriteNode(finish_texture));
	finish_sprite->setPosition(0.f, -76.f);
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(finish_sprite));

	////Add the player's aircraft
	//std::unique_ptr<Aircraft> leader(new Aircraft(AircraftType::kEagle, m_textures, m_fonts));
	//m_player_aircraft = leader.get();
	//m_player_aircraft->setPosition(m_spawn_position);
	//m_player_aircraft->SetVelocity(40.f, m_scrollspeed);
	//m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(leader));

	//Add the particle nodes to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(ParticleType::kSmoke, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(smokeNode));

	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(ParticleType::kPropellant, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(propellantNode));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(m_sounds));
	m_scenegraph.AttachChild(std::move(soundNode));

	//AddEnemies();

	/*std::unique_ptr<Aircraft> left_escort(new Aircraft(AircraftType::kRaptor, m_textures, m_fonts));
	left_escort->setPosition(-80.f, 50.f);
	m_player_aircraft->AttachChild(std::move(left_escort));

	std::unique_ptr<Aircraft> right_escort(new Aircraft(AircraftType::kRaptor, m_textures, m_fonts));
	right_escort->setPosition(80.f, 50.f);
	m_player_aircraft->AttachChild(std::move(right_escort));*/
}

void World::AdaptPlayerPosition()
{
	////keep the player on the screen
	//sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
	//const float border_distance = 40.f;

	//sf::Vector2f position = m_player_aircraft->getPosition();
	//position.x = std::max(position.x, view_bounds.left + border_distance);
	//position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
	//position.y = std::max(position.y, view_bounds.top + border_distance);
	//position.y = std::min(position.y, view_bounds.top + view_bounds.height -border_distance);
	//m_player_aircraft->setPosition(position);

	//we need to adapt this for the player vector
	for (auto& player : m_player_aircrafts)
	{
		sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
		const float border_distance = 40.f;
		sf::Vector2f position = player->getPosition();
		position.x = std::max(position.x, view_bounds.left + border_distance);
		position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
		position.y = std::max(position.y, view_bounds.top + border_distance);
		position.y = std::min(position.y, view_bounds.top + view_bounds.height - border_distance);
		player->setPosition(position);
	}
}

void World::AdaptPlayerVelocity()
{
	//sf::Vector2f velocity = m_player_aircraft->GetVelocity();

	////If they are moving diagonally divide by sqrt 2
	//if (velocity.x != 0.f && velocity.y != 0.f)
	//{
	//	m_player_aircraft->SetVelocity(velocity / std::sqrt(2.f));
	//}
	//We need to adapt this for the player vector
	for (auto& player : m_player_aircrafts)
	{
		sf::Vector2f velocity = player->GetVelocity();
		//If they are moving diagonally divide by sqrt 2
		if (velocity.x != 0.f && velocity.y != 0.f)
		{
			player->SetVelocity(velocity / std::sqrt(2.f));
		}
	}

	//Add scrolling velocity
//	m_player_aircraft->Accelerate(0.f, m_scrollspeed);
}

//void World::SpawnEnemies()
//{
//	//Spawn an enemy when it is relevant i.e when it is in the Battlefieldboudns
//	while (!m_enemy_spawn_points.empty() && m_enemy_spawn_points.back().m_y > GetBattleFieldBounds().top)
//	{
//		SpawnPoint spawn = m_enemy_spawn_points.back();
//		std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.m_type, m_textures, m_fonts));
//		enemy->setPosition(spawn.m_x, spawn.m_y);
//		enemy->setRotation(180.f);
//		m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(enemy));
//		m_enemy_spawn_points.pop_back();
//	}
//}
//
//void World::AddEnemies()
//{
//	AddEnemy(AircraftType::kRaptor, 0.f, 500.f);
//	AddEnemy(AircraftType::kRaptor, 0.f, 1000.f);
//	AddEnemy(AircraftType::kRaptor, 100.f, 1100.f);
//	AddEnemy(AircraftType::kRaptor, -100.f, 1100.f);
//	AddEnemy(AircraftType::kAvenger, -70.f, 1400.f);
//	AddEnemy(AircraftType::kAvenger, 70.f, 1400.f);
//	AddEnemy(AircraftType::kAvenger, 70.f, 1600.f);
//
//	//Sort the enemies according to y-value so that enemies are checked first
//	std::sort(m_enemy_spawn_points.begin(), m_enemy_spawn_points.end(), [](SpawnPoint lhs, SpawnPoint rhs)
//	{
//		return lhs.m_y < rhs.m_y;
//	});
//
//}
//
//void World::AddEnemy(AircraftType type, float relx, float rely)
//{
//	SpawnPoint spawn(type, m_spawn_position.x + relx, m_spawn_position.y - rely);
//	m_enemy_spawn_points.emplace_back(spawn);
//}
void World::SpawnEnemy()
{
	// Choose an enemy type at random.
	AircraftType type = (Utility::RandomInt(2) == 0) ? AircraftType::kRaptor : AircraftType::kAvenger;

	// Get the current view bounds.
	sf::FloatRect viewBounds = GetViewBounds();
	// Define a margin to spawn outside the view.
	float margin = 50.f;

	// Determine a random side (0: top, 1: bottom, 2: left, 3: right)
	int side = Utility::RandomInt(4);

	sf::Vector2f spawnPos;
	switch (side)
	{
	case 0: // Top: spawn at a random x within view (plus margin) and above the view.
	{
		int minX = static_cast<int>(viewBounds.left + margin);
		int maxX = static_cast<int>(viewBounds.left + viewBounds.width - margin);
		float x = static_cast<float>(minX + Utility::RandomInt(maxX - minX));
		float y = viewBounds.top - margin;
		spawnPos = sf::Vector2f(x, y);
		break;
	}
	case 1: // Bottom: spawn at a random x within view (plus margin) and below the view.
	{
		int minX = static_cast<int>(viewBounds.left + margin);
		int maxX = static_cast<int>(viewBounds.left + viewBounds.width - margin);
		float x = static_cast<float>(minX + Utility::RandomInt(maxX - minX));
		float y = viewBounds.top + viewBounds.height + margin;
		spawnPos = sf::Vector2f(x, y);
		break;
	}
	case 2: // Left: spawn at a random y within view (plus margin) and to the left of the view.
	{
		int minY = static_cast<int>(viewBounds.top + margin);
		int maxY = static_cast<int>(viewBounds.top + viewBounds.height - margin);
		float y = static_cast<float>(minY + Utility::RandomInt(maxY - minY));
		float x = viewBounds.left - margin;
		spawnPos = sf::Vector2f(x, y);
		break;
	}
	case 3: // Right: spawn at a random y within view (plus margin) and to the right of the view.
	{
		int minY = static_cast<int>(viewBounds.top + margin);
		int maxY = static_cast<int>(viewBounds.top + viewBounds.height - margin);
		float y = static_cast<float>(minY + Utility::RandomInt(maxY - minY));
		float x = viewBounds.left + viewBounds.width + margin;
		spawnPos = sf::Vector2f(x, y);
		break;
	}
	default:
		// Default to top if something goes wrong.
		spawnPos = sf::Vector2f(viewBounds.left + viewBounds.width / 2.f, viewBounds.top - margin);
		break;
	}

	// Create the enemy and set its properties.
	std::unique_ptr<Aircraft> enemy(new Aircraft(type, m_textures, m_fonts));
	enemy->setPosition(spawnPos);

	// Compute the center of the view (target point).
	sf::Vector2f viewCenter(viewBounds.left + viewBounds.width / 2.f, viewBounds.top + viewBounds.height / 2.f);

	// Compute a vector from the spawn position toward the view center.
	sf::Vector2f toCenter = viewCenter - spawnPos;
	float length = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
	if (length != 0.f)
	{
		toCenter /= length;
	}

	// Compute the angle (in degrees) so that the enemy faces the center.
	// Adjust the angle if your sprite's default orientation requires it.
	float angleRadians = std::atan2(toCenter.y, toCenter.x);
	float angleDegrees = static_cast<float>(Utility::ToDegrees(angleRadians));
	// If your enemy sprite is designed to face upward (0° = up) then add 90 degrees.
	angleDegrees += 90.f;
	enemy->setRotation(angleDegrees);

	// Optionally, set an initial velocity if needed. 
	// We'll leave it at zero so that GuideEnemies() can update it later.
	enemy->SetVelocity(0.f, 0.f);

	// Attach the new enemy to the UpperAir scene layer.
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(enemy));
}


sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize()/2.f, m_camera.getSize());
}

sf::FloatRect World::GetBattleFieldBounds() const
{
	//Return camera bounds + a small area at the top where enemies spawn
	sf::FloatRect bounds = GetViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;

}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(ReceiverCategories::kProjectile);
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time dt)
		{
			//Does the object intersect with the battlefield
			if (!GetBattleFieldBounds().intersects(e.GetBoundingRect()))
			{
				e.Destroy();
			}
		});
	m_command_queue.Push(command);
}

void World::GuideMissiles()
{
	//Target the closest enemy in the world
	Command enemyCollector;
	enemyCollector.category = static_cast<int>(ReceiverCategories::kEnemyAircraft);
	enemyCollector.action = DerivedAction<Aircraft>([this](Aircraft& enemy, sf::Time)
		{
			if (!enemy.IsDestroyed())
			{
				m_active_enemies.emplace_back(&enemy);
			}
		});

	Command missileGuider;
	missileGuider.category = static_cast<int>(ReceiverCategories::kAlliedProjectile);
	missileGuider.action = DerivedAction<Projectile>([this](Projectile& missile, sf::Time dt)
		{
			if (!missile.IsGuided())
			{
				return;
			}

			float min_distance = std::numeric_limits<float>::max();
			Aircraft* closest_enemy = nullptr;

			for (Aircraft* enemy : m_active_enemies)
			{
				float enemy_distance = Distance(missile, *enemy);
				if (enemy_distance < min_distance)
				{
					closest_enemy = enemy;
					min_distance = enemy_distance;
				}
			}

			if (closest_enemy)
			{
				missile.GuideTowards(closest_enemy->GetWorldPosition());
			}
		});

	m_command_queue.Push(enemyCollector);
	m_command_queue.Push(missileGuider);
	m_active_enemies.clear();
}
void World::GuideEnemies(sf::Time dt)
{
	// Command to adjust enemy aircraft velocities.
	Command enemyGuideCommand;
	enemyGuideCommand.category = static_cast<int>(ReceiverCategories::kEnemyAircraft);
	enemyGuideCommand.action = DerivedAction<Aircraft>([this](Aircraft& enemy, sf::Time dt)
		{
			// Skip destroyed enemies
			if (enemy.IsDestroyed())
				return;

			// Get the current position of the enemy
			sf::Vector2f enemyPos = enemy.GetWorldPosition();

			// Find the closest player
			float closestDistance = std::numeric_limits<float>::max();
			sf::Vector2f closestPlayerPos;

			for (auto* player : m_player_aircrafts)
			{
				if(player == nullptr)
				{
					continue;
				}
				if (player->IsDestroyed())
					continue;

				// Get player position
				sf::Vector2f playerPos = player->GetWorldPosition();

				// Compute the distance between the enemy and this player
				float distance = std::sqrt(std::pow(playerPos.x - enemyPos.x, 2) + std::pow(playerPos.y - enemyPos.y, 2));
				if (distance < closestDistance)
				{
					closestDistance = distance;
					closestPlayerPos = playerPos;
				}
			}

			// If there's a closest player, adjust the enemy velocity toward that player
			if (closestDistance < std::numeric_limits<float>::max())  // A player was found
			{
				// Compute direction vector from enemy to closest player
				sf::Vector2f direction = closestPlayerPos - enemyPos;
				float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
				if (length != 0.f)
				{
					direction /= length;  // Normalize the direction
				}

				// Set the enemy velocity towards the player
				enemy.SetVelocity(direction * enemy.GetMaxSpeed());

				// Calculate angle for rotation
				float angleRadians = std::atan2(direction.y, direction.x);
				float angleDegrees = static_cast<float>(Utility::ToDegrees(angleRadians));

				// Adjust angle if necessary (e.g., sprite default orientation)
				angleDegrees += 90.f;

				enemy.SetRotation(angleDegrees);
			}

		});

	// Push the command so it gets executed on enemy aircraft
	m_command_queue.Push(enemyGuideCommand);
}



bool MatchesCategories(SceneNode::Pair& colliders, ReceiverCategories type1, ReceiverCategories type2)
{
	unsigned int category1 = colliders.first->GetCategory();
	unsigned int category2 = colliders.second->GetCategory();

	if (static_cast<int>(type1) & category1 && static_cast<int>(type2) & category2)
	{
		return true;
	}
	else if (static_cast<int>(type1) & category2 && static_cast<int>(type2) & category1)
	{ 
		std::swap(colliders.first, colliders.second);
	}
	else
	{
		return false;
	}
}


void World::HandleCollisions()
{
	std::set<SceneNode::Pair> collision_pairs;
	m_scenegraph.CheckSceneCollision(m_scenegraph, collision_pairs);
	for (SceneNode::Pair pair : collision_pairs)
	{
		if (MatchesCategories(pair, ReceiverCategories::kPlayers, ReceiverCategories::kEnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);
			//Collision response
			player.Damage(enemy.GetHitPoints());
			enemy.Destroy();
		}

		else if (MatchesCategories(pair, ReceiverCategories::kPlayers, ReceiverCategories::kPickup))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);
			//Collision response
			pickup.Apply(player);
			pickup.Destroy();
			player.PlayLocalSound(m_command_queue, SoundEffect::kCollectPickup);
		}
		else if (MatchesCategories(pair, ReceiverCategories::kPlayers, ReceiverCategories::kEnemyProjectile) || MatchesCategories(pair, ReceiverCategories::kEnemyAircraft, ReceiverCategories::kAlliedProjectile))
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);
			//Collision response
			aircraft.Damage(projectile.GetDamage());
			projectile.Destroy();
		}
	}
}

void World::UpdateSounds()
{
	// Set listener's position to player position
	/*m_sounds.SetListenerPosition(m_player_aircraft->GetWorldPosition());*/
	//We need to adapt this for the player vector
	//But because there can only be one listener we will set it to the first player in the vector
	
	m_sounds.SetListenerPosition(m_player_aircrafts[0]->GetWorldPosition());
	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}
