/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "KeyBinding.hpp"
#include "Player.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "StateStack.hpp"
#include "MusicPlayer.hpp"
#include "SoundPlayer.hpp"

class Application
{
public:
	Application();
	void Run();

private:
	void ProcessInput();
	void Update(sf::Time dt);
	void Render();
	void RegisterStates();

private:
	sf::RenderWindow m_window;

	TextureHolder m_textures;
	FontHolder m_fonts;

	StateStack m_stack;
	static const sf::Time kTimePerFrame;

	MusicPlayer m_music;
	SoundPlayer m_sound;

	KeyBinding m_key_binding_1;
	KeyBinding m_key_binding_2;
};

