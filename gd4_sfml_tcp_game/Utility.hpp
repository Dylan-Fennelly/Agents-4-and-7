#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include "Animation.hpp"
#include "Action.hpp"

class Utility
{
	public:
		static sf::Vector2f UnitVector(const sf::Vector2f& source);
		static void CentreOrigin(sf::Sprite& sprite);
		static void CentreOrigin(sf::Text& text);
		static void CentreOrigin(Animation& animation);
		static std::string toString(sf::Keyboard::Key key);
		static double ToRadians(int degrees);
		static double ToDegrees(double angle);
		static int RandomInt(int exclusive_max);
		static int Length(sf::Vector2f vector);
		static bool IsRealTimeAction(Action action);
		
};

