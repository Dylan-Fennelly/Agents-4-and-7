#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Aircraft.hpp"
#include "NetworkProtocol.hpp"
#include <SFML/Network/Packet.hpp>

#include <map>
#include <string>
#include <algorithm>
#include <iostream>

struct AircraftMover
{
    AircraftMover(float vx, float vy, int identifier)
        : velocity(vx, vy)
        , aircraft_id(identifier)
    {
    }
    void operator()(Aircraft& aircraft, sf::Time) const
    {
        if (aircraft.GetIdentifier() == aircraft_id)
        {
            aircraft.Accelerate(velocity * aircraft.GetMaxSpeed());
        }
    }
    sf::Vector2f velocity;
    int aircraft_id;
};
struct AircraftRotator
{
    AircraftRotator(sf::Vector2i target_position, int identifier, sf::TcpSocket* socket = nullptr, sf::RenderWindow* window = nullptr,sf::View camera = sf::View())
        : target(target_position)
        , aircraft_id(identifier)
        , m_socket(socket)
        , window(window)
		, camera(camera)
    {
    }
	AircraftRotator(int identifier, float angle, bool anglePrecomputed)
		: target(sf::Vector2i(0, 0))
		, aircraft_id(identifier)
		, m_socket(nullptr)
		, window(nullptr)
		, camera(sf::View())
		, angle(angle)
		, anglePrecomputed(anglePrecomputed)
    {}

    void operator()(Aircraft& aircraft, sf::Time) const
    {
        if (anglePrecomputed)
        {
            if (aircraft.GetIdentifier() == aircraft_id)
            {
                aircraft.SetRotation(angle);
                return;
            }
            
        }
        if (aircraft.GetIdentifier() == aircraft_id)
        {
           // std::cout << "Rotating Aircraft" << std::endl;
            //std::cout << "Target Position: " << target.x << ", " << target.y << std::endl;
            sf::Vector2f mouse_world = window->mapPixelToCoords(target,camera);
			//std::cout << "Mouse World: " << mouse_world.x << ", " << mouse_world.y << std::endl;

            // Convert mouse position from window coordinates to world coordinates
            sf::Vector2f aircraft_position = aircraft.getPosition();
            // these are world coordinates we need to convert them to window coordinates
           // sf::Vector2i aircraft_position_ScreenPos = window->mapCoordsToPixel(aircraft_position);
         //   std::cout << "aircraft_position: " << aircraft_position.x << ", " << aircraft_position.y << std::endl;
            sf::Vector2f direction = sf::Vector2f(mouse_world) - sf::Vector2f(aircraft_position);
          //  std::cout << "Direction: " << direction.x << ", " << direction.y << std::endl;


                // Calculate angle in radians and convert it to degrees
                float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f;
                angle += 90.f;
                aircraft.SetRotation(angle);
				std::cout << "Angle: " << angle << std::endl;
                // Send rotation update to server
                //if (m_socket)
                //{
                //    sf::Packet packet;
                //    packet << static_cast<sf::Int32>(Client::PacketType::kRotationUpdate);
                //    packet << aircraft_id;
                //    packet << angle;
                //    m_socket->send(packet);
                //}
            
        }
    }

    sf::Vector2i target;
    sf::RenderWindow* window;
    int aircraft_id;
    sf::TcpSocket* m_socket;
	sf::View camera;
    float angle;
	bool anglePrecomputed = false;
};



struct AircraftFireTrigger
{
    AircraftFireTrigger(int identifier)
        : aircraft_id(identifier)
    {
    }

    void operator() (Aircraft& aircraft, sf::Time) const
    {
        if (aircraft.GetIdentifier() == aircraft_id)
            aircraft.Fire();
    }

    int aircraft_id;
};


Player::Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding) 
    : m_key_binding(binding)
    , m_current_mission_status(MissionStatus::kMissionRunning)
    , m_identifier(identifier)
    , m_socket(socket)
{
    //Set initial action bindings
    InitializeActions();

    //Assign all categories to a player's aircraft
    for (auto& pair : m_action_binding)
    {
        pair.second.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    }

}


void Player::HandleEvent(const sf::Event& event, CommandQueue& commands, sf::RenderWindow& window, sf::View camera)
{

    if (event.type == sf::Event::MouseMoved &&m_key_binding != nullptr)
    {
        // Get the current mouse position (window coordinates
        sf::Vector2i mouse_position = sf::Mouse::getPosition(window);

        Command command;
        command.category = static_cast<int>(ReceiverCategories::kPlayerAircraft);

        // Pass the view to the AircraftRotator
        command.action = DerivedAction<Aircraft>(AircraftRotator(mouse_position, m_identifier, m_socket, &window, camera));
        commands.Push(command);
    }

    if (event.type == sf::Event::KeyPressed)
    {
        Action action;
        if (m_key_binding && m_key_binding->CheckAction(event.key.code, action) && !IsRealtimeAction(action))
        {
            // Network connected -> send event over network
            if (m_socket)
            {
                sf::Packet packet;
                packet << static_cast<sf::Int32>(Client::PacketType::kPlayerEvent);
                packet << m_identifier;
                packet << static_cast<sf::Int32>(action);
                m_socket->send(packet);
            }

            // Network disconnected -> local event
            else
            {
                commands.Push(m_action_binding[action]);
            }
        }
    }

    // Realtime change (network connected)
    if ((event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) && m_socket)
    {
        Action action;
        if (m_key_binding && m_key_binding->CheckAction(event.key.code, action) && IsRealtimeAction(action))
        {
            // Send realtime change over network
            sf::Packet packet;
            packet << static_cast<sf::Int32>(Client::PacketType::kPlayerRealtimeChange);
            packet << m_identifier;
            packet << static_cast<sf::Int32>(action);
            packet << (event.type == sf::Event::KeyPressed);
            m_socket->send(packet);
        }
    }
}

bool Player::IsLocal() const
{
    // No key binding means this player is remote
    return m_key_binding != nullptr;

}

void Player::DisableAllRealtimeActions()
{
    for (auto& action : m_action_proxies)
    {
        sf::Packet packet;
        packet << static_cast<sf::Int32>(Client::PacketType::kPlayerRealtimeChange);
        packet << m_identifier;
        packet << static_cast<sf::Int32>(action.first);
        packet << false;
        m_socket->send(packet);
    }
}

void Player::HandleRealtimeInput(CommandQueue& commands)
{
    // Check if this is a networked game and local player or just a single player game
    if ((m_socket && IsLocal()) || !m_socket)
    {
        // Lookup all actions and push corresponding commands to queue
        std::vector<Action> activeActions = m_key_binding->GetRealtimeActions();
        for (Action action : activeActions)
        {
            commands.Push(m_action_binding[action]);
        }
    }
}

void Player::HandleRealtimeNetworkInput(CommandQueue& commands)
{
    //std::cout << "Check: " << (m_socket && !IsLocal()) << std::endl;


    if (m_socket && !IsLocal())
    {
        // Traverse all realtime input proxies. Because this is a networked game, the input isn't handled directly
        for (auto pair : m_action_proxies)
        {
            if (pair.second && IsRealtimeAction(pair.first))
            {
                commands.Push(m_action_binding[pair.first]);
            }
        }
    }
}

void Player::HandleNetworkEvent(Action action, CommandQueue& commands)
{
    commands.Push(m_action_binding[action]);
}

void Player::HandleNetworkRealtimeChange(Action action, bool actionEnabled)
{
    m_action_proxies[action] = actionEnabled;
}

void Player::SetMissionStatus(MissionStatus status)
{
    m_current_mission_status = status;
}

MissionStatus Player::GetMissionStatus() const
{
    return m_current_mission_status;
}

void Player::InitializeActions()
{
    m_action_binding[Action::kMoveLeft].action = DerivedAction<Aircraft>(AircraftMover(-1, 0.f, m_identifier));
    m_action_binding[Action::kMoveRight].action = DerivedAction<Aircraft>(AircraftMover(+1, 0.f, m_identifier));
    m_action_binding[Action::kMoveUp].action = DerivedAction<Aircraft>(AircraftMover(0.f, -1, m_identifier));
    m_action_binding[Action::kMoveDown].action = DerivedAction<Aircraft>(AircraftMover(0.f, 1, m_identifier));
    m_action_binding[Action::kBulletFire].action = DerivedAction<Aircraft>(AircraftFireTrigger(m_identifier));

}
    //m_action_binding[Action::kMissileFire].action = DerivedAction<Aircraft>(AircraftMissileTrigger(m_identifier));

