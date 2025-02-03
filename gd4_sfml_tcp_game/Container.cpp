/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "Container.hpp"
#include "Player.hpp"
#include "ButtonFunction.hpp"
//Because we dont have access to the player class we are not able to access the gamepad class - this makes it impossible to get the joystick id from the player class
//We need to add a context to the container class so that we can get the joystick id from the context


gui::Container::Container(State::Context context)
    : m_selected_child(-1), 
    m_last_joystick_move_time(sf::Time::Zero)
	, m_context(context)
{
}

void gui::Container::Pack(Component::Ptr component)
{
    m_children.emplace_back(component);
    if (!HasSelection() && component->IsSelectable())
    { 
        Select(m_children.size() - 1);
    }
}

bool gui::Container::IsSelectable() const
{
    return false;
}
//ToDO:Consider passing the statestack, that way we can use the canel button to leave a menu

void gui::Container::HandleEvent(const sf::Event& event)
{
    // If a child is active, forward the event
    if (HasSelection() && m_children[m_selected_child]->IsActive())
    {
        m_children[m_selected_child]->HandleEvent(event);
        return;
    }


    if (event.type == sf::Event::JoystickMoved)
    {
        HandleJoystickNavigation(event);
    }

	//Here we check if the event is a button press
    else if (event.type == sf::Event::JoystickButtonPressed)
    {
		//Make sure the right player is pressing the right button
        if (event.joystickButton.joystickId == GetContext().player->GetGamepad().GetJoystickId())
        {
            if (event.joystickButton.button == GetContext().player->GetGamepad().GetButton(ButtonFunction::kConfirm))
            {
                if (HasSelection())
                {
                    m_children[m_selected_child]->Activate();
                }
            }
        }
        else if (event.joystickButton.joystickId == GetContext().player2->GetGamepad().GetJoystickId())
        {
            if (event.joystickButton.button == GetContext().player2->GetGamepad().GetButton(ButtonFunction::kConfirm))
            {
                if (HasSelection())
                {
                    m_children[m_selected_child]->Activate();
                }
            }
        }

    }
}

State::Context gui::Container::GetContext() const
{
	return m_context;
}
//Todo: this needs to be passed off the gamepad class
void gui::Container::HandleJoystickNavigation(const sf::Event& event)
{
    constexpr float threshold = 50.f; // Deadzone threshold to prevent accidental movement
    sf::Time delay = sf::milliseconds(200); // Delay between navigation changes

    if (event.joystickMove.axis == sf::Joystick::PovX || event.joystickMove.axis == sf::Joystick::Y)
    {
        sf::Time now = m_clock.getElapsedTime();
        if (now - m_last_joystick_move_time < delay)
            return; // Prevent rapid switching

        if (event.joystickMove.position < -threshold) // Up
        {
            SelectPrevious();
            m_last_joystick_move_time = now;
        }
        else if (event.joystickMove.position > threshold) // Down
        {
            SelectNext();
            m_last_joystick_move_time = now;
        }
    }
}

void gui::Container::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    for (const Component::Ptr& child : m_children)
    {
        target.draw(*child, states);
    }
}

bool gui::Container::HasSelection() const
{
    return m_selected_child >= 0;
}

void gui::Container::Select(std::size_t index)
{
    if (index < m_children.size() && m_children[index]->IsSelectable())
    {
        if (HasSelection())
        {
            m_children[m_selected_child]->Deselect();
        }
        m_children[index]->Select();
        m_selected_child = index;
    }
}

void gui::Container::SelectNext()
{
    if (!HasSelection())
    {
        return;
    }
    int next = m_selected_child;
    do
    {
        next = (next + 1) % m_children.size();
    } while (!m_children[next]->IsSelectable());
    Select(next);
}

void gui::Container::SelectPrevious()
{
    if (!HasSelection())
    {
        return;
    }
    int prev = m_selected_child;
    do
    {
        prev = (prev + m_children.size() - 1) % m_children.size();
    } while (!m_children[prev]->IsSelectable());
    Select(prev);
}
