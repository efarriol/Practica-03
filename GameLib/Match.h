#pragma once
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "Grid.h"

class Match
{
public:
	int id;
	std::string matchName;
	//Match settings
	int turns; 
	int maxTime;
	int maxFleetSize;
	int currentPlayers = 0;
	//Init grids
	sf::Texture blue_grid;
	Grid grid;
	Grid grid2;

	Match( int _id, std::string _matchName, int _turns, int _maxTime, int _maxFleetSize) : grid(sf::Vector2i(0,0), blue_grid), grid2(sf::Vector2i(0, 0), blue_grid){
		id = _id;
		matchName = _matchName;
		turns = _turns;
		maxTime = _maxTime;
		maxFleetSize = _maxFleetSize;
	};
	~Match();
};

