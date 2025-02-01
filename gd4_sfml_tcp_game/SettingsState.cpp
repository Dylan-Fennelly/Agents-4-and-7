#include "SettingsState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

SettingsState::SettingsState(StateStack& stack, Context context)
	: State(stack, context)
	, m_gui_container(context)
{
	m_background_sprite.setTexture(context.textures->Get(TextureID::kTitleScreen));

	//Build key binding buttons and labels
	AddButtonLabel(ButtonFunction::kConfirm, 150.f, "Fire", context);
	AddButtonLabel(ButtonFunction::kCancel, 200.f, "Missile Fire", context);
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

	//Iterate through all of the key binding buttons to see if they are being presssed, waiting for the user to enter a key
	for (std::size_t button = 0; button < static_cast<int>(ButtonFunction::kButtonCount); ++button)
	{
		if (m_binding_buttons[button]->IsActive())
		{
			is_key_binding = true;

			// Handle gamepad button input
			if (event.type == sf::Event::JoystickButtonPressed)
			{
				Action actionToBind = GetContext().player->GetAssignedAction(GetContext().player->GetGamepad().getButton(static_cast<ButtonFunction>(button)));
				GetContext().player->AssignGamepadButton(actionToBind, event.joystickButton.button);
				m_binding_buttons[button]->Deactivate();
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
	Player& player = *GetContext().player;
	for (std::size_t i = 0; i < static_cast<int>(ButtonFunction::kButtonCount); ++i)
	{
		//sf::Keyboard::Key key = player.GetAssignedKey(static_cast<Action>(i));
		//m_binding_labels[i]->SetText(Utility::toString(key));
		std::string str = std::to_string(GetContext().player->GetAssignedGamepadButton(GetContext().player->GetAssignedAction(GetContext().player->GetGamepad().getButton(static_cast<ButtonFunction>(i)))));
		m_binding_labels[i]->SetText(str);
	}
}

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
