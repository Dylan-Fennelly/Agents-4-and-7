/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"

World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, bool networked)
	: m_target(output_target)
	, m_camera(output_target.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_sounds(sounds)
	, m_scenegraph(ReceiverCategories::kNone)
	, m_scene_layers()
	, m_world_bounds(0.f, 0.f, m_camera.getSize().x, 5000.f)
	, m_spawn_position(m_camera.getSize().x / 2.f, m_world_bounds.height - m_camera.getSize().y / 2.f)
	, m_scrollspeed(0.f)
	, m_enemySpawnTimer(sf::Time::Zero)
	, m_enemySpawnInterval(sf::seconds(2.f)) // Initial spawn interval; will be randomized after each spawn.
	, m_player_aircrafts()
	, m_networked_world(networked)
	, m_network_node(nullptr)
{
	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);
	LoadTextures();
	BuildScene();
	m_camera.setCenter(m_spawn_position);
	//// === Create Player 1 ===
	//auto player1 = std::make_unique<Aircraft>(AircraftType::kAgentFour, m_textures, m_fonts);
	//player1->SetCategory(ReceiverCategories::kPlayerAircraft);
	//player1->setPosition(m_spawn_position.x - 50.f, m_spawn_position.y); // Offset left
	//m_player_aircrafts.push_back(player1.get());
	//m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(player1));

	//// === Create Player 2 ===
	//auto player2 = std::make_unique<Aircraft>(AircraftType::kAgentFour, m_textures, m_fonts);
	//player2->SetCategory(ReceiverCategories::kAlliedAircraft); //We are using the categorys to control the playert
	//player2->setPosition(m_spawn_position.x + 50.f, m_spawn_position.y); // Offset right
	//m_player_aircrafts.push_back(player2.get());
	//m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(player2));
}

void World::Update(sf::Time dt)
{
	//Scroll the world
	m_camera.move(0, m_scrollspeed * dt.asSeconds());

	
	m_enemySpawnTimer += dt;//Track the time until the next enemy spawn
	if (m_enemySpawnTimer >= m_enemySpawnInterval) // Time to spawn a new enemy? 
	{												//ToDO:Check if the nesting on this if statement is correct
		if ((m_totalElapsed.getElapsedTime() < sf::seconds(180))) //Spawm enemies for 3 minutes 
		{
			SpawnEnemy();
			m_enemySpawnTimer = sf::Time::Zero;
			// Randomize the next spawn interval (for example, between 1.5 and 2 seconds):
			float nextInterval = 0.75f + static_cast<float>(Utility::RandomInt(1250)) / 1000.f; // 0.75 to 2.0 seconds
			m_enemySpawnInterval = sf::seconds(nextInterval);
		}
		
	}

	DestroyEntitiesOutsideView();
	GuideMissiles();
	GuideEnemies(dt); //Guide the enemies towards the players

	//Forward commands to the scenegraph
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayerVelocity();

	HandleCollisions();
	// Remove dead players from m_player_aircrafts
	//Dylan - This is a lambda function that removes the player from the vector if they are marked for removal it prevents an annoying bug where a pointer to a player that has been destroyed is still in the vector
	//Chatgpt helped with the structure of this code - I've written too much c# lately and I'm getting rusty with c++
	/*m_player_aircrafts.erase(
		std::remove_if(m_player_aircrafts.begin(), m_player_aircrafts.end(),
			[](Aircraft* player) { return player->IsMarkedForRemoval(); }),
		m_player_aircrafts.end());*/

	auto first_to_remove = std::remove_if(m_player_aircrafts.begin(), m_player_aircrafts.end(), std::mem_fn(&Aircraft::IsMarkedForRemoval));
	m_player_aircrafts.erase(first_to_remove, m_player_aircrafts.end());

	m_scenegraph.RemoveWrecks();


	m_scenegraph.Update(dt, m_command_queue);
	AdaptPlayerPosition();
	UpdateSounds();
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

Aircraft* World::GetAircraft(int identifier) const
{
	for (Aircraft* a : m_player_aircrafts)
	{
		if (a->GetIdentifier() == identifier)
		{
			return a;
		}
	}
	return nullptr;
}

void World::RemoveAircraft(int identifier)
{
	Aircraft* aircraft = GetAircraft(identifier);
	if (aircraft)
	{
		aircraft->Destroy();
		m_player_aircrafts.erase(std::find(m_player_aircrafts.begin(), m_player_aircrafts.end(), aircraft));
	}
}

Aircraft* World::AddAircraft(int identifier)
{
	std::unique_ptr<Aircraft> player(new Aircraft(AircraftType::kAgentFour, m_textures, m_fonts));
	player->setPosition(m_camera.getCenter());
	player->SetIdentifier(identifier);

	m_player_aircrafts.emplace_back(player.get());
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(player));
	return m_player_aircrafts.back();
}

void World::CreatePickup(sf::Vector2f position, PickupType type)
{
	std::unique_ptr<Pickup> pickup(new Pickup(type, m_textures));
	pickup->setPosition(position);
	pickup->SetVelocity(0.f, 1.f);
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(pickup));
}

bool World::PollGameAction(GameActions::Action& out)
{
	return m_network_node->PollGameAction(out);
}

void World::SetCurrentBattleFieldPosition(float lineY)
{
	m_camera.setCenter(m_camera.getCenter().x, lineY - m_camera.getSize().y / 2);
	m_spawn_position.y = m_world_bounds.height;
}

void World::SetWorldHeight(float height)
{
	m_world_bounds.height = height;
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

sf::View World::GetCamera() const
{
	return m_camera;
}

bool World::HasAlivePlayer() const
{
	return !m_player_aircrafts.empty();
}


bool World::HasPlayerReachedEnd(sf::Time dt) //Check if there are any enemies left 
{
	if (sf::seconds(10) < m_totalElapsed.getElapsedTime()) //Give the enemy spawner some time to spawn enemies
	{
		Command enemyCollector;
		enemyCollector.category = static_cast<int>(ReceiverCategories::kEnemyAircraft);
		enemyCollector.action = DerivedAction<Aircraft>([this](Aircraft& enemy, sf::Time)
			{
				if (!enemy.IsDestroyed())
				{
					m_active_enemies.emplace_back(&enemy);
				}
			});
		if (m_active_enemies.empty())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

void World::LoadTextures()
{
	//Most textures have been sourced in DataTables.cpp, unless stated otherwise
	m_textures.Load(TextureID::kAgentFour, "Media/Textures/AgentFour.png");
	m_textures.Load(TextureID::kZombie, "Media/Textures/Zombie.png");
	m_textures.Load(TextureID::kZombie2, "Media/Textures/Zombie.png");
	m_textures.Load(TextureID::kLandscape, "Media/Textures/Desert.png");
	m_textures.Load(TextureID::kBullet, "Media/Textures/Bullet.png");
	m_textures.Load(TextureID::kMissile, "Media/Textures/Missile.png");

	m_textures.Load(TextureID::kHealthRefill, "Media/Textures/HealthRefill.png");
	m_textures.Load(TextureID::kMissileRefill, "Media/Textures/MissileRefill.png");
	m_textures.Load(TextureID::kFireSpread, "Media/Textures/FireSpread.png");
	m_textures.Load(TextureID::kFireRate, "Media/Textures/FireRate.png");
	m_textures.Load(TextureID::kFinishLine, "Media/Textures/FinishLine.png");

	m_textures.Load(TextureID::kEntities, "Media/Textures/Entities.png");
	//Source - https://opengameart.org/content/grass-pixel-art
	m_textures.Load(TextureID::kBackground, "Media/Textures/Background.png");
	m_textures.Load(TextureID::kExplosion, "Media/Textures/5_100x100px.png");
	m_textures.Load(TextureID::kParticle, "Media/Textures/Particle.png");

	//Added by Albert
	m_textures.Load(TextureID::kInvincibility, "Media/Textures/Invincibility.png");
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
	sf::Texture& texture = m_textures.Get(TextureID::kBackground);
	texture.setRepeated(true);

	float view_height = m_camera.getSize().y;
	sf::IntRect texture_rect(m_world_bounds);
	texture_rect.height += static_cast<int>(view_height);

	//Add the background sprite to the world
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, texture_rect));
	background_sprite->setPosition(m_world_bounds.left, m_world_bounds.top - view_height);
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(background_sprite));

	//Add the finish line
	sf::Texture& finish_texture = m_textures.Get(TextureID::kFinishLine);
	std::unique_ptr<SpriteNode> finish_sprite(new SpriteNode(finish_texture));
	finish_sprite->setPosition(0.f, -76.f);
	m_scene_layers[static_cast<int>(SceneLayers::kBackground)]->AttachChild(std::move(finish_sprite));

	//Add the particle nodes to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(ParticleType::kSmoke, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(smokeNode));

	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(ParticleType::kPropellant, m_textures));
	m_scene_layers[static_cast<int>(SceneLayers::kLowerAir)]->AttachChild(std::move(propellantNode));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(m_sounds));
	m_scenegraph.AttachChild(std::move(soundNode));

	if (m_networked_world)
	{
		std::unique_ptr<NetworkNode> network_node(new NetworkNode());
		m_network_node = network_node.get();
		m_scenegraph.AttachChild(std::move(network_node));
	}
}

void World::AdaptPlayerPosition()
{
	//we needed to adapt this for the player vector
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

	//We needed to adapt this for the player vector
	//for (auto& player : m_player_aircrafts)
	//{
	//	sf::Vector2f velocity = player->GetVelocity();
	//	//If they are moving diagonally divide by sqrt 2
	//	if (velocity.x != 0.f && velocity.y != 0.f)
	//	{
	//		player->SetVelocity(velocity / std::sqrt(2.f));
	//	}
	//}

	for (Aircraft* aircraft : m_player_aircrafts)
	{
		sf::Vector2f velocity = aircraft->GetVelocity();

		//If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
		{
			aircraft->SetVelocity(velocity / std::sqrt(2.f));
		}
	}
}

sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
}

sf::FloatRect World::GetBattlefieldBounds() const
{
	//Return camera bounds + a small area at the top where enemies spawn
	sf::FloatRect bounds = GetViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;

}

void World::SpawnEnemy()
{
	// Choose an enemy type at random.
	AircraftType type = (Utility::RandomInt(2) == 0) ? AircraftType::kZombie : AircraftType::kAvenger; 

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

	float angleRadians = std::atan2(toCenter.y, toCenter.x);
	float angleDegrees = static_cast<float>(Utility::ToDegrees(angleRadians));
	angleDegrees += 90.f;//Is this necessary anymore ?
	enemy->setRotation(angleDegrees);

	// Optionally, set an initial velocity if needed. 
	enemy->SetVelocity(0.f, 0.f);

	if (m_networked_world)
	{
		enemy->DisablePickups();
	}

	// Attach the new enemy to the UpperAir scene layer.
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(enemy));
}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(ReceiverCategories::kProjectile); //We only want to remove strat bullets, this lets the enemies spawn off screen
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time dt)
		{
			//Does the object intersect with the battlefield
			if (!GetBattlefieldBounds().intersects(e.GetBoundingRect()))
			{
				e.Remove();
			}
		});
	m_command_queue.Push(command);
}

void World::GuideMissiles() //Todo: Remove this as there are no more missiles
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
void World::GuideEnemies(sf::Time dt) //Heavily based on the guide missile - makes the zombies fleshy missles
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
				if (player == nullptr)
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
	sf::Vector2f listener_position;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (m_player_aircrafts.empty())
	{
		listener_position = m_camera.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		for (Aircraft* aircraft : m_player_aircrafts)
		{
			listener_position += aircraft->GetWorldPosition();
		}

		listener_position /= static_cast<float>(m_player_aircrafts.size());
	}

	// Set listener's position
	m_sounds.SetListenerPosition(listener_position);

	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}