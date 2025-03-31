/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "Aircraft.hpp"
#include "TextureID.hpp"
#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include "DataTables.hpp"
#include "Projectile.hpp"
#include "PickupType.hpp"
#include "Pickup.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"
#include <iostream>

namespace
{
	const std::vector<AircraftData> Table = InitializeAircraftData();
}

//TextureID ToTextureID(AircraftType type)
//{
//	switch (type)
//	{
//	case AircraftType::kAgent:
//		return TextureID::kAgentFour;
//		break;
//	case AircraftType::kZombie:
//		return TextureID::kZombie;
//		break;
//	case AircraftType::kAvenger:
//		return TextureID::kZombie2;
//		break;
//	}
//	return TextureID::kAgentFour;
//}

Aircraft::Aircraft(AircraftType type, const TextureHolder& textures, const FontHolder& fonts, TextureID texture, std::string name)  
	: Entity(Table[static_cast<int>(type)].m_hitpoints)
	, m_type(type)
	, m_sprite(textures.Get(texture), Table[static_cast<int>(type)].m_texture_rect)
	, m_explosion(textures.Get(TextureID::kExplosion))
	, m_health_display(nullptr)
	, m_name_display(nullptr)
	, m_distance_travelled(0.f)
	, m_directions_index(0)
	, m_fire_rate(1)
	, m_spread_level(1)
	, m_is_firing(false)
	, m_is_launching_missile(false)
	, m_fire_countdown(sf::Time::Zero)
	, m_missile_ammo(2)
	, m_is_marked_for_removal(false)
	, m_show_explosion(true)
	, m_explosion_began(false)
	, m_spawned_pickup(false)
	, m_pickups_enabled(true)
	, m_identifier(0)
	, m_rotation(0.f)
	, m_name(name)
{
	m_explosion.SetFrameSize(sf::Vector2i(100, 100));
	m_explosion.SetNumFrames(60);
	m_explosion.SetDuration(sf::seconds(1));
	Utility::CentreOrigin(m_sprite);
	Utility::CentreOrigin(m_explosion);

	m_fire_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_fire_command.action = [this, &textures](SceneNode& node, sf::Time dt)
		{
			CreateBullet(node, textures);
		};

	m_missile_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_missile_command.action = [this, &textures](SceneNode& node, sf::Time dt)
		{
			CreateProjectile(node, ProjectileType::kMissile, 0.f, 0.5f, textures);
		};

	m_drop_pickup_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_drop_pickup_command.action = [this, &textures](SceneNode& node, sf::Time dt)
		{
			CreatePickup(node, textures);
		};

	std::string* health = new std::string("");
	std::unique_ptr<TextNode> health_display(new TextNode(fonts, *health));
	m_health_display = health_display.get();
	AttachChild(std::move(health_display));

	if (Aircraft::GetCategory() == static_cast<int>(ReceiverCategories::kPlayerAircraft))
	{
		std::string* missile_ammo = new std::string("");
		std::unique_ptr<TextNode> missile_display(new TextNode(fonts, *missile_ammo));
		m_name_display = missile_display.get();
		AttachChild(std::move(missile_display));
	}

	//Added by Albert
	//I added the shader here instead of the provided shader classes, because I struggled to get them to work in BloomEffect.cpp etc., plus I wanted the shader to be applied to the invincible mode only
	//The shader below has been written with the aid of ChatGPT
	if (!m_invincibilityShader.loadFromFile("Media/Shaders/Fullpass.vert", "Media/Shaders/GoldenEffectPulse.frag"))
	{
		throw std::runtime_error("Shader failed to load");
	}

	UpdateTexts();
}
void Aircraft::DisablePickups()
{
	m_pickups_enabled = false;
}

int	Aircraft::GetIdentifier()
{
	return m_identifier;
}

void Aircraft::SetIdentifier(int identifier)
{
	m_identifier = identifier;
}


unsigned int Aircraft::GetCategory() const
{
	if (IsAllied())
	{
		return static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
	}
	return static_cast<unsigned int>(ReceiverCategories::kEnemyAircraft);

}

void Aircraft::IncreaseFireRate()
{
	//Modified by Albert
	//I multiply the rate by 10, because I felt like this was the best way to represent the minigun effect
	if (m_fire_rate < 10)
	{
		m_fire_rate *= 10;
	}
}

void Aircraft::CollectMissile(unsigned int count)
{
	m_missile_ammo += count;
}

//Added by Albert
//This function activates the Invincibility powerup that makes the player a god-like floating orb that is immortal for five seconds
void Aircraft::ActivateInvincibility(sf::Time duration)
{
	m_is_invincible = true;
	m_invincibility_timer = duration;
}

//Added by Albert
//This function activates the Invincibility powerup that increases the fire rate of the default weapon and imitates a minigun
void Aircraft::ActivateMinigun(sf::Time duration)
{
	IncreaseFireRate();
	m_minigun_timer = duration;
}

//Added by Albert
//I inherited this function from Entity to check if the player is invincible, if they are, they take no damage
void Aircraft::Damage(int points)
{
	if (!m_is_invincible)
	{
		Entity::Damage(points);
	}
}

void Aircraft::UpdateTexts()
{
	if (IsDestroyed())
	{
		m_health_display->SetString("");
	}
	else
	{
		m_health_display->SetString(std::to_string(GetHitPoints()) + "HP");
	}
	m_health_display->setPosition(0.f, 50.f);

	if (m_name_display)
	{
		m_name_display->setPosition(0.f, 70.f);
		if (m_name == "")
		{
			m_name_display->SetString("");
		}
		else
		{
			m_name_display->SetString(m_name);
		}
	}

	//// Get sprite's world position (ignores rotation)
	//sf::Vector2f spritePosition = getPosition();

	//if (IsDestroyed())
	//{
	//	m_health_display->SetString("");
	//}
	//else
	//{
	//	m_health_display->SetString(std::to_string(GetHitPoints()) + "HP");
	//}

	//// Offset the text manually without rotation
	//m_health_display->setPosition(spritePosition.x, spritePosition.y + 50.f);

	//if (m_name_display)
	//{
	//	if (m_name == "")
	//	{
	//		m_name_display->SetString("");
	//	}
	//	else
	//	{
	//		m_name_display->SetString(m_name);
	//	}

	//	// Offset the name text manually without rotation
	//	m_name_display->setPosition(spritePosition.x, spritePosition.y + 70.f);
	//}
}

float Aircraft::GetMaxSpeed() const
{
	return Table[static_cast<int>(m_type)].m_speed;
}

void Aircraft::Fire()
{
	if (Table[static_cast<int>(m_type)].m_fire_interval != sf::Time::Zero)
	{
		m_is_firing = true;
	}
	
}

void Aircraft::LaunchMissile()
{
	if (m_missile_ammo > 0)
	{
		m_is_launching_missile = true;
		--m_missile_ammo;
	}
}

void Aircraft::CreateBullet(SceneNode& node, const TextureHolder& textures) const
{
	ProjectileType type = IsAllied() ? ProjectileType::kAlliedBullet : ProjectileType::kEnemyBullet;
	switch (m_spread_level)
	{
	case 1:
		CreateProjectile(node, type, 0.0f, 0.5f, textures);
		break;
	case 2:
		CreateProjectile(node, type, -0.5f, 0.5f, textures);
		CreateProjectile(node, type, 0.5f, 0.5f, textures);
		break;
	case 3:
		CreateProjectile(node, type, 0.0f, 0.5f, textures);
		CreateProjectile(node, type, -0.5f, 0.5f, textures);
		CreateProjectile(node, type, 0.5f, 0.5f, textures);
		break;
	}
	
}
void Aircraft::CreateProjectile(SceneNode& node, ProjectileType type, float x_offset, float y_offset, const TextureHolder& textures) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));

	//We need to take the players roatation into account so that our bullets have the right rotaation
	// Get aircraft rotation (in degrees) and convert to radians


	float rotation = getRotation();
	float radians = Utility::ToRadians(rotation);

	// Calculate rotated offset to align with aircraft's rotation
	//Dylan - I used ChatGpt to asset in the semantics of calculating the bullets offset
	sf::Vector2f offset(
		x_offset * m_sprite.getGlobalBounds().width * std::cos(radians) - y_offset * m_sprite.getGlobalBounds().height * std::sin(radians),
		x_offset * m_sprite.getGlobalBounds().width * std::sin(radians) + y_offset * m_sprite.getGlobalBounds().height * std::cos(radians)
	);

	// Corrected bullet velocity so it moves forward
	sf::Vector2f velocity(
		-std::sin(radians),  // Inverted X for correct forward movement
		std::cos(radians)    // Inverted Y for correct forward movement
	);

	float sign = IsAllied() ? -1.f : 1.f; //Unless we add zombies with guns we probally dont need this 
	projectile->setPosition(GetWorldPosition() + offset * sign);
	projectile->SetVelocity(velocity * projectile->GetMaxSpeed() * sign);
	projectile->rotate(rotation);

	node.AttachChild(std::move(projectile));
}

void Aircraft::SetRotation(float angle)
{
	//m_rotation = angle;
	//std::cout << "Rotation: " << angle << std::endl;
	setRotation(angle); //We rotate using the entity class
	m_sprite.setRotation(0); //Set the rotation of the sprite to 0 so we dont have the sprite the wrong way
}

float Aircraft::GetRotation() const
{
	return getRotation();
}

sf::FloatRect Aircraft::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

bool Aircraft::IsMarkedForRemoval() const
{
	return IsDestroyed() && (m_explosion.IsFinished() || !m_show_explosion);
}

void Aircraft::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	//Added by Albert
	//I set the shader here that imitates a glowing orb for the invincibility powerup
	//ChatGPT helped me write this shader
	if (IsAllied() && m_is_invincible)
	{
		states.shader = &m_invincibilityShader;
	}

	if (IsDestroyed() && m_show_explosion)
	{
		target.draw(m_explosion, states);
	}
	else
	{
		target.draw(m_sprite, states);
	}
}


void Aircraft::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	//Added by Albert
	//A timer for both the powerup and the shader (5 seconds)
	if (m_is_invincible)
	{
		m_invincibility_timer -= dt;
		if (m_invincibility_timer <= sf::Time::Zero)
		{
			m_is_invincible = false;
		}

		//Again, ChatGPT helped me write this shader
		m_invincibilityShader.setUniform("time", m_invincibility_timer.asSeconds());
	}
	else
	{
		m_invincibilityShader.setUniform("time", 0.f);
	}

	//Added by Albert
	//A timer for the minigun powerup (5 seconds also)
	if (m_minigun_timer > sf::Time::Zero)
	{
		m_minigun_timer -= dt;
		if (m_minigun_timer <= sf::Time::Zero)
		{
			m_fire_rate = 1;
		}
	}

	if (IsDestroyed())
	{
		CheckPickupDrop(commands);
		m_explosion.Update(dt);
		if (!m_explosion_began)
		{
			SoundEffect soundEffect = (Utility::RandomInt(2) == 0) ? SoundEffect::kDyingZombie : SoundEffect::kDyingZombie2;
			PlayLocalSound(commands, soundEffect);
			//Emit network game action for enemy explodes
			if (!IsAllied())
			{
				sf::Vector2f position = GetWorldPosition();

				Command command;
				command.category = static_cast<int>(ReceiverCategories::kNetwork);
				command.action = DerivedAction<NetworkNode>([position](NetworkNode& node, sf::Time)
					{
						node.NotifyGameAction(GameActions::kEnemyExplode, position);
					});

				commands.Push(command);
			}

			m_explosion_began = true;
		}
		return;
	}

	Entity::UpdateCurrent(dt, commands);
	UpdateTexts();
	UpdateRollAnimation();
	CheckProjectileLaunch(dt, commands);

}

void Aircraft::CheckProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	if (!IsAllied())
	{
		Fire();
	}

	if (m_is_firing && m_fire_countdown <= sf::Time::Zero)
	{
		PlayLocalSound(commands, IsAllied() ? SoundEffect::kEnemyGunfire : SoundEffect::kPistolShot);
		commands.Push(m_fire_command);
		m_fire_countdown += Table[static_cast<int>(m_type)].m_fire_interval / (m_fire_rate + 1.f);
		m_is_firing = false;
	}
	else if (m_fire_countdown > sf::Time::Zero)
	{
		//Wait, can't fire
		m_fire_countdown -= dt;
		m_is_firing = false;
	}

	//Missile launch
	if (m_is_launching_missile)
	{
		PlayLocalSound(commands, SoundEffect::kLaunchRocket);
		commands.Push(m_missile_command);
		m_is_launching_missile = false;
	}
}

bool Aircraft::IsAllied() const
{
	return m_type == AircraftType::kAgent;
}

void Aircraft::Remove()
{
	Entity::Remove();
	m_show_explosion = false;
}

void Aircraft::CreatePickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<PickupType>(Utility::RandomInt(static_cast<int>(PickupType::kPickupCount)));
	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(GetWorldPosition());
	pickup->SetVelocity(0.f, 0.f);
	node.AttachChild(std::move(pickup));
}

void Aircraft::CheckPickupDrop(CommandQueue& commands)
{
	//TODO Get rid of the magic number 3 here 
	if (!IsAllied() && Utility::RandomInt(100) <= 10 && !m_spawned_pickup)
	{
		commands.Push(m_drop_pickup_command);
	}
	m_spawned_pickup = true;
}

void Aircraft::UpdateRollAnimation()
{
	if (Table[static_cast<int>(m_type)].m_has_roll_animation)
	{
		sf::IntRect textureRect = Table[static_cast<int>(m_type)].m_texture_rect;

		//Roll left: Texture rect is offset once
		if (GetVelocity().x < 0.f)
		{
			textureRect.left += textureRect.width;
		}
		else if (GetVelocity().x > 0.f)
		{
			textureRect.left += 2 * textureRect.width;
		}
		m_sprite.setTextureRect(textureRect);

	}
}

void Aircraft::PlayLocalSound(CommandQueue& commands, SoundEffect effect)
{
	sf::Vector2f world_position = GetWorldPosition();

	Command command;
	command.category = static_cast<int>(ReceiverCategories::kSoundEffect);
	command.action = DerivedAction<SoundNode>(
		[effect, world_position](SoundNode& node, sf::Time)
		{
			node.PlaySound(effect, world_position);
		});

	commands.Push(command);
}

