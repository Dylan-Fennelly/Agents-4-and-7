/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include "SceneNode.hpp"
#include "CommandQueue.hpp"

class Entity : public SceneNode
{
public:
	Entity(int hitpoints);
	void SetVelocity(sf::Vector2f velocity);
	void SetVelocity(float vx, float vy);
	sf::Vector2f GetVelocity() const;
	void Accelerate(sf::Vector2f velocity);
	void Accelerate(float vx, float vy);

	int GetHitPoints() const;
	void SetHitpoints(int points);
	void Repair(int points);
	//Added by Albert
	//I made the Damage function virtual so that it can be overridden by the Aircraft class
	virtual void Damage(int points);
	void Destroy();
	virtual bool IsDestroyed() const override;
	virtual void Remove();

	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands);
	
private:
	sf::Vector2f m_velocity;
	int m_hitpoints;
};

