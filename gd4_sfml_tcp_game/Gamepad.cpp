#include "Gamepad.hpp"
#include <SFML/Window/Joystick.hpp>
Gamepad::Gamepad(unsigned int joystick_id)
	: m_joystick_id(joystick_id)
	, m_dead_zone(15.f)
{
	//we have a big problem here as we are assuming that a com
	sf::Joystick::Identification id = sf::Joystick::getIdentification(joystick_id);
	std::string name = id.name.toAnsiString();
	//The Below sets up a default binding for different controllers 
	if (name.find("Xbox") != std::string::npos)
	{
		m_controller_type = ControllerType::kXbox;
		m_button_binding =
		{
			{ ButtonFunction::kConfirm, 0 },
			{ ButtonFunction::kCancel, 1 },
			{ ButtonFunction::kPause, 7}
		};
	}
	else if (name.find("Wireless Controller") != std::string::npos)
	{
		m_controller_type = ControllerType::kPs4;
		m_button_binding =
		{
			{ ButtonFunction::kConfirm, 1 },
			{ ButtonFunction::kCancel, 2 },
			{ ButtonFunction::kPause, 9 }
		};
	}
	else
	{
		m_controller_type = ControllerType::kUnknown; 
		m_button_binding = //We are going to assume that an unknown controller is an Xbox controller, as it is the most common controller type
		{
			{ ButtonFunction::kConfirm, 0 },
			{ ButtonFunction::kCancel, 1 },
			{ ButtonFunction::kPause, 7 }
		};
	}

}

int Gamepad::getButton(ButtonFunction button)
{
	auto it = m_button_binding.find(button);
	return (it != m_button_binding.end()) ? it->second : -1;
}
float Gamepad::getAxisPosition(int axis)
{
	//Because our we are not relying on the range of the axis for movement we can return the position of the axis directly(we are basically treating it like a button)
	//If we do switch to the stick controling something like movement speed in the future we need to use the dead zone and the max range and convert the axis position to a value between -100 and 100
	//This will allow us to use the axis position to control the speed of the player or some similer value
	//We first need to check if the axis is within the range of the number of axes that the controller has
	if (axis < sf::Joystick::AxisCount)
	{
		//We get the position of the axis
		float position = sf::Joystick::getAxisPosition(m_joystick_id, static_cast<sf::Joystick::Axis>(axis));
		//We then check if the position is within the dead zone
		if (position < m_dead_zone && position > -m_dead_zone)
		{
			//If it is we return 0
			return 0.f;
		}
		//If it is not we return the position
		return position;
	}
}

void Gamepad::setDeadZone(float dead_zone)
{
	//We set the deadzone to what ever the player passed in but ensure that it is slightly lower than the maximum range to ensure that the joystick can still be moved
	//Todo: Remove the magic number 80.f , this is the maximum value that the dead zone can be set to
	m_dead_zone = dead_zone < 100.f ? dead_zone : 80.f;
}

void Gamepad::addActionMap(Action action, ButtonFunction button)
{
	//We first need to check if the action is bound to another button 
	auto it = m_action_function_binding.find(action);
	if (it != m_action_function_binding.end())
	{
		//If it is we need to remove the binding
		m_action_function_binding.erase(it);
	}
	//Now we can bind the action to the new button
	m_action_function_binding[action] = button;
}

ButtonFunction Gamepad::getButtonAction(Action action)
{
	//We first need to check if the action is bound to a button
	auto it = m_action_function_binding.find(action);
	return (it != m_action_function_binding.end()) ? it->second : ButtonFunction::kButtonCount;
}

void Gamepad::setButton(ButtonFunction button, unsigned int button_id)
{
	//We first need to check if the function is bound to another button 
	auto it = m_button_binding.find(button);
	if (it != m_button_binding.end())
	{
		//If it is we need to remove the binding
		m_button_binding.erase(it);
	}
	//Now we can bind the function to the new button
	m_button_binding[button] = button_id;

}

unsigned int Gamepad::getJoystickId() const
{
	return m_joystick_id;
}
