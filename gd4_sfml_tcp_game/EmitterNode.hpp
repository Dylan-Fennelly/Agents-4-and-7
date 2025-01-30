/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include "SceneNode.hpp"
#include "ParticleType.hpp"
#include "ParticleNode.hpp"

class EmitterNode : public SceneNode
{
public:
	explicit EmitterNode(ParticleType type);

private:
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands);
	void EmitParticles(sf::Time dt);

private:
	sf::Time m_accumulated_time;
	ParticleType m_type;
	ParticleNode* m_particle_system;
};

