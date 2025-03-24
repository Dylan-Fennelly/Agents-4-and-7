/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include <SFML/Window/Event.hpp>
#include "Command.hpp"
#include "Action.hpp"
#include "CommandQueue.hpp"
#include "MissionStatus.hpp"
#include <map>
#include "KeyBinding.hpp"
#include "Gamepad.hpp"
#include <SFML/Network/TcpSocket.hpp>
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
	Player(sf::TcpSocket* socket, sf::Int32 identifier,  KeyBinding* binding);
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
	/// Handle real time input in a networked environment
	/// </summary>
	/// <param name="commands"></param>
	void HandleRealtimeNetworkInput(CommandQueue& commands);


	void HandleNetworkEvent(Action action, CommandQueue& commands);

	/// <summary>
	/// Handle Real Time Changes when in a networked environment
	/// </summary>
	/// <param name="action"></param>
	/// <param name="action_enabled"></param>
	void HandleNetworkRealtimeChange(Action action, bool action_enabled);
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
	//Gamepad& GetGamepad();
	/// <summary>
	/// Sets the players Assigned gamepad
	/// </summary>
	/// <param name="gamepad">The gamepad the player will be assigned</param>
	//void SetGamepad(Gamepad gamepad);
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

	void DisableAllRealtimeActions();
	bool IsLocal() const;
	KeyBinding * GetKeyBinding() ;


private:
	/// <summary>
	/// Sets up the bindings for Commands to actions
	/// </summary>
	void InitialiseActions();
	
private:

	 KeyBinding* m_key_binding;

	std::map<Action, Command> m_action_binding;

	std::map<Action, bool> m_action_proxies;

	/// <summary>
	///	Globally unique player count
	/// </summary>
	//static unsigned int m_player_count; // Static member to count players
	/// <summary>
	/// The player's ID
	/// </summary>
	int m_identifier;
	/// <summary>
	/// The Players current mission status
	/// </summary>
	MissionStatus m_current_mission_status;
	/// <summary>
	///	The players gamepad
	/// </summary>
	//Gamepad m_gamepad;

	sf::TcpSocket* m_socket;
};

