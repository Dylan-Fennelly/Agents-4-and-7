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


class Gamepad
{
public :

	explicit Gamepad(unsigned int joystick_id);

	void Update(CommandQueue& command_queue);
	void HandleEvent(const sf::Event& event, CommandQueue& command_queue);

	//Button,action and commnad mapping
	void AssignButtonFunction(ButtonFunction button, unsigned int button_id);
	void AssignAction(Action action, ButtonFunction function);
	void AssignCommand(Action action, Command command);


	unsigned int GetButton(ButtonFunction function) const;
	Action GetAction(ButtonFunction function) const;
	//Joystick Input
	float GetAxisPosition(sf::Joystick::Axis axis) ;
	void SetDeadZone(float dead_zone);

	unsigned int GetJoystickId() const;

	
private:
	unsigned int m_joystick_id;
	float m_dead_zone;
	ControllerType m_controller_type;

	std::unordered_map<ButtonFunction, unsigned int> m_button_bindings;
	std::unordered_map<Action, ButtonFunction> m_action_function_bindings;
	std::unordered_map<Action, Command> m_action_command_bindings;
};

