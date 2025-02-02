#include "Gamepad.hpp"
#include <SFML/Window/Joystick.hpp>
#include "CommandQueue.hpp"
#include <SFML/Window/Event.hpp>
#include "StateStack.hpp"
#include "Utility.hpp"
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
		m_button_bindings =
		{
			{ ButtonFunction::kConfirm, 0 },
			{ ButtonFunction::kCancel, 1 },
			{ ButtonFunction::kPause, 7}
		};
	}
	else if (name.find("Wireless Controller") != std::string::npos)
	{
		m_controller_type = ControllerType::kPs4;
		m_button_bindings =
		{
			{ ButtonFunction::kConfirm, 1 },
			{ ButtonFunction::kCancel, 2 },
			{ ButtonFunction::kPause, 9 }
		};
	}
	else
	{
		m_controller_type = ControllerType::kUnknown; 
		m_button_bindings = //We are going to assume that an unknown controller is an Xbox controller, as it is the most common controller type
		{
			{ ButtonFunction::kConfirm, 0 },
			{ ButtonFunction::kCancel, 1 },
			{ ButtonFunction::kPause, 7 }
		};
	}

}

void Gamepad::HandleEvent(const sf::Event& event, CommandQueue& command_queue)
{
	if (event.type == sf::Event::JoystickButtonPressed)
	{
		// Iterate over each action mapped to a button function.
		for (const auto& pair : m_action_function_bindings)
		{
			Action action = pair.first;
			ButtonFunction function = pair.second;
			// Only process if the physical button pressed matches our mapping…
			if (m_button_bindings[function] == event.joystickButton.button)
			{
				// …and only if the action is NOT a real-time action.
				if (!Utility::IsRealTimeAction(action))
				{
					auto it = m_action_command_bindings.find(action);
					if (it != m_action_command_bindings.end())
					{
						command_queue.Push(it->second);
					}
				}
			}
		}
	}
}

void Gamepad::Update(CommandQueue& command_queue)
{
	// Process only real-time actions here.
	for (const auto& binding : m_action_function_bindings)
	{
		Action action = binding.first;
		if (!Utility::IsRealTimeAction(action))
			continue;  // Skip discrete actions in the update loop

		ButtonFunction function = binding.second;
		auto it = m_button_bindings.find(function);
		if (it != m_button_bindings.end() && sf::Joystick::isButtonPressed(m_joystick_id, it->second))
		{
			auto cmdIt = m_action_command_bindings.find(action);
			if (cmdIt != m_action_command_bindings.end())
			{
				command_queue.Push(cmdIt->second);
			}
		}
	}

	// Handle joystick axis movement as before:
	float axisX = GetAxisPosition(sf::Joystick::X);
	float axisY = GetAxisPosition(sf::Joystick::Y);

	if (std::abs(axisX) > m_dead_zone)
	{
		Action moveAction = axisX > 0 ? Action::kMoveRight : Action::kMoveLeft;
		if (m_action_command_bindings.find(moveAction) != m_action_command_bindings.end())
		{
			command_queue.Push(m_action_command_bindings[moveAction]);
		}
	}
	if (std::abs(axisY) > m_dead_zone)
	{
		Action moveAction = axisY > 0 ? Action::kMoveDown : Action::kMoveUp;
		if (m_action_command_bindings.find(moveAction) != m_action_command_bindings.end())
		{
			command_queue.Push(m_action_command_bindings[moveAction]);
		}
	}
}


void Gamepad::AssignButtonFunction(ButtonFunction function, unsigned int button)
{
	m_button_bindings[function] = button;
}
void Gamepad::AssignAction(Action action, ButtonFunction function)
{
	m_action_function_bindings[action] = function;
}

void Gamepad::AssignCommand(Action action, Command command)
{
	m_action_command_bindings[action] = command;
}

unsigned int Gamepad::GetButton(ButtonFunction function) const
{
	auto it = m_button_bindings.find(function);
	return (it != m_button_bindings.end()) ? it->second : 0; // Default to button 0 if not found
}

Action Gamepad::GetAction(ButtonFunction function) const
{
	//Iterate over the action function bindings and return
	for (const auto& pair : m_action_function_bindings)
	{
		if (pair.second == function)
		{
			return pair.first;
		}
	}

}


float Gamepad::GetAxisPosition(sf::Joystick::Axis axis) 
{
	//Because our we are not relying on the range of the axis for movement we can return the position of the axis directly(we are basically treating it like a button)
	//If we do switch to the stick controling something like movement speed in the future we need to use the dead zone and the max range and convert the axis position to a value between -100 and 100
	//This will allow us to use the axis position to control the speed of the player or some similer value
	//We first need to check if the axis is within the range of the number of axes that the controller has
	if (axis < sf::Joystick::AxisCount)
	{
		//We get the position of the axis
		float position = sf::Joystick::getAxisPosition(m_joystick_id,axis);
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

void Gamepad::SetDeadZone(float dead_zone)
{
	//We set the deadzone to what ever the player passed in but ensure that it is slightly lower than the maximum range to ensure that the joystick can still be moved
	//Todo: Remove the magic number 80.f , this is the maximum value that the dead zone can be set to
	m_dead_zone = std::min(dead_zone, 80.f);
}


unsigned int Gamepad::GetJoystickId() const
{
	return m_joystick_id;
}
