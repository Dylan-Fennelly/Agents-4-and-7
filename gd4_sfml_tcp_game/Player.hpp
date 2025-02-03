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

/// <summary>
/// Class that represents a player
/// </summary>
class Player
{
public:
	/// <summary>
	/// Default constructor for the player
	/// </summary>
	Player();
	/// <summary>
	///	Hanldes the events for the player(Are passed to the gamepad to Handle)
	/// </summary>
	/// <param name="event">The event to handle</param>
	/// <param name="command_queue">The command queue to push commands to</param>
	void HandleEvent(const sf::Event& event, CommandQueue& command_queue);
	/// <summary>
	///	Handles the real time input for the player(Passed to the gamepad)
	/// </summary>
	/// <param name="command_queue">The command queue to push commands to</param>
	void HandleRealTimeInput(CommandQueue& command_queue);
	/// <summary>
	/// Sets the current mission status
	/// </summary>
	/// <param name="status">The status to set</param>
	void SetMissionStatus(MissionStatus status);
	/// <summary>
	/// Gets the current mission status
	/// </summary>
	/// <returns>The players current misson status</returns>
	MissionStatus GetMissionStatus() const;
	/// <summary>
	/// Get's the players assigned gamepad
	/// </summary>
	/// <returns>The GamePad</returns>
	Gamepad& GetGamepad();
	/// <summary>
	/// Sets the players Assigned gamepad
	/// </summary>
	/// <param name="gamepad">The gamepad the player will be assigned</param>
	void SetGamepad(Gamepad gamepad);
	/// <summary>
	/// Gets this players unique identifier
	/// </summary>
	/// <returns>The Players Id</returns>
	unsigned int GetPlayerID() const;
	/// <summary>
	/// Gets the total amount of players
	/// </summary>
	/// <returns></returns>
	unsigned int GetPlayerCount() const;


private:
	/// <summary>
	/// Sets up the bindings for Commands to actions
	/// </summary>
	void InitialiseActions();
	
private:
	/// <summary>
	///	Globally unique player count
	/// </summary>
	static unsigned int m_player_count; // Static member to count players
	/// <summary>
	/// The player's ID
	/// </summary>
	unsigned int m_player_id = 0; // Unique player identifier
	/// <summary>
	/// The Players current mission status
	/// </summary>
	MissionStatus m_current_mission_status;
	/// <summary>
	///	The players gamepad
	/// </summary>
	Gamepad m_gamepad;
};

