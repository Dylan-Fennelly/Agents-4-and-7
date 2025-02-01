#pragma once
#include <map>
#include <unordered_map>
#include "ControllerType.hpp"
#include "ButtonFunction.hpp"
#include "Action.hpp"
/// <summary>
/// This class is wrapper for the Sfml Joystick class, it is used to allow mapping of buttons to a type of functionality 
/// Provides an interface to other classes to get the position of the axis and the button that is bound to a function
/// </summary>
class Gamepad
{
public :
	/// <summary>
	/// Constructor for an instance of a players gamepad
	/// </summary>
	/// <param name="joystick_id">The ID of the gamepad allocated by Sfml</param>
	Gamepad(unsigned int joystick_id);

	/// <summary>
	/// Returns the button that is bound to the given function
	/// </summary>
	/// <param name="button">An Enum representing the function of the button</param>
	/// <returns>An integer representing the button that is bound to the given function</returns>
	int getButton(ButtonFunction button);


	/// <summary>
	/// Sets the button that is bound to the given function
	/// </summary>
	/// <param name="button">The function of the button</param>
	/// <param name="button_id">The button that the function will be bound to</param>
	void setButton(ButtonFunction button, unsigned int button_id);

	unsigned int getJoystickId() const;

	/// <summary>
	/// Returns the position of the axis given the axis number
	/// </summary>
	/// <param name="axis">The Axis of a joystick or analogue button</param>
	/// <returns>Returns a float representing the postion of the axis in the range: -100/100</returns>
	float getAxisPosition(int axis);

	/// <summary>
	/// Sets the dead zone of the gamepad, this is the range of the joystick that is ignored
	/// </summary>
	/// <param name="dead_zone">The value of the deadzone - This prevents the game recognising movement when the joystick is in the centre</param>
	void setDeadZone(float dead_zone);

	void addActionMap(Action action, ButtonFunction button);
	ButtonFunction getButtonAction(Action action);

	
private:
	/// <summary>
	/// The ID of the gamepad allocated by Sfml
	/// </summary>
	unsigned int m_joystick_id;
	/// <summary>
	/// The dead zone of the gamepad, this is the range of the joystick that is ignored
	/// Has a default value of 15.f
	/// </summary>
	float m_dead_zone;
	/// <summary>
	/// The type of controller that the gamepad is
	/// </summary>
	ControllerType m_controller_type;
	/// <summary>
	/// A map of the buttons that are bound to the functions of the gamepad
	/// </summary>
	std::unordered_map<ButtonFunction, unsigned int> m_button_binding;
	std::unordered_map<Action, ButtonFunction> m_action_function_binding;
};

