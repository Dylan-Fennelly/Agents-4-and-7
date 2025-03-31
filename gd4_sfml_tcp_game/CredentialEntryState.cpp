#include "CredentialEntryState.hpp"
#include "Button.hpp"
#include <fstream>
#include <iostream>
CredentialEntryState::CredentialEntryState(StateStack& stack, Context context, bool is_host)
	: State(stack, context)
	, m_is_host(is_host)
    
{
    //Set the background
    sf::Texture& texture = context.textures->Get(TextureID::kTitleScreen);
	m_background_sprite.setTexture(texture);

	//Create the text boxes
	TextBox ipTextbox(context.fonts->Get(Font::kMain), sf::Vector2f(100, 100), sf::Vector2f(200, 50), 20, 20);
	std::ifstream input_file("ip.txt");
	std::string ip_address;
	if (input_file >> ip_address)
	{
		ipTextbox.setText(ip_address);
	}

	TextBox userNameTextbox(context.fonts->Get(Font::kMain), sf::Vector2f(100, 200), sf::Vector2f(200, 50), 20, 20);
	std::ifstream input_file2("username.txt");
	std::string username;
	if (input_file2 >> username)
	{
		userNameTextbox.setText(username);
	}

	//Add the text boxes to the vector
	if (!m_is_host)
	{
		m_text_boxes.push_back(ipTextbox);
	}
	m_text_boxes.push_back(userNameTextbox);

	auto confirm_button = std::make_shared<gui::Button>(context);
	confirm_button->setPosition(100, 300);
	if (m_is_host)
	{
		confirm_button->SetText("Host Game");
	}
	else
	{
		confirm_button->SetText("Join Game");
	}
	confirm_button->SetCallback([this]()
		{
			if (!m_is_host)
			{
				std::ofstream output_file("ip.txt");
				std::string local_address = m_text_boxes[0].getText();
				output_file << local_address;
			}


			std::ofstream output_file2("username.txt");
			std::string username = m_text_boxes[m_text_boxes.size() - 1].getText();
			output_file2 << username;

			RequestStackPop();
			if (m_is_host)
			{
				RequestStackPush(StateID::kHostGame);

			}
			else
			{
				RequestStackPush(StateID::kJoinGame);

			}
		});

	auto back_button = std::make_shared<gui::Button>(context);
	back_button->setPosition(100, 350);
	back_button->SetText("Back");
	back_button->SetCallback([this]()
		{
			RequestStackPop();
			RequestStackPush(StateID::kMenu);
		});
	m_gui_container.Pack(confirm_button);
	m_gui_container.Pack(back_button);
	for (auto& textBox : m_text_boxes)
	{
		textBox.handleEvent(sf::Event());
	}

	context.music->Play(MusicThemes::kMenuTheme);

}

void CredentialEntryState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.setView(window.getDefaultView());
	window.draw(m_background_sprite);
	window.draw(m_gui_container);
	
	for (int i = 0; i<m_text_boxes.size(); i++)
	{
		if (i == m_text_box_Selected)
		{
			m_text_boxes[i].draw(window, true);
		}
		else
		{
			m_text_boxes[i].draw(window, false);
		}
	}
}

bool CredentialEntryState::Update(sf::Time dt)
{
	if (m_text_box_Selected != -1)
	{
		m_text_boxes[m_text_box_Selected].update();
	}
	return true;

}

bool CredentialEntryState::HandleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::Key::Escape)
		{
			m_text_box_Selected = -1;
		}
		else if (event.key.code == sf::Keyboard::Key::Tab)
		{
			if (m_text_box_Selected == -1)
			{
				m_text_box_Selected = 0;
			}
			else
			{
				m_text_box_Selected++;
				if (m_text_box_Selected >= m_text_boxes.size())
				{
					m_text_box_Selected = 0;
				}
			}
		}
	}

	if (event.type == sf::Event::MouseButtonPressed)
	{
		sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
		m_text_box_Selected = -1;
		for (int i = 0; i < m_text_boxes.size(); i++)
		{
			if (m_text_boxes[i].getBox().getGlobalBounds().contains(mousePos))
			{
				m_text_box_Selected = i;
				std::cout << "Selected: " << i << std::endl;
				break;
			}
		}
	}
	if (m_text_box_Selected == -1)
	{
		m_gui_container.HandleEvent(event); 
	}
	else
	{
		m_text_boxes[m_text_box_Selected].handleEvent(event);
	}
	return true;
}
