#pragma once
#include "State.hpp"
#include "CommandQueue.hpp"
#include <SFML/Graphics.hpp>
#include <set>

class ControllerSelectState : public State
{
public:
    ControllerSelectState(StateStack& stack, Context context);

    virtual void Draw();
    virtual bool Update(sf::Time dt);
    virtual bool HandleEvent(const sf::Event& event);

private:
    sf::Text m_instructionText;
    bool m_player1Registered;
    bool m_player2Registered;
    unsigned int m_player1Joystick;
    unsigned int m_player2Joystick;
};
