#pragma once
#include <Windows.h>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "Ship.h"
#include "Grid.h"
#include <iostream>

#define MAX_SHIPS 5

class Fleet
{
private:
	Grid &grid;
	std::vector<Ship> ships;
	int shipCount = 0;
	int shipType = 2;	//shipType = 2 represents the big boat (ShipType enum), it's the first to be placed 
	bool canBePlaced, canBeRotated, isInsideGrid;
	sf::Texture shipTexture;

public:
	Fleet(Faction _fleetFaction, std::string texturePath, Grid &_grid);
	~Fleet();
	void PlaceFleet(sf::RenderWindow &window, sf::Event &evento, sf::Mouse &mouseEvent, bool &isPlaced);
	void Render(sf::RenderWindow &window);
	void ChangeFaction(Faction faction);
	Ship& GetShip(int id);	
};

