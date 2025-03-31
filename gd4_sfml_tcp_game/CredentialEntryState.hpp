#pragma once
#pragma once
#include "State.hpp"
#include "MenuOptions.hpp"
#include "Container.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <vector>
#include "TextBox.hpp"
class CredentialEntryState : public State
{
public:
	CredentialEntryState(StateStack& stack, Context context,bool is_host);
	virtual void Draw() override;
	virtual bool Update(sf::Time dt) override;
	virtual bool HandleEvent(const sf::Event& event) override;

private:
	sf:: Sprite m_background_sprite;
	gui::Container m_gui_container;

	std::vector<TextBox> m_text_boxes;
	int m_text_box_Selected = -1;
	bool m_is_host;

};

