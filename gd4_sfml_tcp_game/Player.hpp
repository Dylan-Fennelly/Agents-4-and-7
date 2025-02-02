/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include <SFML/Window/Event.hpp>
#include "Action.hpp"
#include "CommandQueue.hpp"
#include "MissionStatus.hpp"
#include <map>
#include "Gamepad.hpp"
class Command;


class Player
{
public:
	Player(unsigned int player_id,unsigned int joystick_id);
	void HandleEvent(const sf::Event& event, CommandQueue& command_queue);
	void HandleRealTimeInput(CommandQueue& command_queue);
	void SetMissionStatus(MissionStatus status);
	MissionStatus GetMissionStatus() const;
	Gamepad& GetGamepad();
	void SetGamepad(Gamepad gamepad);

	

private:
	void InitialiseActions();

private:
	unsigned int m_player_id = 0; // Unique player identifier
	MissionStatus m_current_mission_status;
	Gamepad m_gamepad;
};

