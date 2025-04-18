/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "MenuState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Button.hpp"

MenuState::MenuState(StateStack& stack, Context context)
    :State(stack, context)
{
    sf::Texture& texture = context.textures->Get(TextureID::kTitleScreen);

    m_background_sprite.setTexture(texture);

    auto play_button = std::make_shared<gui::Button>(context);
    play_button->setPosition(100, 300);
    play_button->SetText("Join Game");
    play_button->SetCallback([this]()
        {
            RequestStackPop();
            RequestStackPush(StateID::kCredentialEntryJoin);
        });

    auto host_play_button = std::make_shared<gui::Button>(context);
    host_play_button->setPosition(100, 350);
    host_play_button->SetText("Host Game");
    host_play_button->SetCallback([this]()
        {
            RequestStackPop();
            RequestStackPush(StateID::kCredentialEntryHost);
        });

    auto settings_button = std::make_shared<gui::Button>(context);
    settings_button->setPosition(100, 400);
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
}

bool MenuState::Update(sf::Time dt)
{
    return true;
}

bool MenuState::HandleEvent(const sf::Event& event)
{
    m_gui_container.HandleEvent(event);
    return true;
}