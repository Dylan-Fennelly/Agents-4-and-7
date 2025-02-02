/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "DataTables.hpp"
#include "AircraftType.hpp"
#include "ProjectileType.hpp"
#include "PickupType.hpp"
#include "Aircraft.hpp"
#include "ParticleType.hpp"

std::vector<AircraftData> InitializeAircraftData()
{
    std::vector<AircraftData> data(static_cast<int>(AircraftType::kAircraftCount));

    //Modified by Albert
	//Changed the texture for the player
    //Source - https://kenney.nl/assets/top-down-shooter
    data[static_cast<int>(AircraftType::kAgentFour)].m_hitpoints = 100;
    data[static_cast<int>(AircraftType::kAgentFour)].m_speed = 200.f;
    data[static_cast<int>(AircraftType::kAgentFour)].m_fire_interval = sf::seconds(1);
    data[static_cast<int>(AircraftType::kAgentFour)].m_texture = TextureID::kAgentFour;
    data[static_cast<int>(AircraftType::kAgentFour)].m_texture_rect = sf::IntRect(0, 0, 50, 50);
    data[static_cast<int>(AircraftType::kAgentFour)].m_has_roll_animation = false;

    //Modified by Albert
    //Changed the texture for the enemy
    //Source - https://kenney.nl/assets/top-down-shooter
    data[static_cast<int>(AircraftType::kZombie)].m_hitpoints = 10;
    data[static_cast<int>(AircraftType::kZombie)].m_speed = 90.f;
    data[static_cast<int>(AircraftType::kZombie)].m_fire_interval = sf::Time::Zero;
    data[static_cast<int>(AircraftType::kZombie)].m_texture = TextureID::kZombie;
    data[static_cast<int>(AircraftType::kZombie)].m_texture_rect = sf::IntRect(0, 0, 43, 35);
    data[static_cast<int>(AircraftType::kZombie)].m_has_roll_animation = false;

    //Modified by Albert
	//Temporarily changed the texture for the other enemy to add more enemies on-screen
    data[static_cast<int>(AircraftType::kAvenger)].m_hitpoints = 40;
    data[static_cast<int>(AircraftType::kAvenger)].m_speed = 40.f;
    data[static_cast<int>(AircraftType::kAvenger)].m_fire_interval = sf::Time::Zero;
    data[static_cast<int>(AircraftType::kAvenger)].m_texture = TextureID::kZombie;
    data[static_cast<int>(AircraftType::kAvenger)].m_texture_rect = sf::IntRect(0, 0, 43, 35);
    data[static_cast<int>(AircraftType::kAvenger)].m_has_roll_animation = false;

    return data;
}

std::vector<ProjectileData> InitializeProjectileData()
{
    std::vector<ProjectileData> data(static_cast<int>(ProjectileType::kProjectileCount));
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_damage = 10;
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_speed = 300;
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_texture = TextureID::kEntities;
    data[static_cast<int>(ProjectileType::kAlliedBullet)].m_texture_rect = sf::IntRect(175, 64, 3, 14);

    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_damage = 10;
    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_speed = 300;
    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_texture = TextureID::kEntities;
    data[static_cast<int>(ProjectileType::kEnemyBullet)].m_texture_rect = sf::IntRect(175, 64, 3, 14);


    data[static_cast<int>(ProjectileType::kMissile)].m_damage = 200;
    data[static_cast<int>(ProjectileType::kMissile)].m_speed = 150;
    data[static_cast<int>(ProjectileType::kMissile)].m_texture = TextureID::kEntities;
    data[static_cast<int>(ProjectileType::kMissile)].m_texture_rect = sf::IntRect(160, 64, 15, 32);

    return data;
}

std::vector<PickupData> InitializePickupData()
{
	//Modified by Albert
    //Health
    std::vector<PickupData> data(static_cast<int>(PickupType::kPickupCount));
    data[static_cast<int>(PickupType::kHealth)].m_texture = TextureID::kEntities;
    data[static_cast<int>(PickupType::kHealth)].m_texture_rect = sf::IntRect(0, 64, 40, 40);
    data[static_cast<int>(PickupType::kHealth)].m_action = [](Aircraft& a)
        {
            a.Repair(25);
        };

    //Added by Albert
	//Invincibility
    //Source - https://scrixels.tumblr.com/post/611665191596343296/944-ouroboros
	data[static_cast<int>(PickupType::kInvincibility)].m_texture = TextureID::kInvincibility;
	data[static_cast<int>(PickupType::kInvincibility)].m_texture_rect = sf::IntRect(0, 0, 50, 50);
	data[static_cast<int>(PickupType::kInvincibility)].m_action = std::bind(&Aircraft::ActivateInvincibility, std::placeholders::_1, sf::seconds(5));

    //Added by Albert
    //Minigun
    //Source - https://piq.codeus.net/picture/132761/Minigun
    data[static_cast<int>(PickupType::kMinigun)].m_texture = TextureID::kMinigun;
    data[static_cast<int>(PickupType::kMinigun)].m_texture_rect = sf::IntRect(0, 0, 50, 50);
    data[static_cast<int>(PickupType::kMinigun)].m_action = std::bind(&Aircraft::ActivateMinigun, std::placeholders::_1, sf::seconds(5));
    
    return data;
}

std::vector<ParticleData> InitializeParticleData()
{
    std::vector<ParticleData> data(static_cast<int>(ParticleType::kParticleCount));

    data[static_cast<int>(ParticleType::kPropellant)].m_color = sf::Color(255, 255, 50);
    data[static_cast<int>(ParticleType::kPropellant)].m_lifetime = sf::seconds(0.5f);

    data[static_cast<int>(ParticleType::kSmoke)].m_color = sf::Color(50, 50, 50);
    data[static_cast<int>(ParticleType::kSmoke)].m_lifetime = sf::seconds(2.5f);

    return data;
}
