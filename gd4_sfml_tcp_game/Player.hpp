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
	Player();
	void HandleEvent(const sf::Event& event, CommandQueue& command_queue);
	void HandleRealTimeInput(CommandQueue& command_queue);
	void SetMissionStatus(MissionStatus status);
	MissionStatus GetMissionStatus() const;
	Gamepad& GetGamepad();
	void SetGamepad(Gamepad gamepad);
	unsigned int GetPlayerID() const;
	unsigned int GetPlayerCount() const;


private:
	void InitialiseActions();
	
private:
	static unsigned int m_player_count; // Static member to count players
	unsigned int m_player_id = 0; // Unique player identifier
	MissionStatus m_current_mission_status;
	Gamepad m_gamepad;
};

