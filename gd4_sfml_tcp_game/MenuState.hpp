/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include "State.hpp"
#include "MenuOptions.hpp"
#include "Container.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <vector>
#include "TextBox.hpp"


class MenuState : public State
{
public:
	MenuState(StateStack& stack, Context context);
	virtual void Draw() override;
	virtual bool Update(sf::Time dt) override;
	virtual bool HandleEvent(const sf::Event& event) override;

private:
	sf::Sprite m_background_sprite;
	gui::Container m_gui_container;

	TextBox m_text_box;
	bool m_text_box_Selected;
};

