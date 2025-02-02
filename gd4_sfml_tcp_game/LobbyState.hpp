#pragma once
#include "State.hpp"
#include "Player.hpp"
#include "Container.hpp"
#include "Button.hpp"
#include "Label.hpp"
#include <SFML/Graphics/Sprite.hpp>

#include <array>


class LobbyState : public State
{
public:
	LobbyState(StateStack& stack, Context context);
	virtual void Draw() override;
	virtual bool Update(sf::Time dt) override;
	virtual bool HandleEvent(const sf::Event& event);

private:
	sf::Sprite m_background_sprite;
	gui::Container m_gui_container;


};

