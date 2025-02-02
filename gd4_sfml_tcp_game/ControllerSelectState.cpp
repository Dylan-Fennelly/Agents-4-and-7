#include "ControllerSelectState.hpp"
#include "Application.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"
#include "ReceiverCategories.hpp"
#include <sstream>

ControllerSelectState::ControllerSelectState(StateStack& stack, Context context)
    : State(stack, context)
    , m_player1Registered(false)
    , m_player2Registered(false)
{
    // Set up instruction text (using your font from context)
    m_instructionText.setFont(context.fonts->Get(Font::kMain));
    m_instructionText.setString("Player 1: Press any button on your controller");
    Utility::CentreOrigin(m_instructionText);
    m_instructionText.setPosition(context.window->getView().getSize() / 2.f);
}

bool ControllerSelectState::HandleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::JoystickButtonPressed)
    {
        unsigned int joystickId = event.joystickButton.joystickId;
        // If this joystick hasn't been registered, register it
        if (!m_player1Registered)
        {
            m_player1Registered = true;
            m_player1Joystick = joystickId;

            // Clear the instruction text for Player 1
            m_instructionText.setString("Player 2: Press any button on your controller");
            Utility::CentreOrigin(m_instructionText);
        }
        else if (!m_player2Registered && joystickId != m_player1Joystick)
        {
            m_player2Registered = true;
            m_player2Joystick = joystickId;
        }
    }

    // Once both players have registered, push the next state (e.g. MenuState or GameState)
    if (m_player1Registered && m_player2Registered)
    {
        // Store the joystick IDs in the context (or in the Player objects)
        GetContext().player->SetGamepad(Gamepad(m_player1Joystick, GetContext().player->GetPlayerID()));
        GetContext().player2->SetGamepad(Gamepad(m_player2Joystick, GetContext().player2->GetPlayerID()));

        // Pop the controller selection state and push the next state.
        RequestStackPop();
        RequestStackPush(StateID::kMenu);
    }

    return true;
}

void ControllerSelectState::Draw()
{
    sf::RenderWindow& window = *GetContext().window;
    window.clear();

    // Draw the instruction text only if it's not empty
    if (!m_instructionText.getString().isEmpty()) {
        window.draw(m_instructionText);
    }
}

bool ControllerSelectState::Update(sf::Time)
{
    return true;
}
