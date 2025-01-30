#pragma once
#include <SFML/Window/Event.hpp>
#include "Action.hpp"
#include "CommandQueue.hpp"
#include "MissionStatus.hpp"
#include <map>

class Command;


class Player
{
public:
	Player(unsigned int player_id,unsigned int joystick_id);
	void HandleEvent(const sf::Event& event, CommandQueue& command_queue);
	void HandleRealTimeInput(CommandQueue& command_queue);

	void AssignKey(Action action, sf::Keyboard::Key key);
	void AssignGamepadButton(Action action, unsigned int button);
	sf::Keyboard::Key GetAssignedKey(Action action) const;
	unsigned int GetAssignedGamepadButton(Action action) const;
	void SetMissionStatus(MissionStatus status);
	MissionStatus GetMissionStatus() const;

private:
	void InitialiseActions();
	static bool IsRealTimeAction(Action action);

private:
	unsigned int m_player_id = 0; // Unique player identifier
	unsigned int m_joystick_id = 0; // Unique joystick ID for this player

	std::map<sf::Keyboard::Key, Action> m_key_binding;
	std::map<Action, Command> m_action_binding;
	MissionStatus m_current_mission_status;

	std::map<unsigned int, Action> m_gamepad_binding;  // Mapping for gamepad buttons
	std::map<sf::Joystick::Axis, Action> m_gamepad_axes; // Mapping for joystick movement

};

