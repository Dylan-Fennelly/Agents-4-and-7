#include "Player.hpp"
#include "ReceiverCategories.hpp"
#include "Aircraft.hpp"
#include <iostream>

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

Player::Player(unsigned int player_id, unsigned int joystick_id)
    : m_player_id(player_id) // Assign player ID
    , m_current_mission_status(MissionStatus::kMissionRunning)
	, m_gamepad(joystick_id) // Create gamepad object
{
    //Set initial action bindings
    InitialiseActions();
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

void Player::InitialiseActions()
{
    const float kPlayerSpeed = 200.f;

    // Move Left
    Command moveLeft;
    moveLeft.action = DerivedAction<Aircraft>(AircraftMover(-kPlayerSpeed, 0.f));
    moveLeft.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    m_gamepad.AssignCommand(Action::kMoveLeft, moveLeft);

    // Move Right
    Command moveRight;
    moveRight.action = DerivedAction<Aircraft>(AircraftMover(kPlayerSpeed, 0.f));
    moveRight.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    m_gamepad.AssignCommand(Action::kMoveRight, moveRight);

    // Move Up
    Command moveUp;
    moveUp.action = DerivedAction<Aircraft>(AircraftMover(0.f, -kPlayerSpeed));
    moveUp.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    m_gamepad.AssignCommand(Action::kMoveUp, moveUp);

    // Move Down
    Command moveDown;
    moveDown.action = DerivedAction<Aircraft>(AircraftMover(0.f, kPlayerSpeed));
    moveDown.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    m_gamepad.AssignCommand(Action::kMoveDown, moveDown);

    // Bullet Fire
    Command fireBullet;
    fireBullet.action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time) { a.Fire(); });
    fireBullet.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    m_gamepad.AssignCommand(Action::kBulletFire, fireBullet);
    m_gamepad.AssignAction(Action::kBulletFire, ButtonFunction::kConfirm);

    // Missile Fire
    Command fireMissile;
    fireMissile.action = DerivedAction<Aircraft>([](Aircraft& a, sf::Time) { a.LaunchMissile(); });
    fireMissile.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    m_gamepad.AssignCommand(Action::kMissileFire, fireMissile);
    m_gamepad.AssignAction(Action::kMissileFire, ButtonFunction::kCancel);

	//Assigning pause to prevent an issue where pause will take a random button if there is no match
	m_gamepad.AssignAction(Action::kPause, ButtonFunction::kPause);
	m_gamepad.AssignCommand(Action::kPause, Command());
}