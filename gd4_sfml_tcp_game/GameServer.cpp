/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "GameServer.hpp"
#include <SFML/Network/Packet.hpp>
#include "NetworkProtocol.hpp"
#include <SFML/System/Sleep.hpp>
#include "Utility.hpp"
#include "PickupType.hpp"
#include "AircraftType.hpp"
#include <iostream>
#include "World.hpp"
#include <fstream>
#include <ctime>  // For timestamps
#include <iomanip> // For formatting timestamps

GameServer::GameServer(sf::Vector2f battlefield_size, sf::RenderTarget& output_target)
    : m_thread(&GameServer::ExecutionThread, this)
    , m_listening_state(false)
    , m_client_timeout(sf::seconds(5.f))
    , m_max_connected_players(15)
    , m_connected_players(0)
    , m_world_height(1100.f)
    , m_battlefield_rect(0.f, m_world_height - battlefield_size.y, battlefield_size.x, battlefield_size.y)
    , m_battlefield_scrollspeed(0.f)
    , m_aircraft_count(0)
    , m_peers(1)
    , m_aircraft_identifier_counter(1)
    , m_waiting_thread_end(false)
    , m_last_spawn_time(sf::Time::Zero)
    , m_time_for_next_spawn(sf::seconds(0.2f))
	, m_target(output_target)
	, m_camera(output_target.getDefaultView())
    , m_player_aircrafts()
{
    m_listener_socket.setBlocking(false);
    m_peers[0].reset(new RemotePeer());
    m_thread.launch();
}

GameServer::~GameServer()
{
    m_waiting_thread_end = true;
    m_thread.wait();
}

void GameServer::NotifyPlayerSpawn(sf::Int32 aircraft_identifier, std::string clientName)
{
    sf::Packet packet;
    //First thing in every packets is what type of packet it is
    packet << static_cast<sf::Int32>(Server::PacketType::kPlayerConnect);
    packet << aircraft_identifier << m_aircraft_info[aircraft_identifier].m_position.x << m_aircraft_info[aircraft_identifier].m_position.y << clientName;
    SendToAll(packet);
}

void GameServer::NotifyPlayerRealtimeChange(sf::Int32 aircraft_identifier, sf::Int32 action, bool action_enabled)
{
    sf::Packet packet;
    //First thing in every packets is what type of packet it is
    packet << static_cast<sf::Int32>(Server::PacketType::kPlayerRealtimeChange);
    packet << aircraft_identifier;
    packet << action;
    packet << action_enabled;
    SendToAll(packet);
}

void GameServer::NotifyPlayerEvent(sf::Int32 aircraft_identifier, sf::Int32 action)
{
    sf::Packet packet;
    //First thing in every packets is what type of packet it is
    packet << static_cast<sf::Int32>(Server::PacketType::kPlayerEvent);
    packet << aircraft_identifier;
    packet << action;
    SendToAll(packet);
}

void GameServer::SetListening(bool enable)
{
    //Check is the server is already listening
    if (enable)
    {
        if (!m_listening_state)
        {
            m_listening_state = (m_listener_socket.listen(SERVER_PORT) == sf::TcpListener::Done);
        }
    }
    else
    {
        m_listener_socket.close();
        m_listening_state = false;
    }
}

void GameServer::ExecutionThread()
{
    //Initialisation
    SetListening(true);

    sf::Time frame_rate = sf::seconds(1.f / 60.f);
    sf::Time frame_time = sf::Time::Zero;
    sf::Time tick_rate = sf::seconds(1.f / 20.f);
    sf::Time tick_time = sf::Time::Zero;
    sf::Clock frame_clock, tick_clock;

    while (!m_waiting_thread_end)
    {
        //This is the game loop
        HandleIncomingConnections();
        HandleIncomingPackets();

        frame_time += frame_clock.getElapsedTime();
        frame_clock.restart();

        tick_time += tick_clock.getElapsedTime();
        tick_clock.restart();

        //Fixed time step
        while (frame_time >= frame_rate)
        {
            frame_time -= frame_rate;
        }

        //Fixed time step
        while (tick_time >= tick_rate)
        {
            Tick();
            tick_time -= tick_rate;
        }
        //sleep
        sf::sleep(sf::milliseconds(50));
    }
}

sf::FloatRect GameServer::GetViewBounds() const
{
    return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
}

void GameServer::Tick()
{
    UpdateClientState();
    //Check if the game is over = all planes postion.y < offset

    bool all_aircraft_done = false;
    for (const auto& current : m_aircraft_info)
    {
        if (Now() >= sf::seconds(900.f))
        {
            all_aircraft_done = true;
            break;
        }
    }

    if (all_aircraft_done)
    {
        sf::Packet mission_success_packet;
        mission_success_packet << static_cast<sf::Int32>(Server::PacketType::kMissionSuccess);
        SendToAll(mission_success_packet);
    }

    //Remove aircraft that have been destroyed
    for (auto itr = m_aircraft_info.begin(); itr != m_aircraft_info.end();)
    {
        if (itr->second.m_hitpoints <= 0)
        {
            m_aircraft_info.erase(itr++);
        }
        else
        {
            ++itr;
        }
    } 

	//The if statement below has been refined by ChatGPT - it was originally written by Dylan, but enhanced by AI
    //Check if it is time to spawn enemies
    if (Now() >= m_time_for_next_spawn + m_last_spawn_time)
    {
        std::size_t enemy_count = 1;

        sf::FloatRect viewBounds = GetViewBounds();
        std::cout << "View Bounds: Left = " << viewBounds.left << ", Top = " << viewBounds.top
            << ", Width = " << viewBounds.width << ", Height = " << viewBounds.height << std::endl;
        float margin = 2.f;

        // Expand the bounds to include the margin
        viewBounds.left -= margin;
        viewBounds.top -= margin;
        viewBounds.width += 2 * margin;
        viewBounds.height += 2 * margin;

        // Determine a random side
        int side = Utility::RandomInt(4);
        sf::Vector2f spawnPos;
        switch (side)
        {
        case 0: // Top
            spawnPos.x = viewBounds.left + Utility::RandomInt(static_cast<int>(viewBounds.width));
            spawnPos.y = viewBounds.top - margin;
            break;
        case 1: // Bottom
            spawnPos.x = viewBounds.left + Utility::RandomInt(static_cast<int>(viewBounds.width));
            spawnPos.y = viewBounds.top + viewBounds.height + margin;
            break;
        case 2: // Left
            spawnPos.x = viewBounds.left - margin;
            spawnPos.y = viewBounds.top + Utility::RandomInt(static_cast<int>(viewBounds.height));
            break;
        case 3: // Right
            spawnPos.x = viewBounds.left + viewBounds.width + margin;
            spawnPos.y = viewBounds.top + Utility::RandomInt(static_cast<int>(viewBounds.height));
            break;
        default:
            spawnPos = sf::Vector2f(viewBounds.left + viewBounds.width / 2.f, viewBounds.top - margin);
            break;
        }

        std::cout << "Spawn Position: (" << spawnPos.x << ", " << spawnPos.y << "), Side: " << side << "\n";

        //Send the spawn packets to the clients
        for (std::size_t i = 0; i < enemy_count; ++i)
        {
            sf::Packet packet;
            packet << static_cast<sf::Int32>(Server::PacketType::kSpawnEnemy);
            packet << static_cast<sf::Int32>(1 + Utility::RandomInt(static_cast<int>(AircraftType::kAircraftCount) - 1));

            packet << spawnPos.x;
            packet << spawnPos.y;

            std::cout << "Sending enemy spawn packet: Type=" << 1 + Utility::RandomInt(static_cast<int>(AircraftType::kAircraftCount) - 1)
                << ", Position=(" << spawnPos.x << ", " << spawnPos.y << ")\n";

            SendToAll(packet);
        }

        m_last_spawn_time = Now();
        m_time_for_next_spawn = sf::seconds(0.25f + static_cast<float>(Utility::RandomInt(1250)) / 1000.f);
    }
}

sf::Time GameServer::Now() const
{
    return m_clock.getElapsedTime();
}

void GameServer::HandleIncomingPackets()
{
    bool detected_timeout = false;

    for (PeerPtr& peer : m_peers)
    {
        if (peer->m_ready)
        {
            sf::Packet packet;
            while (peer->m_socket.receive(packet) == sf::Socket::Done)
            {
                //Interpret the packet and react to it
                HandleIncomingPackets(packet, *peer, detected_timeout);

                peer->m_last_packet_time = Now();
                packet.clear();
            }

            if (Now() > peer->m_last_packet_time + m_client_timeout)
            {
                peer->m_timed_out = true;
                detected_timeout = true;
            }

        }
    }

    if (detected_timeout)
    {
        HandleDisconnections();
    }
}



void GameServer::HandleIncomingPackets(sf::Packet& packet, RemotePeer& receiving_peer, bool& detected_timeout)
{
    sf::Int32 packet_type;
    packet >> packet_type;

    switch (static_cast<Client::PacketType> (packet_type))
    {

	case Client::PacketType::kRotationUpdate:
    {
        sf::Int32 aircraft_identifier;
		float angle;
		packet >> aircraft_identifier >> angle;

        sf::Packet update;
		update << static_cast<sf::Int32>(Server::PacketType::kRotationUpdate);
		update << aircraft_identifier;
		update << angle;
		SendToAll(update);
    }
    break;
    case Client::PacketType::kQuit:
    {
        receiving_peer.m_timed_out = true;
        detected_timeout = true;
    }
    break;

    case Client::PacketType::kPlayerEvent:
    {
        sf::Int32 aircraft_identifier;
        sf::Int32 action;
        
    }
    break;

    case Client::PacketType::kPlayerRealtimeChange:
    {
        sf::Int32 aircraft_identifier;
        sf::Int32 action;
        bool action_enabled;
        packet >> aircraft_identifier >> action >> action_enabled;
        NotifyPlayerRealtimeChange(aircraft_identifier, action, action_enabled);
    }
    break;

    case Client::PacketType::kRequestCoopPartner:
    {
        receiving_peer.m_aircraft_identifiers.emplace_back(m_aircraft_identifier_counter);
        m_aircraft_info[m_aircraft_identifier_counter].m_position = sf::Vector2f(m_battlefield_rect.width / 2, m_battlefield_rect.top + m_battlefield_rect.height / 2);
        m_aircraft_info[m_aircraft_identifier_counter].m_hitpoints = 100;

        sf::Packet request_packet;
        request_packet << static_cast<sf::Int32>(Server::PacketType::kAcceptCoopPartner);
        request_packet << m_aircraft_identifier_counter;
        request_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.x;
        request_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.y;

        receiving_peer.m_socket.send(request_packet);
        m_aircraft_count++;

        // Tell everyone else about the new plane
        sf::Packet notify_packet;
        notify_packet << static_cast<sf::Int32>(Server::PacketType::kPlayerConnect);
        notify_packet << m_aircraft_identifier_counter;
        notify_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.x;
        notify_packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.y;

        for (PeerPtr& peer : m_peers)
        {
            if (peer.get() != &receiving_peer && peer->m_ready)
            {

                peer->m_socket.send(notify_packet);
            }
        }

        m_aircraft_identifier_counter++;
    }
    break;

    case Client::PacketType::kStateUpdate:
    {
        sf::Int32 num_aircraft;
        packet >> num_aircraft;

        for (sf::Int32 i = 0; i < num_aircraft; ++i)
        {
            sf::Int32 aircraft_identifier;
            sf::Int32 aircraft_hitpoints;
            sf::Int32 missile_ammo;
            sf::Vector2f aircraft_position;
			float aircraft_rotation;
            packet >> aircraft_identifier >> aircraft_position.x >> aircraft_position.y >> aircraft_hitpoints >> aircraft_rotation;
            m_aircraft_info[aircraft_identifier].m_position = aircraft_position;
			m_aircraft_info[aircraft_identifier].m_rotation = aircraft_rotation;
            m_aircraft_info[aircraft_identifier].m_hitpoints = aircraft_hitpoints;

        }
    }
    break;

    case Client::PacketType::kGameEvent:
    {
        sf::Int32 action;
        float x;
        float y;

        packet >> action;
        packet >> x;
        packet >> y;

		int random_number = Utility::RandomInt(100);
        std::cout << "RANDOM: " << Utility::RandomInt(100) << std::endl;
        //Enemy explodes, with a certain probability, drop a pickup
        //To avoid multiple messages only listen to the first peer (host)
        if (action == GameActions::kEnemyExplode && random_number <= 10 && &receiving_peer == m_peers[0].get())
        {
            sf::Packet packet;
            packet << static_cast<sf::Int32>(Server::PacketType::kSpawnPickup);
            packet << static_cast<sf::Int32>(Utility::RandomInt(static_cast<int>(PickupType::kPickupCount)));
            packet << x;
            packet << y;

            SendToAll(packet);
        }
    }
    }
}

void GameServer::HandleIncomingConnections()
{
    if (!m_listening_state)
    {
        return;
    }

    if (m_listener_socket.accept(m_peers[m_connected_players]->m_socket) == sf::TcpListener::Done)
    {
        // Try to receive the client's name packet right away.
        sf::Packet namePacket;
        // Wait briefly (or try multiple times) for the client to send its name.
        // (A more robust solution might involve a timeout or a separate handshake.)
        if (m_peers[m_connected_players]->m_socket.receive(namePacket) == sf::Socket::Done)
        {
            sf::Int32 packetType;
            namePacket >> packetType;
            if (packetType == static_cast<sf::Int32>(Client::PacketType::kClientName))
            {
                std::string clientName;
                namePacket >> clientName;
                m_peers[m_connected_players]->m_name = clientName;
                std::cout << "Received client name: " << clientName << std::endl;
            }
        }
        else
        {
            // Fallback if no name received, or wait a bit longer as needed.
            m_peers[m_connected_players]->m_name = "DefaultPlayer";
        }


        //Order the new client to spawn its player 1
        m_aircraft_info[m_aircraft_identifier_counter].m_position = sf::Vector2f(m_battlefield_rect.width / 2, m_battlefield_rect.top + m_battlefield_rect.height / 2);
        m_aircraft_info[m_aircraft_identifier_counter].m_hitpoints = 100;

        sf::Packet packet;
        packet << static_cast<sf::Int32>(Server::PacketType::kSpawnSelf);
        packet << m_aircraft_identifier_counter;
        packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.x;
        packet << m_aircraft_info[m_aircraft_identifier_counter].m_position.y;
		packet << m_peers[m_connected_players]->m_name;

        m_peers[m_connected_players]->m_aircraft_identifiers.emplace_back(m_aircraft_identifier_counter);

        BroadcastMessage("Player:" + m_peers[m_connected_players]->m_name + " has joined");
        InformWorldState(m_peers[m_connected_players]->m_socket);
        NotifyPlayerSpawn(m_aircraft_identifier_counter++, m_peers[m_connected_players]->m_name);

        m_peers[m_connected_players]->m_socket.send(packet);
        m_peers[m_connected_players]->m_ready = true;
        m_peers[m_connected_players]->m_last_packet_time = Now();

        m_aircraft_count++;
        m_connected_players++;

        if (m_connected_players >= m_max_connected_players)
        {
            SetListening(false);
        }
        else
        {
            m_peers.emplace_back(PeerPtr(new RemotePeer()));
        }
    }
}

//The entire survived time/high score logic has been generated by ChatGPT
std::vector<PlayerScore> GameServer::ReadHighScores(const std::string& filename)
{
    std::vector<PlayerScore> scores;
    std::ifstream file(filename);

    if (!file)
    {
        std::cerr << "Warning: Could not open " << filename << ". Creating a new file." << std::endl;
        return scores;
    }

    sf::Int32 id;
    float time;
    while (file >> id >> time)
    {
        scores.push_back({ id, time });
    }

    return scores;
}

void GameServer::WriteHighScores(const std::vector<PlayerScore>& scores, const std::string& filename)
{
    std::ofstream file(filename, std::ios::trunc); // Overwrite file

    if (!file)
    {
        std::cerr << "Error: Could not open file for writing!" << std::endl;
        return;
    }

    for (const auto& score : scores)
    {
        file << score.identifier << " " << score.time_survived << "\n";
    }
}

void GameServer::UpdateHighScores(sf::Int32 identifier, float time_survived, const std::string& filename)
{
    std::vector<PlayerScore> scores = ReadHighScores(filename);

    // Add new score
    scores.push_back({ identifier, time_survived });

    // Sort by highest time survived
    std::sort(scores.begin(), scores.end());

    // Keep only top 10
    if (scores.size() > 10)
    {
        scores.resize(10);
    }

    // Write updated scores back to file
    WriteHighScores(scores, filename);
}

void GameServer::HandleDisconnections()
{
    for (auto itr = m_peers.begin(); itr != m_peers.end();)
    {
        if ((*itr)->m_timed_out)
        {
            //Inform everyone of a disconnection, erase
            for (sf::Int32 identifier : (*itr)->m_aircraft_identifiers)
            {
                SendToAll((sf::Packet() << static_cast<sf::Int32>(Server::PacketType::kPlayerDisconnect) << identifier));
                m_aircraft_info.erase(identifier);
				float time_survived = Now().asSeconds();
                std::cout << "Player " << identifier << " has survived " << time_survived << " seconds" << std::endl;
                UpdateHighScores(identifier, time_survived, "survival_times.txt");
            }

            m_connected_players--;
            m_aircraft_count -= (*itr)->m_aircraft_identifiers.size();

            itr = m_peers.erase(itr);

            //If the number of peers has dropped below max_connections
            if (m_connected_players < m_max_connected_players)
            {
                m_peers.emplace_back(PeerPtr(new RemotePeer()));
                SetListening(true);
            }

            BroadcastMessage("A player has disconnected");
        }
        else
        {
            ++itr;
        }
    }

}

void GameServer::InformWorldState(sf::TcpSocket& socket)
{
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::PacketType::kInitialState);
    packet << m_world_height << m_battlefield_rect.top + m_battlefield_rect.height;
    packet << static_cast<sf::Int32>(m_aircraft_count);

    for (std::size_t i = 0; i < m_connected_players; ++i)
    {
        if (m_peers[i]->m_ready)
        {
            for (sf::Int32 identifier : m_peers[i]->m_aircraft_identifiers)
            {
				packet << identifier << m_aircraft_info[identifier].m_position.x << m_aircraft_info[identifier].m_position.y << m_aircraft_info[identifier].m_hitpoints << m_aircraft_info[identifier].m_rotation << m_peers[i]->m_name;
            }
        }
    }

    socket.send(packet);
}

void GameServer::BroadcastMessage(const std::string& message)
{
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::PacketType::kBroadcastMessage);
    packet << message;
    for (std::size_t i = 0; i < m_connected_players; ++i)
    {
        if (m_peers[i]->m_ready)
        {
            m_peers[i]->m_socket.send(packet);
        }
    }
}

void GameServer::SendToAll(sf::Packet& packet)
{
    for (std::size_t i = 0; i < m_connected_players; ++i)
    {
        if (m_peers[i]->m_ready)
        {
            m_peers[i]->m_socket.send(packet);
        }
    }
}

void GameServer::UpdateClientState()
{
    sf::Packet update_client_state_packet;
    update_client_state_packet << static_cast<sf::Int32>(Server::PacketType::kUpdateClientState);
    update_client_state_packet << static_cast<float>(m_battlefield_rect.top + m_battlefield_rect.height);

    // Compute active aircraft count instead of total aircraft count
    sf::Int32 activeAircraftCount = 0;
    for (const auto& pair : m_aircraft_info)
    {
        if (pair.second.m_hitpoints > 0)
        {
            activeAircraftCount++;
        }
    }
    update_client_state_packet << activeAircraftCount;

    // Only send updates for aircraft that are still active
    for (const auto& aircraft : m_aircraft_info)
    {
        if (aircraft.second.m_hitpoints > 0)
        {
            update_client_state_packet << aircraft.first
                << aircraft.second.m_position.x
                << aircraft.second.m_position.y
                << aircraft.second.m_hitpoints
                << aircraft.second.m_rotation;
        }
    }
    SendToAll(update_client_state_packet);
}



//It is essential to set the sockets to non-blocking - m_socket.setBlocking(false)
//otherwise the server will hang waiting to read input from a connection

GameServer::RemotePeer::RemotePeer() : m_ready(false), m_timed_out(false)
{
    m_socket.setBlocking(false);
}
