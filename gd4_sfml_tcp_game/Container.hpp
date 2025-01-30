#pragma once
#include "Component.hpp"
#include <SFML/System/Clock.hpp> // Add this for timing

namespace gui
{
	class Container : public Component
	{
	public:
		typedef std::shared_ptr<Container> Ptr;

	public:
		Container();
		void Pack(Component::Ptr component);
		virtual bool IsSelectable() const override;
		virtual void HandleEvent(const sf::Event& event) override;

		void HandleJoystickNavigation(const sf::Event& event);

	private:
		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
		bool HasSelection() const;
		void Select(std::size_t index);
		void SelectNext();
		void SelectPrevious();

	private:
		std::vector<Component::Ptr> m_children;
		int m_selected_child;

		// Added for controller support
		sf::Clock m_clock;                // Tracks time for joystick movement
		sf::Time m_last_joystick_move_time; // Last movement time to prevent rapid selection
	};
}
