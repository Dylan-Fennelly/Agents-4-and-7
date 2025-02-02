/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "SettingsState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

SettingsState::SettingsState(StateStack& stack, Context context)
	: State(stack, context)
	, m_gui_container(context)
{
	m_background_sprite.setTexture(context.textures->Get(TextureID::kTitleScreen));

	//Build key binding buttons and labels
	AddButtonLabel(ButtonFunction::kConfirm, 150.f, "Fire/Confirm", context);
	AddButtonLabel(ButtonFunction::kCancel, 200.f, "Cancel", context);
	AddButtonLabel(ButtonFunction::kPause, 250.f, "Pause Button", context);


	UpdateLabels();

	auto back_button = std::make_shared<gui::Button>(context);
	back_button->setPosition(80.f, 350.f);
	back_button->SetText("Back");
	back_button->SetCallback(std::bind(&SettingsState::RequestStackPop, this));
	m_gui_container.Pack(back_button);
}

void SettingsState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.draw(m_background_sprite);
	window.draw(m_gui_container);
}

bool SettingsState::Update(sf::Time dt)
{
	return true;
}

bool SettingsState::HandleEvent(const sf::Event& event)
{
	bool is_key_binding = false;

	//Iterate through all of the key binding buttons to see if they are being presssed, waiting for the user to press the button
	//make sure this uses kButtonCount : kButtonCount is not the last in the enum but it is where we draw the limit for controls 
	for (std::size_t button = 0; button < static_cast<int>(ButtonFunction::kButtonCount); ++button)
	{
		if (m_binding_buttons[button]->IsActive())
		{
			is_key_binding = true;

			// Handle gamepad button input
			if (event.type == sf::Event::JoystickButtonPressed)
			{
				//We use the index of the loop to get the button function
				//We then get the gamepad, and use this to get the action assicatied with that buttonfunction
				//Finally we take the button that was pressed and assign it to the action and 
				//then update the labels
				if (event.joystickButton.joystickId == GetContext().player->GetGamepad().GetJoystickId())
				{
					Gamepad& gamepad = GetContext().player->GetGamepad();
					ButtonFunction function = static_cast<ButtonFunction>(button);

					Action action = gamepad.GetAction(function);
					gamepad.AssignButtonFunction(function, event.joystickButton.button);
					gamepad.AssignAction(action, function);
					//deselct the button
					m_binding_buttons[button]->Deactivate();
				}
				else if (event.joystickButton.joystickId == GetContext().player2->GetGamepad().GetJoystickId())
				{
					Gamepad& gamepad = GetContext().player2->GetGamepad();
					ButtonFunction function = static_cast<ButtonFunction>(button);
					Action action = gamepad.GetAction(function);
					gamepad.AssignButtonFunction(function, event.joystickButton.button);
					gamepad.AssignAction(action, function);
					//deselct the button
					m_binding_buttons[button]->Deactivate();
				}
			}
			break;
		}
	}

	//If pressed button changed key bindings, then update the labels
	if (is_key_binding)
	{
		UpdateLabels();
	}
	else
	{
		m_gui_container.HandleEvent(event);
	}
	return false;
}

void SettingsState::UpdateLabels()
{
    // Get the active player from the context.
    Player& player = *GetContext().player;
	Player& player2 = *GetContext().player2;
    
    // For each button function, update its label text.
    for (std::size_t i = 0; i < static_cast<int>(ButtonFunction::kButtonCount); ++i)
    {
        // Get the corresponding ButtonFunction
        ButtonFunction function = static_cast<ButtonFunction>(i);
        // Get the mapped joystick button number from the gamepad
        unsigned int buttonNumber1 = player.GetGamepad().GetButton(function);
		unsigned int buttonNumber2 = player2.GetGamepad().GetButton(function);
        // Convert that to a string (you might have a helper to convert numbers to a nicer string)
        std::string buttonString = "P1:"+std::to_string(buttonNumber1) +"\tP2:" +std::to_string(buttonNumber2);
        // Update the label text accordingly
        m_binding_labels[i]->SetText(buttonString);
    }
}

/// <summary>
/// Adds a button and label to the settings screen it uses the index of the button to add the label to the correct button 
/// </summary>
/// <param name="buttonFunction"></param>
/// <param name="y">The y position of the button</param>
/// <param name="text">The text that will be displayed on the button</param>
/// <param name="context"> The context of the game</param>
void SettingsState::AddButtonLabel(ButtonFunction buttonFunction, float y, const std::string& text, Context context)
{
	m_binding_buttons[static_cast<int>(buttonFunction)] = std::make_shared<gui::Button>(context);
	m_binding_buttons[static_cast<int>(buttonFunction)]->setPosition(80.f, y);
	m_binding_buttons[static_cast<int>(buttonFunction)]->SetText(text);
	m_binding_buttons[static_cast<int>(buttonFunction)]->SetToggle(true);

	
	m_binding_labels[static_cast<int>(buttonFunction)] = std::make_shared<gui::Label>("", *context.fonts);
	m_binding_labels[static_cast<int>(buttonFunction)]->setPosition(300.f, y + 15.f);


	m_gui_container.Pack(m_binding_buttons[static_cast<int>(buttonFunction)]);
	m_gui_container.Pack(m_binding_labels[static_cast<int>(buttonFunction)]);
}
