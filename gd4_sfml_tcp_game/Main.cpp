#include <SFML/Graphics.hpp>
#include "Game.hpp"
#include "ResourceIdentifiers.hpp"
#include <iostream>
#include "Application.hpp"


 int DetectJoystick()
{
	for (unsigned int i = 0; i < sf::Joystick::Count; ++i)
	{
		if (sf::Joystick::isConnected(i))
		{
			std::cout << "Joystick " << i << " is connected!" << std::endl;
			sf::Joystick::Identification id = sf::Joystick::getIdentification(i);
			std::cout << " - Name: " << id.name.toAnsiString() << std::endl;
			std::cout << " - Vendor ID: " << id.vendorId << std::endl;
			std::cout << " - Product ID: " << id.productId << std::endl;
			std::cout << " - Button Count: " << sf::Joystick::getButtonCount(i) << std::endl;
			return i;
		}
	}
	return -1;
}
int main()
{
	int joystick_id = DetectJoystick();
	//TextureHolder game_textures;
	try
	{
		Application app(joystick_id);
		app.Run();
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}
}
//}
//#include <SFML/Window.hpp>
//#include <SFML/Graphics.hpp>
//#include <iostream>
//#include <map>
//#include <cmath> // For std::abs
//
//// Define a dead zone for analog sticks
//const float DEAD_ZONE = 15.0f;
//
//// Track previous axis positions to detect significant changes
//std::map<std::pair<unsigned int, sf::Joystick::Axis>, float> previousAxisPositions;
//
//bool hasSignificantChange(unsigned int joystickId, sf::Joystick::Axis axis, float newPosition) {
//    auto key = std::make_pair(joystickId, axis);
//
//    // Check if we have a previous position for this axis
//    if (previousAxisPositions.find(key) != previousAxisPositions.end()) {
//        float previousPosition = previousAxisPositions[key];
//
//        // Only log if the change exceeds a threshold
//        if (std::abs(newPosition - previousPosition) > 5.0f) { // Significant change threshold
//            previousAxisPositions[key] = newPosition; // Update the stored position
//            return true;
//        }
//        return false; // No significant change
//    }
//
//    // If it's the first time checking, store the position and log it
//    previousAxisPositions[key] = newPosition;
//    return true;
//}
//
//int main() {
//    sf::RenderWindow window(sf::VideoMode(800, 600), "Joystick Input Test");
//
//    // Check which joysticks are connected at the start
//    for (unsigned int i = 0; i < sf::Joystick::Count; ++i) {
//        if (sf::Joystick::isConnected(i)) {
//            std::cout << "Joystick " << i << " is connected!" << std::endl;
//            sf::Joystick::Identification id = sf::Joystick::getIdentification(i);
//            std::cout << " - Name: " << id.name.toAnsiString() << std::endl;
//            std::cout << " - Vendor ID: " << id.vendorId << std::endl;
//            std::cout << " - Product ID: " << id.productId << std::endl;
//            std::cout << " - Button Count: " << sf::Joystick::getButtonCount(i) << std::endl;
//        }
//    }
//
//    while (window.isOpen()) {
//        sf::Event event;
//        while (window.pollEvent(event)) {
//            if (event.type == sf::Event::Closed)
//                window.close();
//        }
//
//        // Continuously check all joysticks
//        for (unsigned int i = 0; i < sf::Joystick::Count; ++i) {
//            if (sf::Joystick::isConnected(i)) {
//                // Check button states
//                for (unsigned int button = 0; button < sf::Joystick::getButtonCount(i); ++button) {
//                    if (sf::Joystick::isButtonPressed(i, 6))
//                    {
//                        continue;
//                    }
//                    if (sf::Joystick::isButtonPressed(i, button)) {
//                        std::cout << "Joystick " << i << " - Button " << button << " is pressed!" << std::endl;
//                    }
//                }
//
//                // Check axis movement with dead zone and significant change filtering
//                for (int axis = sf::Joystick::X; axis <= sf::Joystick::PovY; ++axis) {
//                    if (sf::Joystick::hasAxis(i, static_cast<sf::Joystick::Axis>(axis))) {
//                        float position = sf::Joystick::getAxisPosition(i, static_cast<sf::Joystick::Axis>(axis));
//
//                        // Apply dead zone and check for significant changes
//                        if (std::abs(position) > DEAD_ZONE && hasSignificantChange(i, static_cast<sf::Joystick::Axis>(axis), position)) {
//                            std::cout << "Joystick " << i << " - Axis "
//                                << axis << " moved to " << position << std::endl;
//                        }
//                    }
//                }
//            }
//        }
//
//        window.clear();
//        window.display();
//    }
//
//    return 0;
//}
