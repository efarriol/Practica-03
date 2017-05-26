#pragma once
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#define CELL_SIZE 64
#define MAX_CELLS 10

class Grid
{
private:
	sf::Sprite sprite;
	int gridArray[MAX_CELLS][MAX_CELLS]{0};

public:
	Grid();
	Grid(sf::Vector2i _position, sf::Texture &texture);
	~Grid();

	void Render(sf::RenderWindow &window);
	void SetCell(sf::Vector2i position, int id);
	int GetCell(sf::Vector2i position);
};

