/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "Player.hpp"
#include "ReceiverCategories.hpp"
#include "Aircraft.hpp"
#include <iostream>
#include "Constants.hpp"

unsigned int Player::m_player_count = 0;

struct AircraftMover
{
    AircraftMover(float vx, float vy) :velocity(vx, vy)
    {}
    void operator()(Aircraft& aircraft, sf::Time) const
    {
        aircraft.Accelerate(velocity);
    }

    sf::Vector2f velocity;
};
//This struct is used to rotate the aircraft
struct AircraftRotator
{
    AircraftRotator(float angle) : rotation_angle(angle) {}

    void operator()(Aircraft& aircraft, sf::Time) const
    {
        aircraft.setRotation(rotation_angle);
    }

    float rotation_angle;
};


Player::Player()
    : m_player_id(++m_player_count) // Assign player ID
    , m_current_mission_status(MissionStatus::kMissionRunning)
	, m_gamepad(0,0)
{ //We now longer initalise actions on construction as we dont have a joystick id yet
}

void Player::HandleEvent(const sf::Event& event, CommandQueue& command_queue)
{
    m_gamepad.HandleEvent(event, command_queue);
}

void Player::HandleRealTimeInput(CommandQueue& command_queue)
{
    m_gamepad.Update(command_queue);
}



void Player::SetMissionStatus(MissionStatus status)
{
    m_current_mission_status = status;
}

MissionStatus Player::GetMissionStatus() const
{
    return m_current_mission_status;
}

Gamepad& Player::GetGamepad()
{
	return m_gamepad;
}

void Player::SetGamepad(Gamepad gamepad)
{
	m_gamepad = gamepad;
    InitialiseActions();
}

unsigned int Player::GetPlayerID() const
{
    return m_player_id;
}

unsigned int Player::GetPlayerCount() const
{
	return m_player_count;
}

void Player::InitialiseActions()
{
    const float kPlayerSpeed = PLAYER_SPEED;

    // Move Left
    Command moveLeft;
    moveLeft.action = DerivedAction<Aircraft>(AircraftMover(-kPlayerSpeed, 0.f));
    moveLeft.category = static_cast<unsigned int>((m_player_id == 1) ? ReceiverCategories::kPlayerAircraft : ReceiverCategories::kAlliedAircraft);

    m_gamepad.AssignCommand(Action::kMoveLeft, moveLeft);

    // Move Right
    Command moveRight;
    moveRight.action = DerivedAction<Aircraft>(AircraftMover(kPlayerSpeed, 0.f));
    moveRight.category = static_cast<unsigned int>((m_player_id == 1) ? ReceiverCategories::kPlayerAircraft : ReceiverCategories::kAlliedAircraft);
    m_gamepad.AssignCommand(Action::kMoveRight, moveRight);

    // Move Up
    Command moveUp;
    moveUp.action = DerivedAction<Aircraft>(AircraftMover(0.f, -kPlayerSpeed));
    moveUp.category = static_cast<unsigned int>((m_player_id == 1) ? ReceiverCategories::kPlayerAircraft : ReceiverCategories::kAlliedAircraft);
    m_gamepad.AssignCommand(Action::kMoveUp, moveUp);

    // Move Down
    Command moveDown;
    moveDown.action = DerivedAction<Aircraft>(AircraftMover(0.f, kPlayerSpeed));
    moveDown.category = static_cast<unsigned int>((m_player_id == 1) ? ReceiverCategories::kPlayerAircraft : ReceiverCategories::kAlliedAircraft);
    m_gamepad.AssignCommand(Action::kMoveDown, moveDown);

    // Bullet Fire
    Command fireBullet;
    fireBullet.action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time) { a.Fire(); });
    fireBullet.category = static_cast<unsigned int>((m_player_id == 1) ? ReceiverCategories::kPlayerAircraft : ReceiverCategories::kAlliedAircraft);
    m_gamepad.AssignCommand(Action::kBulletFire, fireBullet);
    m_gamepad.AssignAction(Action::kBulletFire, ButtonFunction::kConfirm);

    // Missile Fire
    /*Command fireMissile;
    fireMissile.action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time) { a.LaunchMissile(); });
    fireMissile.category = static_cast<unsigned int>((m_player_id == 1) ? ReceiverCategories::kPlayerAircraft : ReceiverCategories::kAlliedAircraft);*/
	//Maybe someday the agents will get missile launchers, but not today.
    m_gamepad.AssignCommand(Action::kMissileFire, Command());
    m_gamepad.AssignAction(Action::kMissileFire, ButtonFunction::kCancel);

	//Assigning pause to prevent an issue where pause will take a random button if there is no match
	m_gamepad.AssignAction(Action::kPause, ButtonFunction::kPause);
	m_gamepad.AssignCommand(Action::kPause, Command());
}