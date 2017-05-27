#pragma once
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>
#include <PlayerInfo.h>

int main(){

	//Init grids
	sf::Texture blue_grid;
	blue_grid.loadFromFile("./../Resources/Images/blue_grid.png");
	Grid tempGrid(sf::Vector2i(0, 0), blue_grid);	

	//Bool to control flow of the game
	bool gameReady = false;
	bool createMatch = false;
	//Init players vector
	std::vector<PlayerInfo*> playerList;
	playerList.push_back(new PlayerInfo("", ISA, tempGrid));
	//Match vector
	int matchID = 0;
	std::vector<Match*> matchList;
	sf::String message; //message to send to the clients

	//Init tcp variables
	sf::TcpListener listener;
	sf::Socket::Status statusListen;
	sf::Socket::Status statusAccept;
	sf::Socket::Status statusReceive;
	sf::Packet packet;
	int socketCount = 0;
	statusListen = listener.listen(5001);
	if (statusListen != sf::Socket::Done) {
		std::cout << "Error: " << statusListen << " al vincular puerto 5000" << "." << std::endl;
		system("pause");
		return 0;
	}

	listener.setBlocking(false);
	
	//Start main loop
	while (true) {
		
		packet.clear();
		//iteration for the two players
		for (int i = 0; i < playerList.size(); i++) {
			//If player connects for first time
			if (playerList[i]->firstContact) {
				statusAccept = listener.accept(playerList[i]->playerSocket);
				if (statusAccept == sf::Socket::Done) {
					int tempState;
					playerList[i]->playerSocket.setBlocking(false);
					playerList[i]->playerSocket.receive(packet);
					packet >> playerList[i]->name >> tempState;
					playerList[i]->currentState = (PlayerInfo::STATE)tempState;
					if (playerList[i]->currentState == PlayerInfo::STATE::Host) {
						playerList[i]->isInMatch = true;
						std::string matchName = "";
						int turns = 0;
						int maxTime = 0;
						int maxFleetSize = 0;
						packet >> matchName >> turns >> maxTime >> maxFleetSize;
						matchList.push_back(new Match(matchID, matchName, turns, maxTime, maxFleetSize));
						for (int j = 0; j < playerList.size(); j++) {
							if (playerList[j]->currentState == PlayerInfo::STATE::Search) {
								int matchSize = 1;
								packet.clear();
								packet << matchSize;
								packet << matchList[matchList.size() - 1]->id << matchList[matchList.size() - 1]->matchName << matchList[matchList.size() - 1]->currentPlayers;
								playerList[j]->playerSocket.send(packet);
							}
						}
						matchID++;
					}
					else if(playerList[i]->currentState == PlayerInfo::STATE::Search) {
						//SEND ALL MATCHES AVAILABLES
						packet.clear();
						int matchSize = matchList.size();
						std::cout << matchSize << std::endl;

						packet << matchSize;
						for (int j = 0; j < matchList.size(); j++) {
							packet << matchList[j]->id << matchList[j]->matchName << matchList[j]->currentPlayers;
						}
						playerList[i]->playerSocket.send(packet);
					}
					//players[i]->hasTurn = 1 * i; //"random" method to init the turn
					//packet.clear();
					//packet << i << players[i]->hasTurn;  // "i" refers to the faction (0 ISA, 1 RSF)
					//players[i]->playerSocket.send(packet);
					//while (statusAccept == sf::Socket::Partial) { //Handle partial send
					//	players[i]->playerSocket.send(packet);
					//	statusAccept = listener.accept(players[i]->playerSocket);
					//}
					playerList.push_back(new PlayerInfo("", ISA, tempGrid));
					packet.clear();
					playerList[i]->firstContact = false;
				}
			}
			else {
				//When we receive from the clients
				statusReceive = playerList[i]->playerSocket.receive(packet);
				if (statusReceive == sf::Socket::Done) {
					if (!playerList[i]->isInMatch) {
						bool matchExists = false;
						int matchId;
						packet >> matchId;
						//Search Mach
						for (int j = 0; j < matchList.size(); j++) {
							if (matchList[j]->id == matchID) {
								matchList[j]->currentPlayers++;
								playerList[i]->currentMatch = matchList[j];
								playerList[i]->isInMatch = true;
								playerList[i]->matchReady = true;
								matchExists = true;
								packet.clear();
								packet << true;
								playerList[i]->playerSocket.send(packet);
								//Search Other player
								for (int x = 0; x < playerList.size(); x++) {
									if (playerList[x]->currentState == PlayerInfo::STATE::Host && playerList[x]->currentMatch->id == matchList[j]->id) {
										playerList[x]->matchReady = true;
										packet.clear();
										packet << true;
										playerList[x]->playerSocket.send(packet);
									}
								}
							}
						}
						if (!matchExists) {
							packet << true;
							playerList[i]->playerSocket.send(packet);
						}
					}
					else {	//When client send grid information...
						if (!playerList[i]->isReady) {
							for (int x = 0; x < MAX_CELLS; x++) {
								for (int y = 0; y < MAX_CELLS; y++) {
									int value;
									packet >> value;
									playerList[i]->grid.SetCell(sf::Vector2i(y, x), value);	//Copy the information to the player grid
								}
							}
							playerList[i]->isReady = true; //Now player is ready
						}
						//When two players are ready, game starts
						if (playerList[0]->isReady && playerList[1]->isReady) {
							//Send to the players that game has started
							if (!gameReady) {
								gameReady = true;
								packet.clear();
								packet << gameReady;
								playerList[i]->playerSocket.send(packet);
								while (statusReceive == sf::Socket::Partial) {	//Handle partial send
									playerList[i]->playerSocket.send(packet);
									statusReceive = playerList[i]->playerSocket.receive(packet);
								}
								playerList[i]->playerSocket.send(packet);
								while (statusReceive == sf::Socket::Partial) {	//Handle partial send
									playerList[i]->playerSocket.send(packet);
									statusReceive = playerList[i]->playerSocket.receive(packet);
								}
								packet.clear();
							}
							else {
								//Start to receive the players shots
								packet >> playerList[i]->shotCoords.x >> playerList[i]->shotCoords.y;

								message = "";
								//Create opponents iterators
								int opponentIt;
								if (i == 0) opponentIt = 1;
								else opponentIt = 0;

								//Check the players shots
								if (playerList[opponentIt]->grid.GetCell(playerList[i]->shotCoords) == 0) { //When is 0, is water
									packet.clear();
									playerList[i]->hasTurn = false;	//Lose the turn
									playerList[i]->isImpact = false;
									playerList[opponentIt]->hasTurn = true;	//Opponent have turn now
									playerList[opponentIt]->isImpact = false;
								}
								else {	//Impact whit ship
									packet.clear();
									playerList[i]->hasTurn = true;		//Player still having turn
									playerList[i]->isImpact = true;
									playerList[opponentIt]->hasTurn = false;
									playerList[opponentIt]->isImpact = true;

									int boatId = playerList[opponentIt]->grid.GetCell(playerList[i]->shotCoords); //Get impacted ship ID 
									playerList[opponentIt]->fleet.GetShip(boatId).TakeDamage();	//Take damage to that ship
									//If that ship have no live points...
									if (playerList[opponentIt]->fleet.GetShip(boatId).GetDamage() <= 0) {
										//Then ship destroyed
										playerList[opponentIt]->currentShips--;
										message = playerList[opponentIt]->fleet.GetShip(boatId).GetBoatName(playerList[opponentIt]->fleet.GetShip(boatId).GetType())
											+ " SHIP HAS BEEN DESTROYED !";
									}
								}
								//If there are no ships alive, the game is over
								if (playerList[opponentIt]->currentShips <= 0) {
									message = "GameOver";
								}
								//Put all the information to the packet and send to the player
								packet << playerList[i]->hasTurn << playerList[i]->isImpact << playerList[i]->shotCoords.x << playerList[i]->shotCoords.y << message;
								playerList[i]->playerSocket.send(packet);
								while (statusReceive == sf::Socket::Partial) {	//Handle partial send
									playerList[i]->playerSocket.send(packet);
									statusReceive = playerList[i]->playerSocket.receive(packet);
								}
								packet.clear();
								//Put all the information to the packet and send to the other player
								packet << playerList[opponentIt]->hasTurn << playerList[opponentIt]->isImpact << playerList[i]->shotCoords.x << playerList[i]->shotCoords.y << message;
								playerList[i]->playerSocket.send(packet);
								while (statusReceive == sf::Socket::Partial) {	//Handle partial send
									playerList[i]->playerSocket.send(packet);
									statusReceive = playerList[i]->playerSocket.receive(packet);
								}
							}
						}
					}
				}
				//When player disconnects from the server
				else if (statusReceive == sf::Socket::Disconnected) {
					message = "Disconnection";
					packet << playerList[i]->hasTurn << playerList[i]->isImpact << -15 << -15 << message; //the only important variable is "message"
					if (i == 0) {
						playerList[i]->playerSocket.send(packet);
						while (statusReceive == sf::Socket::Partial) {		//Handle partial send
							playerList[i]->playerSocket.send(packet);
							statusReceive = playerList[i]->playerSocket.receive(packet);
						}
					}
					else {
						playerList[i]->playerSocket.send(packet);
						while (statusReceive == sf::Socket::Partial) {	//Handle partial send
							playerList[i]->playerSocket.send(packet);
							statusReceive = playerList[i]->playerSocket.receive(packet);
						}
					}
					//Disconnect and delete sockets  and palyers
					playerList[i]->playerSocket.disconnect();
					//playerList[i]->playerSocket.disconnect();
					//delete lobbySockets[0], lobbySockets[1];
					//delete playerList[0], playerList[1];
					return 0;
				}
			}
		}
	}
}


