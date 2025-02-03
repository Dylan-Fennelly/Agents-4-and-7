/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/
#pragma once
#include "State.hpp"
#include "CommandQueue.hpp"
#include <SFML/Graphics.hpp>
#include <set>
/// <summary>
/// Represents the state where the players can select their controller
/// </summary>
class ControllerSelectState : public State
{
public:
    /// <summary>
	/// Constructor for the ControllerSelectState
    /// </summary>
    /// <param name="stack">The State Stack</param>
    /// <param name="context">The game Context</param>
    ControllerSelectState(StateStack& stack, Context context);

    /// <summary>
	/// Draw the ControllerSelectState
    /// </summary>
    virtual void Draw();
    /// <summary>
	/// Update the ControllerSelectState
    /// </summary>
    /// <param name="dt">Delta Time
    /// <returns></returns>
    virtual bool Update(sf::Time dt);
    /// <summary>
	/// Handle the events for the ControllerSelectState
    /// </summary>
    /// <param name="event"></param>
    /// <returns></returns>
    virtual bool HandleEvent(const sf::Event& event);

private:

    /// <summary>
	/// Updates the instruction text
    /// </summary>
    sf::Text m_instructionText;
    /// <summary>
	/// flag for if the player has registered their controller
    /// </summary>
    bool m_player1Registered;
    /// <summary>
	/// flag for if the player has registered their controller
    /// </summary>
    bool m_player2Registered;
    /// <summary>
	/// The joystick for player 1
    /// </summary>
    unsigned int m_player1Joystick;
    /// <summary>
	/// The joystick for player 2
    /// </summary>
    unsigned int m_player2Joystick;
};
