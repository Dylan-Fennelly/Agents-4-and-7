/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/
#pragma once
#include <map>
#include <unordered_map>
#include "ControllerType.hpp"
#include "ButtonFunction.hpp"
#include "Action.hpp"
#include "Command.hpp"
#include "CommandQueue.hpp"
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Event.hpp>

/// <summary>
///	This is a wrapper class for an SFML Joystick, allows a gamepad to handle its on mapping of its button to actions
/// </summary>
class Gamepad
{
public :

	/// <summary>
	///	Constructor for the Gamepad
	/// </summary>
	/// <param name="joystick_id">The Id of the joystick </param>
	/// <param name="player_id">The ID of the player that the joystick belongs to</param>
	explicit Gamepad(unsigned int joystick_id, unsigned int player_id);
	/// <summary>
	/// Handle the events for the gamepad
	/// </summary>
	/// <param name="command_queue"></param>
	void Update(CommandQueue& command_queue);
	void HandleEvent(const sf::Event& event, CommandQueue& command_queue);

	/// <summary>
	/// Assing the function of a button to a joysticks button
	/// </summary>
	/// <param name="button">The Button Functionality </param>
	/// <param name="button_id">The Id of the physical controller button</param>
	void AssignButtonFunction(ButtonFunction button, unsigned int button_id);
	/// <summary>
	///	Assigns a game action to Buttonfunction
	/// </summary>
	/// <param name="action">The game Action to be assigned</param>
	/// <param name="function">The Button Function</param>
	void AssignAction(Action action, ButtonFunction function);
	/// <summary>
	/// Assigns a command to an action
	/// </summary>
	/// <param name="action"></param>
	/// <param name="command"></param>
	void AssignCommand(Action action, Command command);

	/// <summary>
	/// Returns the button id for a button function
	/// </summary>
	/// <param name="function">The Button function</param>
	/// <returns>The Id of the Joystick button</returns>
	unsigned int GetButton(ButtonFunction function) const;
	/// <summary>
	///	Returns the Action assigned to a button function
	/// </summary>
	/// <param name="function">The Button Function</param>
	/// <returns>The Action that the Button was assigned to </returns>
	Action GetAction(ButtonFunction function) const;
	/// <summary>
	///	Returns the possition of joystick axis 
	/// </summary>
	/// <param name="axis">The requested Joystick Axis</param>
	/// <returns>The postion of the Joystick Axis </returns>
	float GetAxisPosition(sf::Joystick::Axis axis);
	/// <summary>
	///	Sets the dead zone for the joystick
	/// </summary>
	/// <param name="dead_zone">The dead zone value</param>
	void SetDeadZone(float dead_zone);
	/// <summary>
	/// Returns the Id of the joystick
	/// </summary>
	/// <returns>The Joystick Id</returns>
	unsigned int GetJoystickId() const;

	
private:
	/// <summary>
	/// The Joystick ID
	/// </summary>
	unsigned int m_joystick_id;
	/// <summary>
	/// The Value of the dead zone
	/// </summary>
	float m_dead_zone;
	/// <summary>
	/// An enum representing the controller type
	/// </summary>
	ControllerType m_controller_type;
	/// <summary>
	/// The Id of the player assigned to this Joycon
	/// </summary>
	unsigned int m_player_id;
	/// <summary>
	/// The button bindings for the gamepad
	/// </summary>
	std::unordered_map<ButtonFunction, unsigned int> m_button_bindings;
	/// <summary>
	/// The action bindings for the gamepad
	/// </summary>
	std::unordered_map<Action, ButtonFunction> m_action_function_bindings;
	/// <summary>
	/// The command bindings for the gamepad
	/// </summary>
	std::unordered_map<Action, Command> m_action_command_bindings;
};

