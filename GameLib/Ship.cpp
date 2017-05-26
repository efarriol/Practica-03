#include "Ship.h"



Ship::Ship(sf::Vector2i _position, ShipType _shipType, Faction faction, sf::Texture &texture)
{
	position = _position;
	isRotated = false;
	shipType = _shipType;
	sprite.setTexture(texture);
	isPlaced = false;
	//Init sprite and damage in relation to ship type
	switch (shipType)
	{
		default:
		case BIG:
			sprite.setTextureRect(sf::IntRect(0 + faction * 64,0,64,256));
			damage = 4;
			break;
		case MEDIUM:
			sprite.setTextureRect(sf::IntRect(128 + faction * 64, 0, 64, 192));
			damage = 3;
			break;
		case SMALL:
			sprite.setTextureRect(sf::IntRect(256 + faction * 64, 0, 64, 128));
			damage = 2;
			break;
	}
	sprite.setPosition(position.x, position.y);
	sprite.setScale(1, 1);
}

Ship::Ship()
{
}


Ship::~Ship()
{
}

void Ship::Update()
{
	if(!isPlaced){
		sprite.setPosition(position.x, position.y);
	}
	if (isRotated) {
		sprite.setRotation(-90);
		sprite.setPosition(position.x, position.y+64);
	}
	else sprite.setRotation(0);
}

void Ship::SetPosition(int _x, int _y)
{
	position.x = _x; 
	position.y = _y;
}

void Ship::SetPlaced(bool _isPlaced)
{
	isPlaced = _isPlaced;
}

sf::Vector2i Ship::GetPosition()
{
	return position;
}

bool Ship::GetPlaced()
{
	return isPlaced;
}

ShipType Ship::GetType()
{
	return shipType;
}

sf::String Ship::GetBoatName(ShipType _shipType) {
	switch (_shipType) {
		case BIG:
			return sf::String("A DEMOLISHER");
			break;
		case MEDIUM:
			return sf::String("A CRUISER");
			break;
		case SMALL:
			return sf::String("AN INTERCEPTOR");
			break;
	}
}

bool Ship::GetRotation()
{
	return isRotated;
}

void Ship::SetRotation()
{
	if (isRotated) isRotated = false;
	else isRotated = true;
}

int Ship::GetDamage()
{
	return damage;
}

void Ship::TakeDamage()
{
	damage -= 1;
}

void Ship::Render(sf::RenderWindow &window)
{
	window.draw(sprite);
}

void Ship::ChangeSprite(Faction faction)
{
	switch (shipType)
	{
		default:
		case BIG:
			sprite.setTextureRect(sf::IntRect(0 + faction * 64, 0, 64, 256));
			damage = 4;
			break;
		case MEDIUM:
			sprite.setTextureRect(sf::IntRect(128 + faction * 64, 0, 64, 192));
			damage = 3;
			break;
		case SMALL:
			sprite.setTextureRect(sf::IntRect(256 + faction * 64, 0, 64, 128));
			damage = 2;
			break;
	}
}
