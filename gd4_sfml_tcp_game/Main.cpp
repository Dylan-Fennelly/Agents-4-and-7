/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include <SFML/Graphics.hpp>
#include "ResourceIdentifiers.hpp"
#include <iostream>
#include "Application.hpp"

//We dont need this anymore as the title state handles assigning a joystick to the player class, it will oc
 unsigned int DetectJoystick()
{
	 sf::Joystick::update();//Because we dont have a window at this statge we need to manually update to get the default id for the joystick
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
	//int joystick_id = DetectJoystick();
	//TextureHolder game_textures;
	try
	{
		Application app;
		app.Run();
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}
}
//
//#include <SFML/Graphics.hpp>
//#include <SFML/Window.hpp>
//#include <iostream>
//#include <cmath>
//#include <map>
//
//const float DEAD_ZONE = 15.f; // Adjust this value for the dead zone as needed
//
//bool hasSignificantChange(unsigned int joystickId, sf::Joystick::Axis axis, float position) {
//    static std::map<std::pair<unsigned int, sf::Joystick::Axis>, float> previousPositions;
//
//    auto key = std::make_pair(joystickId, axis);
//    auto it = previousPositions.find(key);
//
//    // If it's the first time this axis is being checked, store its value
//    if (it == previousPositions.end()) {
//        previousPositions[key] = position;
//        return true;
//    }
//
//    float prevPosition = it->second;
//    previousPositions[key] = position;
//
//    // Check if the change is significant (this can be adjusted)
//    return std::abs(position - prevPosition) > 5.f; // Threshold for significant change
//}
//
//int main() {
//    sf::RenderWindow window(sf::VideoMode(800, 600), "Joystick Input Test");
//
//    // Track the connected state of joysticks
//    bool joystickConnected[sf::Joystick::Count] = { false };
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
//            bool isCurrentlyConnected = sf::Joystick::isConnected(i);
//
//            // Check if a joystick has been disconnected
//            if (isCurrentlyConnected != joystickConnected[i]) {
//                if (isCurrentlyConnected) {
//                    std::cout << "Joystick " << i << " has been connected!" << std::endl;
//                    sf::Joystick::Identification id = sf::Joystick::getIdentification(i);
//                    std::cout << " - Name: " << id.name.toAnsiString() << std::endl;
//                    std::cout << " - Vendor ID: " << id.vendorId << std::endl;
//                    std::cout << " - Product ID: " << id.productId << std::endl;
//                    std::cout << " - Button Count: " << sf::Joystick::getButtonCount(i) << std::endl;
//                }
//                else {
//                    std::cout << "Joystick " << i << " has been disconnected!" << std::endl;
//                }
//                joystickConnected[i] = isCurrentlyConnected;
//            }
//
//            if (isCurrentlyConnected) {
//                // Check button states
//                for (unsigned int button = 0; button < sf::Joystick::getButtonCount(i); ++button) {
//                    try {
//                        if (sf::Joystick::isButtonPressed(i, 6)) {
//                            continue;
//                        }
//                        if (sf::Joystick::isButtonPressed(i, button)) {
//                            std::cout << "Joystick " << i << " - Button " << button << " is pressed!" << std::endl;
//                        }
//                    }
//                    catch (const std::exception& e) {
//                        std::cerr << "Error checking button state: " << e.what() << std::endl;
//                    }
//                }
//
//                // Check axis movement with dead zone and significant change filtering
//                for (int axis = sf::Joystick::X; axis <= sf::Joystick::PovY; ++axis) {
//                    if (sf::Joystick::hasAxis(i, static_cast<sf::Joystick::Axis>(axis))) {
//                        try {
//                            float position = sf::Joystick::getAxisPosition(i, static_cast<sf::Joystick::Axis>(axis));
//
//                            // Apply dead zone and check for significant changes
//                            if (std::abs(position) > DEAD_ZONE && hasSignificantChange(i, static_cast<sf::Joystick::Axis>(axis), position)) {
//                                std::cout << "Joystick " << i << " - Axis "
//                                    << axis << " moved to " << position << std::endl;
//                            }
//                        }
//                        catch (const std::exception& e) {
//                            std::cerr << "Error checking axis position: " << e.what() << std::endl;
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
//
