/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
/*
This code was prompted from chatgpt https://chatgpt.com/share/67eaccbe-f338-8001-88fe-34eb2ef7129e
With Modifications to make it work with our solution.
It provides a text box that interacted with a modern level convenince(copy,paste etc)


*/

/// <summary>
/// TextBox class that allows the user to input text - It is the responsibility of a game state
/// to handle the text input
/// </summary>
class TextBox 
{
public:
    TextBox(sf::Font& font, sf::Vector2f position, sf::Vector2f size, unsigned int charSize, unsigned int maxChars);

    void handleEvent(const sf::Event& event);
    void update();
    void draw(sf::RenderWindow& window, bool showCursor);
    sf::RectangleShape getBox();
    std::string getText() const;
    void setText(const std::string& newText);

private:
    sf::RectangleShape box;
    sf::Text text;
    sf::RectangleShape cursor;
    sf::RectangleShape highlight;

    std::string inputText;
    int cursorPos = 0;
    int selectionStart = -1;
    bool shiftPressed = false;
    bool ctrlPressed = false;
    unsigned int maxChars;

    sf::Clock cursorTimer;
    bool cursorVisible = true;

    void updateCursor();
    void updateHighlight();
};
