#include "MusicPlayer.hpp"

MusicPlayer::MusicPlayer()
	: m_volume(10.f)
{
	/*All music pieces have been self - composed / produced by Albert.Made using Bosca Ceoil Blue.
	The chosen file extension is .wav, an industry standard for editing and playing music files*/

	m_filenames[MusicThemes::kMenuTheme] = "Media/Music/MenuTheme.wav"; /*Reasoning for music choice -
	a dark soundtrack that introduces an unsettling atmosphere since the game takes place during a zombie apocalypse. Additionally,
	you can hear wind gusts in the background, which indicate lack of other human beings in the vicinity.
	The agents can only count on each other.*/

	m_filenames[MusicThemes::kGameplayTheme] = "Media/Music/GameplayTheme.wav";
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