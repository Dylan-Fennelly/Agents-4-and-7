/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include "TextureID.hpp"
#include <string>
#include <SFML/Graphics.hpp>


class TextureHolder
{
public:
	void Load(TextureID id, const std::string& filename);
	sf::Texture& Get(TextureID id);
	const sf::Texture& Get(TextureID id) const;

private:
	std::map<TextureID, std::unique_ptr<sf::Texture>> m_texture_map;
};

