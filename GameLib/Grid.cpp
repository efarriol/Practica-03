#include "Grid.h"


Grid::Grid() {

}

Grid::Grid(sf::Vector2i _position, sf::Texture & texture)
{
	sprite.setTexture(texture);
	sprite.setPosition(_position.x, _position.y);
	sprite.setScale(1, 1);
}

 
void Grid::SetCell(sf::Vector2i position, int id)
{
	gridArray[position.x][position.y] = id;
}


int Grid::GetCell(sf::Vector2i position)
{
	return gridArray[position.x][position.y];
}

void Grid::Render(sf::RenderWindow & window)
{
	window.draw(sprite);
}

Grid::~Grid()
{
}