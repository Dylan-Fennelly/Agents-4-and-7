/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Config.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Thread.hpp>
#include <map>
#include <memory>
#include <string>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics.hpp>
#include "Aircraft.hpp"

struct PlayerScore
{
	sf::Int32 identifier;
	float time_survived;
	std::string timestamp; // Store timestamp as a string

	// Sorting order: Higher survival time first
	bool operator<(const PlayerScore& other) const
	{
		return time_survived > other.time_survived;
	}
};

class GameServer
{
public:
	explicit GameServer(sf::Vector2f battlefield_size, sf::RenderTarget& target);
	~GameServer();
	void NotifyPlayerSpawn(sf::Int32 aircraft_identifier,std::string clientName);
	void NotifyPlayerRealtimeChange(sf::Int32 aircraft_identifier, sf::Int32 action, bool action_enabled);
	void NotifyPlayerEvent(sf::Int32 aircraft_identifier, sf::Int32 action);
	sf::FloatRect GetViewBounds() const;

private:
	struct RemotePeer
	{
		RemotePeer();
		sf::TcpSocket m_socket;
		sf::Time m_last_packet_time;
		std::vector<sf::Int32> m_aircraft_identifiers;
		bool m_ready;
		bool m_timed_out;
		std::string m_name;
	};

	struct AircraftInfo
	{
		sf::Vector2f m_position;
		sf::Int32 m_hitpoints;
		float m_rotation;
		std::map<sf::Int32, bool> m_realtime_actions;
	};

	typedef std::unique_ptr<RemotePeer> PeerPtr;

private:
	void SetListening(bool enable);
	void ExecutionThread();
	void Tick();
	sf::Time Now() const;

	void HandleIncomingPackets();
	void HandleIncomingPackets(sf::Packet& packet, RemotePeer& receiving_peer, bool& detected_timeout);

	void HandleIncomingConnections();
	void HandleDisconnections();

	void InformWorldState(sf::TcpSocket& socket);
	void BroadcastMessage(const std::string& message);
	void SendToAll(sf::Packet& packet);
	void UpdateClientState();

	std::vector<PlayerScore> ReadHighScores(const std::string& filename);
	void WriteHighScores(const std::vector<PlayerScore>& scores, const std::string& filename);
	void UpdateHighScores(sf::Int32 identifier, float time_survived, const std::string& filename);

private:
	sf::Thread m_thread;
	sf::Clock m_clock;
	sf::TcpListener m_listener_socket;
	bool m_listening_state;
	sf::Time m_client_timeout;

	std::size_t m_max_connected_players;
	std::size_t m_connected_players;

	float m_world_height;
	sf::FloatRect m_battlefield_rect;
	float m_battlefield_scrollspeed;

	std::size_t m_aircraft_count;
	std::map<sf::Int32, AircraftInfo> m_aircraft_info;

	std::vector<PeerPtr> m_peers;
	sf::Int32 m_aircraft_identifier_counter;
	bool m_waiting_thread_end;

	sf::Time m_last_spawn_time;
	sf::Time m_time_for_next_spawn;

	sf::RenderTarget& m_target;
	sf::View m_camera;
	std::vector<Aircraft*> m_player_aircrafts;
	std::vector<Aircraft*> m_enemy_aircrafts;
};

