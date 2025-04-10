/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#pragma once
#include "Component.hpp"
#include "ResourceIdentifiers.hpp"

namespace gui
{
	class Label : public Component
	{
	public:
		typedef std::shared_ptr<Label> Ptr;

	public:
		Label(const std::string& text, const FontHolder& fonts);
		virtual bool IsSelectable() const override;
		void SetText(const std::string& text);
		void HandleEvent(const sf::Event& event) override;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	private:
		sf::Text m_text;

	};
}

