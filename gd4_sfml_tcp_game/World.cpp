/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "World.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"
#include "ParticleNode.hpp"
#include "SoundNode.hpp"
#include <iostream>

World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, bool networked)
	: m_target(output_target)
	, m_camera(output_target.getDefaultView())
	, m_textures()
	, m_fonts(font)
	, m_sounds(sounds)
	, m_scenegraph(ReceiverCategories::kNone)
	, m_scene_layers()
	, m_world_bounds(0.f, 0.f, m_camera.getSize().x, 1200.f)
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
}

void World::Update(sf::Time dt)
{
	//Scroll the world
	m_camera.move(0, m_scrollspeed * dt.asSeconds());

	DestroyEntitiesOutsideView();
	GuideEnemies(dt); //Guide the enemies towards the players

	//Forward commands to the scenegraph
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayerVelocity();

	HandleCollisions();

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

Aircraft* World::AddAircraft(int identifier,std::string clientName)
{
	int texture_id = identifier % 7;
	TextureID texture;
	std::string name;
	switch (texture_id)
    {
	case 1:
		texture = TextureID::kAgentOne;
		name = "Agent One";
        break;
	case 2:
		texture = TextureID::kAgentTwo;
		name = "Agent Two";
        break;
	case 3:
		texture = TextureID::kAgentThree;
		name = "Agent Three";
        break;
    case 4:
		texture = TextureID::kAgentFour;
		name = "Agent Four";
        break;
	case 5:
		texture = TextureID::kAgentFive;
		name = "Agent Five";
        break;
    case 6:
		texture = TextureID::kAgentSix;
		name = "Agent Six";
        break;
	case 7:
		texture = TextureID::kAgentSeven;
		name = "Agent Seven";
		break;
	default:
		texture = TextureID::kAgentOne;
		name = "Agent One";
        break;
    }

	std::unique_ptr<Aircraft> player(new Aircraft(AircraftType::kAgent, m_textures, m_fonts, texture, clientName));
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
	m_textures.Load(TextureID::kAgentOne, "Media/Textures/AgentOne.png");
	m_textures.Load(TextureID::kAgentTwo, "Media/Textures/AgentTwo.png");
	m_textures.Load(TextureID::kAgentThree, "Media/Textures/AgentThree.png");
	m_textures.Load(TextureID::kAgentFour, "Media/Textures/AgentFour.png");
	m_textures.Load(TextureID::kAgentFive, "Media/Textures/AgentFive.png");
	m_textures.Load(TextureID::kAgentSix, "Media/Textures/AgentSix.png");
	m_textures.Load(TextureID::kAgentSeven, "Media/Textures/AgentSeven.png");
	m_textures.Load(TextureID::kZombie, "Media/Textures/Zombie.png");
	m_textures.Load(TextureID::kZombie2, "Media/Textures/BloodyZombie.png");
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

void World::AddEnemy(AircraftType type, float x, float y)
{
	std::cout << "Attempting to spawn enemy of type: " << static_cast<int>(type)
		<< " at position (" << x << ", " << y << ")\n";

	TextureID texture_id;
	if (type == AircraftType::kZombie)
	{
		texture_id = TextureID::kZombie2;
	}
	else if (type == AircraftType::kAvenger)
	{
		texture_id = TextureID::kZombie;
	}

	// Create the enemy and set its properties.
	std::unique_ptr<Aircraft> enemy(new Aircraft(type, m_textures, m_fonts, texture_id, " "));
	enemy->setPosition(x, y);
	std::cout << "Enemy created at position (" << enemy->getPosition().x << ", " << enemy->getPosition().y << ")\n";

	sf::Vector2f velocity = enemy->GetVelocity();
	std::cout << "Enemy velocity: (" << velocity.x << ", " << velocity.y << ")\n";

	// Compute the center of the view (target point).
	sf::Vector2f viewCenter(m_camera.getCenter());

	// Compute a vector from the spawn position toward the view center.
	sf::Vector2f toCenter = viewCenter - sf::Vector2f(x, y);
	float length = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
	if (length != 0.f)
	{
		toCenter /= length;
	}

	// Calculate the angle from the spawn position to the view center.
	float angleRadians = std::atan2(toCenter.y, toCenter.x);
	float angleDegrees = static_cast<float>(Utility::ToDegrees(angleRadians));
	angleDegrees += 90.f;  // Adjust for the sprite's orientation if needed.

	enemy->setRotation(angleDegrees);

	// Optionally, set an initial velocity if needed.
	//enemy->SetVelocity(0.f, 0.f);

	// Attach the new enemy to the UpperAir scene layer.
	m_scene_layers[static_cast<int>(SceneLayers::kUpperAir)]->AttachChild(std::move(enemy));
	std::cout << "Enemy attached to the scene layer.\n";
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