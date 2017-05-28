#pragma once
#include <SFML\Graphics.hpp>
#include "Fleet.h"
#include <iostream>
#include "Match.h"

#define TIME 5
#define MAX_WORDS 10

class PlayerInfo
{
public:
	int id;
	Fleet fleet;
	Grid grid;
	int faction;
	std::string name;
	std::vector<std::string> messages;
	std::string input;
	int score;
	bool isHost = false;
	bool matchReady = false;
	bool isInMatch = false;
	bool isReady = false;
	bool hasTurn;
	bool isImpact;
	bool firstContact = true;
	enum STATE {None, Search, Host, Playing};
	STATE currentState = None;
	sf::TcpSocket playerSocket;
	sf::Vector2i shotCoords;
	std::vector<sf::Vector2i> coordRegister;
	int currentShips = MAX_SHIPS;
	Match *currentMatch;
	PlayerInfo* opponent;

	//Constructor
	PlayerInfo(int _id, std::string _name, Faction _faction, Grid &_grid) : id(_id), grid(_grid), fleet(_faction, "./../Resources/Images/Spaceships.png", _grid) {
		name = _name;
		faction = _faction;
		score = 0;
	};

	~PlayerInfo();
};