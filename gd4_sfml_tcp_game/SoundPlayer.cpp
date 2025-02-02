/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "SoundPlayer.hpp"

#include "SoundEffect.hpp"

#include <SFML/Audio/Listener.hpp>

#include <cmath>


namespace
{
	// Sound coordinate system, point of view of a player in front of the screen:
	// X = left; Y = up; Z = back (out of the screen)
	const float ListenerZ = 300.f;
	const float Attenuation = 1.f;
	const float MinDistance2D = 200.f;
	const float MinDistance3D = std::sqrt(MinDistance2D * MinDistance2D + ListenerZ * ListenerZ);
}

//Modified by Albert
SoundPlayer::SoundPlayer()
{
	//All sounds have been added by Albert, except for the kButton, which I did not change. All sounds were chosen on the basis of two facts: royalty-free and fitting the game's theme
	//Source - https://pixabay.com/sound-effects/pistol-shot-233473/
	m_sound_buffers.Load(SoundEffect::kPistolShot, "Media/Sound/PistolShot.wav");
	//Source - https://pixabay.com/sound-effects/pistol-shot-233473/
	m_sound_buffers.Load(SoundEffect::kEnemyGunfire, "Media/Sound/PistolShot.wav");
	//Source - https://pixabay.com/sound-effects/zombie-6851/
	m_sound_buffers.Load(SoundEffect::kDyingZombie, "Media/Sound/DyingZombie.wav");
	//Source - https://pixabay.com/sound-effects/zombie-6851/
	m_sound_buffers.Load(SoundEffect::kDyingZombie2, "Media/Sound/DyingZombie.wav");
	//Source - https://pixabay.com/sound-effects/rpg-7-sound-effect-267739/
	m_sound_buffers.Load(SoundEffect::kLaunchRocket, "Media/Sound/LaunchRocket.wav");
	//Source - https://pixabay.com/sound-effects/item-pickup-37089/
	m_sound_buffers.Load(SoundEffect::kCollectPickup, "Media/Sound/CollectPickup.wav");
	m_sound_buffers.Load(SoundEffect::kButton, "Media/Sound/Button.wav");

	// Listener points towards the screen (default in SFML)
	sf::Listener::setDirection(0.f, 0.f, -1.f);
}

void SoundPlayer::Play(SoundEffect effect)
{
	Play(effect, GetListenerPosition());
}

void SoundPlayer::Play(SoundEffect effect, sf::Vector2f position)
{
	m_sounds.emplace_back(sf::Sound());
	sf::Sound& sound = m_sounds.back();

	sound.setBuffer(m_sound_buffers.Get(effect));
	sound.setPosition(position.x, -position.y, 0.f);
	sound.setAttenuation(Attenuation);
	sound.setMinDistance(MinDistance3D);
	sound.setVolume(100.f);

	sound.play();
}

void SoundPlayer::RemoveStoppedSounds()
{
	m_sounds.remove_if([](const sf::Sound& s)
		{
			return s.getStatus() == sf::Sound::Stopped;
		});
}

void SoundPlayer::SetListenerPosition(sf::Vector2f position)
{
	sf::Listener::setPosition(position.x, -position.y, ListenerZ);
}

sf::Vector2f SoundPlayer::GetListenerPosition() const
{
	sf::Vector3f position = sf::Listener::getPosition();
	return sf::Vector2f(position.x, -position.y);
}
