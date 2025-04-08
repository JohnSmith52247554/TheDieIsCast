#include "Bar.h"

Bar::Bar()
{
	spark_fliping = new Animation(6, 6, &spark_icon, 64, 64);
	spark_icon.setScale(0.7f * 4.f, 0.7f * 4.f);

	spark_info = new sf::Text;
	spark_info->setFont(*ResourceManagement::getFont(0));
	spark_info->setCharacterSize(20 * 16);
	spark_info->setScale(0.25, 0.25);

	six_digit_points << std::setw(6) << std::setfill('0') << points;
	minutes_and_seconds << std::setw(2) << std::setfill('0') << static_cast<int>(minute) << ':' << std::setw(2) << std::setfill('0') << static_cast<int>(second);
	info << six_digit_points.str() << "      " << "x" << spark_num << "    " << minutes_and_seconds.str();

	texture.create(300 * 4, 30 * 4);
}

Bar::~Bar()
{
	delete spark_fliping;
	delete spark_info;
}

void Bar::updateViewCenter(sf::Vector2f center)
{
	view_center = center;
}

void Bar::update()
{
	spark_fliping->setFrame();
	/*sf::Vector2f icon_position =
	{
		view_center.x - view.getSize().x / 2 + 79,
		view_center.y - view.getSize().y / 2 - 14
	};*/
	sf::Vector2f icon_position =
	{
		270,
		-70
	};
	spark_icon.setPosition(icon_position);
	/*sf::Vector2f info_position =
	{
		view_center.x - view.getSize().x / 2 + 8,
		view_center.y - view.getSize().y / 2 + 6
	};*/
	sf::Vector2f info_position =
	{
		8,
		6
	};
	spark_info->setPosition(info_position);
}

void Bar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	six_digit_points.str("");
	six_digit_points.clear();
	minutes_and_seconds.str("");
	minutes_and_seconds.clear();
	info.str("");
	info.clear();

	//texture.setView(view);
	texture.clear(sf::Color::Transparent);
	texture.draw(spark_icon);
	if (spark_info != nullptr)
	{
		six_digit_points << std::setw(6) << std::setfill('0') << points;
		minutes_and_seconds << std::setw(2) << std::setfill('0') << static_cast<int>(minute) << ':' << std::setw(2) << std::setfill('0') << static_cast<int>(second);
		info << six_digit_points.str() << "      " << "x" << spark_num << "    " << minutes_and_seconds.str();
		spark_info->setString(info.str());
		texture.draw(*spark_info);
	}

	bar_sprite.setTexture(texture.getTexture());
	if (view.getSize().x > SCREEN_WIDTH)
	{
		float scale = view.getSize().x / static_cast<float>(SCREEN_WIDTH) / 4;
		bar_sprite.setScale(scale, -scale);
	}
	else
		bar_sprite.setScale(0.25, -0.25);
	
	bar_sprite.setPosition({
		view_center.x - view.getSize().x / 2 + 8,
		view_center.y - view.getSize().y / 2 + bar_sprite.getGlobalBounds().height + 5
		});

	target.draw(bar_sprite);
}