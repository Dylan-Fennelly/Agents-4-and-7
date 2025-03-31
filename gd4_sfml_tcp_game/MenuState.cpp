/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "MenuState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Button.hpp"

MenuState::MenuState(StateStack& stack, Context context)
    : State(stack, context)
	, m_text_box(context.fonts->Get(Font::kMain), sf::Vector2f(100, 100), sf::Vector2f(200, 50), 20, 20)
	, m_text_box_Selected(false)

{
    sf::Texture& texture = context.textures->Get(TextureID::kTitleScreen);

    m_background_sprite.setTexture(texture);

    auto play_button = std::make_shared<gui::Button>(context);
    play_button->setPosition(100, 300);
    play_button->SetText("Play");
    play_button->SetCallback([this]()
    {
        RequestStackPop();
        RequestStackPush(StateID::kGame);
    });

    auto host_play_button = std::make_shared<gui::Button>(context);
    host_play_button->setPosition(100, 350);
    host_play_button->SetText("Host");
    host_play_button->SetCallback([this]()
        {
            RequestStackPop();
            RequestStackPush(StateID::kHostGame);
        });

    auto join_play_button = std::make_shared<gui::Button>(context);
    join_play_button->setPosition(100, 400);
    join_play_button->SetText("Join");
    join_play_button->SetCallback([this]()
        {
            RequestStackPop();
            RequestStackPush(StateID::kJoinGame);
        });

    auto settings_button = std::make_shared<gui::Button>(context);
    settings_button->setPosition(100, 450);
    settings_button->SetText("Settings");
    settings_button->SetCallback([this]()
    {
        RequestStackPush(StateID::kSettings);
    });

    auto exit_button = std::make_shared<gui::Button>(context);
    exit_button->setPosition(100, 500);
    exit_button->SetText("Exit");
    exit_button->SetCallback([this]()
    {
            RequestStackPop();
    });

    m_gui_container.Pack(play_button);
    m_gui_container.Pack(host_play_button);
    m_gui_container.Pack(join_play_button);
    m_gui_container.Pack(settings_button);
    m_gui_container.Pack(exit_button);

    //Play the music
    context.music->Play(MusicThemes::kMenuTheme);
}

void MenuState::Draw()
{
    sf::RenderWindow& window = *GetContext().window;
    window.setView(window.getDefaultView());
    window.draw(m_background_sprite);
    window.draw(m_gui_container);

    m_text_box.draw(window, m_text_box_Selected);
}

bool MenuState::Update(sf::Time dt)
{
    if (m_text_box_Selected)
    {
        m_text_box.update();
    }
    return true;
}

bool MenuState::HandleEvent(const sf::Event& event)
{

    // Handle clicks to toggle text box selection
    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
        if (m_text_box.getBox().getGlobalBounds().contains(mousePos))
        {
            m_text_box_Selected = true;
        }
        else 
        {
            m_text_box_Selected = false;
        }
    }

    // Pass events only if text box is selected
    if (m_text_box_Selected) 
    {
        m_text_box.handleEvent(event);
    }
    else 
    {
        m_gui_container.HandleEvent(event);
    }
    return true;
}

