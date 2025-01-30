#include "Player.hpp"
#include "ReceiverCategories.hpp"
#include "Aircraft.hpp"
#include <iostream>

struct AircraftMover
{
    AircraftMover(float vx, float vy) :velocity(vx, vy)
    {}
    void operator()(Aircraft& aircraft, sf::Time) const
    {
        aircraft.Accelerate(velocity);
    }

    sf::Vector2f velocity;
};

Player::Player(unsigned int player_id, unsigned int joystick_id)
    : m_player_id(player_id) // Assign player ID
    , m_joystick_id(joystick_id) // Assign joystick ID
    , m_current_mission_status(MissionStatus::kMissionRunning)
{
    //Set initial key bindings
    m_key_binding[sf::Keyboard::A] = Action::kMoveLeft;
    m_key_binding[sf::Keyboard::D] = Action::kMoveRight;
    m_key_binding[sf::Keyboard::W] = Action::kMoveUp;
    m_key_binding[sf::Keyboard::S] = Action::kMoveDown;
    m_key_binding[sf::Keyboard::M] = Action::kMissileFire;
    m_key_binding[sf::Keyboard::Space] = Action::kBulletFire;


    // Set gamepad button bindings (assuming PLaystation layout)
	m_gamepad_binding[1] = Action::kBulletFire;  // X Button
	m_gamepad_binding[2] = Action::kMissileFire; // Circle Button
    //m_gamepad_binding[0] = Action::kBulletFire;  // A Button
    //m_gamepad_binding[1] = Action::kMissileFire; // B Button

    // Set joystick axis bindings
    m_gamepad_axes[sf::Joystick::X] = Action::kMoveRight;
    m_gamepad_axes[sf::Joystick::Y] = Action::kMoveUp;

    //Set initial action bindings
    InitialiseActions();

    //Assign all categories to a player's aircraft
    for (auto& pair : m_action_binding)
    {
        pair.second.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    }
}

void Player::HandleEvent(const sf::Event& event, CommandQueue& command_queue)
{
    if (event.type == sf::Event::KeyPressed)
    {
        auto found = m_key_binding.find(event.key.code);
        if (found != m_key_binding.end() && !IsRealTimeAction(found->second))
        {
            command_queue.Push(m_action_binding[found->second]);
        }
    }

    // Handle gamepad button presses
    if (event.type == sf::Event::JoystickButtonPressed)
    {
        auto found = m_gamepad_binding.find(event.joystickButton.button);
		std::cout << "Button pressed: " << event.joystickButton.button << std::endl;
        if (found != m_gamepad_binding.end() && !IsRealTimeAction(found->second))
        {
            command_queue.Push(m_action_binding[found->second]);
        }
    }
}

void Player::HandleRealTimeInput(CommandQueue& command_queue)
{
    //Check if any of the key bindings are pressed
    for (auto pair : m_key_binding)
    {
        if (sf::Keyboard::isKeyPressed(pair.first) && IsRealTimeAction(pair.second))
        {
            command_queue.Push(m_action_binding[pair.second]);
        }
    }


    // Check gamepad button input
    for (auto pair : m_gamepad_binding)
    {
        if (sf::Joystick::isButtonPressed(m_joystick_id, pair.first) && IsRealTimeAction(pair.second))
        {
            command_queue.Push(m_action_binding[pair.second]);
        }
    }

    // Check joystick axis movement
    const float deadZone = 15.f; // Dead zone to prevent unintentional movement

    for (auto pair : m_gamepad_axes)
    {
        float axis_position = sf::Joystick::getAxisPosition(m_joystick_id, pair.first);

        if (std::abs(axis_position) > deadZone) // Ignore small movements
        {
            if (pair.first == sf::Joystick::X)
            {
                command_queue.Push(axis_position > 0 ? m_action_binding[Action::kMoveRight] : m_action_binding[Action::kMoveLeft]);
            }
            else if (pair.first == sf::Joystick::Y)
            {
                command_queue.Push(axis_position > 0 ? m_action_binding[Action::kMoveDown] : m_action_binding[Action::kMoveUp]);
            }
        }
    }
}

void Player::AssignKey(Action action, sf::Keyboard::Key key)
{
    //Remove keys that are currently bound to the action
    for (auto itr = m_key_binding.begin(); itr != m_key_binding.end();)
    {
        if (itr->second == action)
        {
            m_key_binding.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }
    m_key_binding[key] = action;
}

void Player::AssignGamepadButton(Action action, unsigned int button)
{
    // Remove old binding
    for (auto it = m_gamepad_binding.begin(); it != m_gamepad_binding.end();)
    {
        if (it->second == action)
            it = m_gamepad_binding.erase(it);
        else
            ++it;
    }
    m_gamepad_binding[button] = action;
}

sf::Keyboard::Key Player::GetAssignedKey(Action action) const
{
    for (auto pair : m_key_binding)
    {
        if (pair.second == action)
        {
            return pair.first;
        }
    }
    return sf::Keyboard::Unknown;
}

unsigned int Player::GetAssignedGamepadButton(Action action) const
{
    for (auto pair : m_gamepad_binding)
    {
        if (pair.second == action)
            return pair.first;
    }
    return -1; // No binding found
}


void Player::SetMissionStatus(MissionStatus status)
{
    m_current_mission_status = status;
}

MissionStatus Player::GetMissionStatus() const
{
    return m_current_mission_status;
}

void Player::InitialiseActions()
{
    const float kPlayerSpeed = 200.f;
    m_action_binding[Action::kMoveLeft].action = DerivedAction<Aircraft>(AircraftMover(-kPlayerSpeed, 0.f));
    m_action_binding[Action::kMoveRight].action = DerivedAction<Aircraft>(AircraftMover(kPlayerSpeed, 0.f));
    m_action_binding[Action::kMoveUp].action = DerivedAction<Aircraft>(AircraftMover(0.f, -kPlayerSpeed));
    m_action_binding[Action::kMoveDown].action = DerivedAction<Aircraft>(AircraftMover(0.f, kPlayerSpeed));
    m_action_binding[Action::kBulletFire].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time dt)
        {
            a.Fire();
        }
    );

    m_action_binding[Action::kMissileFire].action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time dt)
        {
            a.LaunchMissile();
        }
    );

}

bool Player::IsRealTimeAction(Action action)
{
    switch (action)
    {
    case Action::kMoveLeft:
    case Action::kMoveRight:
    case Action::kMoveDown:
    case Action::kMoveUp:
    case Action::kBulletFire:
        return true;
    default:
        return false;
    }
}
