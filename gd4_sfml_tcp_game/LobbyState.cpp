#include "LobbyState.hpp"

LobbyState::LobbyState(StateStack& stack, Context context)
	: State(stack,context)
	, m_gui_container(context)
{
	sf::Texture& texture = context.textures->Get(TextureID::kTitleScreen);

	m_background_sprite.setTexture(texture);
}

void LobbyState::Draw()
{
}

bool LobbyState::Update(sf::Time dt)
{
	return false;
}

bool LobbyState::HandleEvent(const sf::Event& event)
{
	return false;
}
