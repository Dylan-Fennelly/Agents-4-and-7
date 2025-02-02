#include "MusicPlayer.hpp"

MusicPlayer::MusicPlayer()
	: m_volume(10.f)
{
	/*All music pieces have been self-composed/produced by Albert. Made using Bosca Ceoil Blue.
	The chosen file extension is .wav, an industry standard for editing and playing music files*/

	m_filenames[MusicThemes::kMenuTheme] = "Media/Music/MenuTheme.wav"; /*Reasoning for music choice -
	a dark soundtrack that introduces an unsettling atmosphere since the game takes place during a zombie apocalypse. Additionally,
	you can hear wind gusts in the background, which indicate lack of other human beings in the vicinity.
	The agents can only count on each other.*/

	m_filenames[MusicThemes::kGameplayTheme] = "Media/Music/GameplayTheme.wav"; /*Reasoning for music choice - 
	although the menu theme is darker than the gameplay theme, it is an intentional choice. I did not want the player to be bored
	whilst slaying hordes of zombies. I wanted it to be something more upbeat to give an illusion of a faster gameplay.
	In the intro, there are noises in the background, which are supposed to immitate zombie sounds.
	This track has been slightly inspired by Plants vs Zombies.*/

	m_filenames[MusicThemes::kGameOverTheme] = "Media/Music/GameOverTheme.wav";

	m_filenames[MusicThemes::kGameWonTheme] = "Media/Music/GameWonTheme.wav";
	//https://pixabay.com/sound-effects/horn-stabs-entrance-14741/
	//https://pixabay.com/sound-effects/victorymale-version-230553/
}

void MusicPlayer::Play(MusicThemes theme)
{
	std::string filename = m_filenames[theme];

	if (!m_music.openFromFile(filename))
		throw std::runtime_error("Music " + filename + " could not be loaded.");

	m_music.setVolume(m_volume);
	m_music.setLoop(true);
	m_music.play();
}

void MusicPlayer::Stop()
{
	m_music.stop();
}

void MusicPlayer::SetVolume(float volume)
{
	m_volume = volume;
}

void MusicPlayer::SetPaused(bool paused)
{
	if (paused)
		m_music.pause();
	else
		m_music.play();
}