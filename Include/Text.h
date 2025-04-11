#pragma once

#include <codecvt>
#include <locale>

#include "Config.h"
#include "ResourceManagement.h"

class Text : public sf::Drawable, public sf::Transformable
{
	//变量
private:
	std::shared_ptr<sf::Font> font;
	std::shared_ptr<sf::Texture> background_texture;
	sf::Text text;
	static sf::Sprite* sprite;
	static unsigned int shared_sprite_counter;
	sf::IntRect textureRects[9];
	sf::FloatRect targetRects[9];

	// 定义边距
	const int margin = 4;

	int background_width;
	int left;

	char mode;	//排版模式，0代表居中，1代表靠左对齐

	sf::Vector2f position;

	//函数
public:
	Text();
	Text(std::string i_message);
	~Text();

	void setMessage(std::wstring& i_message);
	void setFont(int ID);
	void setPosition(sf::Vector2f coord);
	void setPosition(float x, float y);
	void setCenter(sf::Vector2f coord);
	void setFillColor(sf::Color color);
	void setCharacterSize(int size);

	//强制更改背景的宽度，可以用于对齐多个文本
	void setBackgroundWidthForcely(int i_width);
	void setBackgroundWidthForcely(int i_width, char mode);

	int getBackgroundWidth();

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::FloatRect getGloabalBounds() const;
	sf::FloatRect getTextBounds() const;

	std::wstring getMessage() const;

private:
	void initBackground();
};

