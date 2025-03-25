/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include "Entity.hpp"
#include "AircraftType.hpp"
#include "ResourceIdentifiers.hpp"
#include "TextNode.hpp"
#include "Utility.hpp"
#include "ProjectileType.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include "Animation.hpp"
#include <SFML/Graphics/Shader.hpp>

class Aircraft : public Entity
{
public:
	Aircraft(AircraftType type, const TextureHolder& textures, const FontHolder& fonts);
	unsigned int GetCategory() const override;

	void DisablePickups();
	int GetIdentifier();
	void SetIdentifier(int identifier);

	void IncreaseFireRate();
	void CollectMissile(unsigned int count);
	
	//Added by Albert
	void ActivateInvincibility(sf::Time duration);
	//Added by Albert
	void Damage(int points) override;
	//Added by Albert
	void ActivateMinigun(sf::Time duration);

	void UpdateTexts();

	float GetMaxSpeed() const;
	void Fire();
	void LaunchMissile();
	void CreateBullet(SceneNode& node, const TextureHolder& textures) const;
	void CreateProjectile(SceneNode& node, ProjectileType type, float x_float, float y_offset, const TextureHolder& textures) const;
	void SetRotation(float angle);
	float GetRotation() const;

	sf::FloatRect GetBoundingRect() const override;
	bool IsMarkedForRemoval() const override;
	void Remove() override;
	void PlayLocalSound(CommandQueue& commands, SoundEffect effect);
	


private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void CheckProjectileLaunch(sf::Time dt, CommandQueue& commands);
	bool IsAllied() const;
	void CreatePickup(SceneNode& node, const TextureHolder& textures) const;
	void CheckPickupDrop(CommandQueue& commands);
	void UpdateRollAnimation();

private:
	AircraftType m_type;
	sf::Sprite m_sprite;
	Animation m_explosion;



	TextNode* m_health_display;
	TextNode* m_missile_display;
	float m_distance_travelled;
	int m_directions_index;

	Command m_fire_command;
	Command m_missile_command;
	Command m_drop_pickup_command;

	unsigned int m_fire_rate;
	unsigned int m_spread_level;
	unsigned int m_missile_ammo;

	bool m_is_firing;
	bool m_is_launching_missile;
	sf::Time m_fire_countdown;

	bool m_is_marked_for_removal;
	bool m_show_explosion;
	bool m_explosion_began;
	bool m_spawned_pickup;
	bool m_pickups_enabled;

	int m_identifier;

	float m_rotation;

	//Added by Albert
	bool m_is_invincible;
	//Added by Albert
	sf::Time m_invincibility_timer;
	//Added by Albert
	sf::Shader m_invincibilityShader;
	//Added by Albert
	sf::Time m_minigun_timer;
};

