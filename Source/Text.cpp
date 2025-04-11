#include "Text.h"

sf::Sprite* Text::sprite = nullptr;
unsigned int Text::shared_sprite_counter = 0U;

Text::Text()
{
	font = ResourceManagement::getFont(0);
	text.setFont(*font);
	text.setString("Hello World");
	text.setCharacterSize(30);
	text.setPosition(100, 100);
	background_texture = ResourceManagement::getTexture(7);
	background_width = -1;
	left = -1;
	mode = 0;

	if (shared_sprite_counter == 0U)
	{
		sprite = new sf::Sprite;
		sprite->setTexture(*background_texture);
	}
	shared_sprite_counter++;
}

Text::Text(std::string i_message)
	: Text::Text()
{
	text.setString(i_message);
}

Text::~Text()
{
	if (font != nullptr)
		font = nullptr;
	if (background_texture != nullptr)
		background_texture = nullptr;

	shared_sprite_counter--;
	if (shared_sprite_counter == 0U)
	{
		delete sprite;
		sprite = nullptr;
	}
}

void Text::setMessage(std::wstring& i_message)
{
	text.setString(i_message);
	initBackground();
}

void Text::setFont(int ID)
{
	font = ResourceManagement::getFont(ID);
	text.setFont(*font);
	initBackground();
}

void Text::setPosition(sf::Vector2f coord)
{
	position = coord;
	text.setPosition(coord);
	initBackground();
}

void Text::setPosition(float x, float y)
{
	position = { x, y };
	text.setPosition(x, y);
	initBackground();
}

void Text::setCenter(sf::Vector2f coord)
{
	if (mode == 0)
	{
		sf::FloatRect textBounds = text.getGlobalBounds();
		float x = coord.x - textBounds.width / 2;
		float y = coord.y - textBounds.height / 2;
		setPosition(x, y);
	}
	else if (mode == 1)
	{
		sf::FloatRect textBounds = text.getGlobalBounds();
		float x = coord.x - background_width / 2 + margin + 2;
		float y = coord.y - textBounds.height / 2;
		setPosition(x, y);
	}
}

void Text::setFillColor(sf::Color color)
{
	text.setFillColor(color);
	initBackground();
}

void Text::setCharacterSize(int size)
{
	text.setCharacterSize(size * 4);
	text.setScale(0.25, 0.25);
	initBackground();
}

void Text::setBackgroundWidthForcely(int i_width)
{
	background_width = i_width;
}

void Text::setBackgroundWidthForcely(int i_width, char i_mode)
{
	background_width = i_width;
	mode = i_mode;
}

int Text::getBackgroundWidth()
{
	int width = text.getGlobalBounds().width + margin * 2 + 4;
	if (background_width > width)
		width = background_width;

	return width;
}

void Text::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	for (int i = 0; i < 9; ++i)
	{
		sprite->setTextureRect(textureRects[i]);
		sprite->setPosition(targetRects[i].left, targetRects[i].top);
		sprite->setScale(targetRects[i].width / textureRects[i].width, targetRects[i].height / textureRects[i].height);
		target.draw(*sprite, states);
	}

	target.draw(text, states);
}

sf::FloatRect Text::getGloabalBounds() const
{
	sf::FloatRect textBounds = text.getGlobalBounds();
	textBounds.height += margin * 2 + 4;
	textBounds.width += margin * 2 + 4;
	textBounds.left -= margin + 2;
	textBounds.top -= margin + 2;
	if (background_width > textBounds.width)
	{
		textBounds.left -= (background_width - textBounds.width) / 2;
		textBounds.width = background_width;
	}
	return textBounds;
}

sf::FloatRect Text::getTextBounds() const 
{
	return text.getGlobalBounds();
}

std::wstring Text::getMessage() const
{
	return text.getString();
}

void Text::initBackground()
{
	// 使用九宫格缩放保证边缘不变形
	sf::FloatRect textBounds = text.getGlobalBounds();

	// 背景略微大于文字
	textBounds.height += margin * 2 + 4;
	textBounds.width += margin * 2 + 4;
	textBounds.left -= margin + 2;
	textBounds.top -= margin + 2;
	//强制设定宽度
	if (background_width > textBounds.width)
	{
		if (mode == 0)
		{
			textBounds.left -= (background_width - textBounds.width) / 2;
			textBounds.width = background_width;
		}
		else if (mode == 1)
		{
			textBounds.width = background_width;
		}
	}
	// 定义纹理顶点
	// 四角不拉伸
	textureRects[0] = sf::IntRect(0, 0, margin, margin);                     // 左上
	textureRects[1] = sf::IntRect(background_texture->getSize().x - margin, 0, margin, margin); // 右上
	textureRects[2] = sf::IntRect(0, background_texture->getSize().y - margin, margin, margin); // 左下
	textureRects[3] = sf::IntRect(background_texture->getSize().x - margin, background_texture->getSize().y - margin, margin, margin); // 右下

	// 四边单向拉伸
	textureRects[4] = sf::IntRect(margin, 0, background_texture->getSize().x - 2 * margin, margin); // 上
	textureRects[5] = sf::IntRect(margin, background_texture->getSize().y - margin, background_texture->getSize().x - 2 * margin, margin); // 下
	textureRects[6] = sf::IntRect(0, margin, margin, background_texture->getSize().y - 2 * margin); // 左
	textureRects[7] = sf::IntRect(background_texture->getSize().x - margin, margin, margin, background_texture->getSize().y - 2 * margin); // 右

	// 中心双向拉伸
	textureRects[8] = sf::IntRect(margin, margin, background_texture->getSize().x - 2 * margin, background_texture->getSize().y - 2 * margin);

	// 定义目标区域
	// 四角不拉伸
	targetRects[0] = sf::FloatRect(textBounds.left, textBounds.top, margin, margin);                     // 左上
	targetRects[1] = sf::FloatRect(textBounds.left + textBounds.width - margin, textBounds.top, margin, margin); // 右上
	targetRects[2] = sf::FloatRect(textBounds.left, textBounds.top + textBounds.height - margin, margin, margin); // 左下
	targetRects[3] = sf::FloatRect(textBounds.left + textBounds.width - margin, textBounds.top + textBounds.height - margin, margin, margin); // 右下

		// 四边单向拉伸
	targetRects[4] = sf::FloatRect(textBounds.left + margin, textBounds.top, textBounds.width - 2 * margin, margin); // 上
	targetRects[5] = sf::FloatRect(textBounds.left + margin, textBounds.top + textBounds.height - margin, textBounds.width - 2 * margin, margin); // 下
	targetRects[6] = sf::FloatRect(textBounds.left, textBounds.top + margin, margin, textBounds.height - 2 * margin); // 左
	targetRects[7] = sf::FloatRect(textBounds.left + textBounds.width - margin, textBounds.top + margin, margin, textBounds.height - 2 * margin); // 右

		// 中心双向拉伸
	targetRects[8] = sf::FloatRect(textBounds.left + margin, textBounds.top + margin, textBounds.width - 2 * margin, textBounds.height - 2 * margin);
}