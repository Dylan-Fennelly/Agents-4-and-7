#include "TextBox.hpp"
#include <iostream>

TextBox::TextBox(sf::Font& font, sf::Vector2f position, sf::Vector2f size, unsigned int charSize, unsigned int maxChars)
    : maxChars(maxChars)
{
    // TextBox Setup
    box.setSize(size);
    box.setFillColor(sf::Color::White);
    box.setOutlineColor(sf::Color::Black);
    box.setOutlineThickness(2);
    box.setPosition(position);

    // Text Setup
    text.setFont(font);
    text.setCharacterSize(charSize);
    text.setFillColor(sf::Color::Black);
    text.setPosition(position.x + 5, position.y + 5);

    // Cursor Setup
    cursor.setSize(sf::Vector2f(2, charSize));
    cursor.setFillColor(sf::Color::Black);

    // Highlight Setup
    highlight.setFillColor(sf::Color(173, 216, 230, 150));
}

void TextBox::handleEvent(const sf::Event& event) 
{
    if (event.type == sf::Event::KeyPressed) 
    {
        if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::RShift)
        {
            shiftPressed = true;
        }

        if (event.key.code == sf::Keyboard::LControl || event.key.code == sf::Keyboard::RControl)
        {
            ctrlPressed = true;
        }


        // Handle Copy-Paste-Select
        if (ctrlPressed) 
        {
            if (event.key.code == sf::Keyboard::A)
            {
                selectionStart = 0;
                cursorPos = inputText.size();
            }
            if (event.key.code == sf::Keyboard::C && selectionStart != -1)
            {
                sf::Clipboard::setString(inputText.substr(std::min(cursorPos, selectionStart),
                    std::max(cursorPos, selectionStart) - std::min(cursorPos, selectionStart)));
            }
            if (event.key.code == sf::Keyboard::X && selectionStart != -1) 
            {
                int start = std::min(cursorPos, selectionStart);
                int end = std::max(cursorPos, selectionStart);
                sf::Clipboard::setString(inputText.substr(start, end - start));
                inputText.erase(start, end - start);
                cursorPos = start;
                selectionStart = -1;
            }
            if (event.key.code == sf::Keyboard::V)
            {
                std::string pasteText = sf::Clipboard::getString();
                if (selectionStart != -1) 
                {
                    int start = std::min(cursorPos, selectionStart);
                    int end = std::max(cursorPos, selectionStart);
                    inputText.erase(start, end - start);
                    cursorPos = start;
                    selectionStart = -1;
                }
                if (inputText.size() + pasteText.size() <= maxChars)
                {
                    inputText.insert(cursorPos, pasteText);
                }

                cursorPos += pasteText.size();
            }
        }

        // Move cursor with arrow keys
        if (event.key.code == sf::Keyboard::Left && cursorPos > 0)
        {
            cursorPos--;
            if (shiftPressed) selectionStart = (selectionStart == -1) ? cursorPos + 1 : selectionStart;
            else selectionStart = -1;
        }
        if (event.key.code == sf::Keyboard::Right && cursorPos < inputText.size())
        {
            cursorPos++;
            if (shiftPressed) selectionStart = (selectionStart == -1) ? cursorPos - 1 : selectionStart;
            else selectionStart = -1;
        }
    }

    if (event.type == sf::Event::KeyReleased) 
    {
        if (event.key.code == sf::Keyboard::LShift || event.key.code == sf::Keyboard::RShift)
        {
            shiftPressed = false;
        }

        if (event.key.code == sf::Keyboard::LControl || event.key.code == sf::Keyboard::RControl)
        {
            ctrlPressed = false;
        }

    }

    if (event.type == sf::Event::TextEntered) 
    {
        if (event.text.unicode == 8)// Backspace
        {
            if (selectionStart != -1)
            {
                int start = std::min(cursorPos, selectionStart);
                int end = std::max(cursorPos, selectionStart);
                inputText.erase(start, end - start);
                cursorPos = start;
                selectionStart = -1;
            }
            else if (cursorPos > 0) 
            {
                inputText.erase(cursorPos - 1, 1);
                cursorPos--;
            }
        }
        else if (event.text.unicode >= 32 && event.text.unicode <= 126)  // Printable characters
        {
            if (selectionStart != -1)
            {
                int start = std::min(cursorPos, selectionStart);
                int end = std::max(cursorPos, selectionStart);
                inputText.erase(start, end - start);
                cursorPos = start;
                selectionStart = -1;
            }
            if (inputText.size() < maxChars) 
            {
                inputText.insert(cursorPos, 1, static_cast<char>(event.text.unicode));
                cursorPos++;
            }
        }
    }

    updateCursor();
    updateHighlight();
}

void TextBox::update()
{
    if (cursorTimer.getElapsedTime().asSeconds() > 0.5) 
    {
        cursorVisible = !cursorVisible;
        cursorTimer.restart();
    }
}

void TextBox::draw(sf::RenderWindow& window, bool showCursor)
{
    window.draw(box);
    if (selectionStart != -1) window.draw(highlight);
    window.draw(text);
    if (cursorVisible&&showCursor) window.draw(cursor);
}

sf::RectangleShape TextBox::getBox()
{
	return box;
}

std::string TextBox::getText() const 
{
    return inputText;
}

void TextBox::setText(const std::string& newText) 
{
    inputText = newText.substr(0, maxChars);
    cursorPos = inputText.size();
}

void TextBox::updateCursor()
{
    text.setString(inputText);
    sf::Text tempText;
    tempText.setFont(*text.getFont()); // Dereference the pointer to get the sf::Font object
	tempText.setCharacterSize(text.getCharacterSize());
	tempText.setString(inputText.substr(0, cursorPos));
    cursor.setPosition(text.getPosition().x + tempText.getLocalBounds().width, text.getPosition().y);
}

void TextBox::updateHighlight() 
{
    if (selectionStart != -1) 
    {
        int start = std::min(cursorPos, selectionStart);
        int end = std::max(cursorPos, selectionStart);
		sf::Text selectedText;
		selectedText.setFont(*text.getFont());
		selectedText.setCharacterSize(text.getCharacterSize());
		selectedText.setString(inputText.substr(start, end - start));
        highlight.setSize(sf::Vector2f(selectedText.getLocalBounds().width, 30));
		sf::Text tempText;
		tempText.setFont(*text.getFont());
		tempText.setCharacterSize(text.getCharacterSize());
		tempText.setString(inputText.substr(0, start));

		highlight.setPosition(text.getPosition().x + tempText.getLocalBounds().width, text.getPosition().y);
        //highlight.setPosition(text.getPosition().x + sf::Text(inputText.substr(0, start), text.getFont(), text.getCharacterSize()).getLocalBounds().width, text.getPosition().y);
    }
}
