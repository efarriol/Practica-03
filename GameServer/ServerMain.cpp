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
	//Init players vector
	std::vector<PlayerInfo*> playerList;
	int playerID = 0;
	playerList.push_back(new PlayerInfo(playerID, "", ISA, tempGrid));
	playerID++;
	//Match vector
	int matchID = 0;
	int receivedID = 0;

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
						playerList[i]->isHost = true;
						playerList[i]->isInMatch = true;
						std::string matchName = "";
						int turns = 0;
						int maxTime = 0;
						int maxFleetSize = 0;
						packet >> matchName >> turns >> maxTime >> maxFleetSize;
						matchList.push_back(new Match(matchID, matchName, turns, maxTime, maxFleetSize));
						playerList[i]->currentMatch = matchList[matchList.size()-1];
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
						packet << matchSize;
						for (int j = 0; j < matchList.size(); j++) {
							packet << matchList[j]->id << matchList[j]->matchName << matchList[j]->currentPlayers;
						}
						playerList[i]->playerSocket.send(packet);
					}
					playerList.push_back(new PlayerInfo(playerID, "", ISA, tempGrid));
					playerID++;
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
						packet >> receivedID;
						//Search Mach
						for (int j = 0; j < matchList.size(); j++) {
							if (matchList[j]->id == receivedID) {
								if (matchList[j]->currentPlayers < 2) {
									matchList[j]->currentPlayers++;
									playerList[i]->currentMatch = matchList[j];
									playerList[i]->isInMatch = true;
									playerList[i]->matchReady = true;
									matchExists = true;
									packet.clear();
									packet << playerList[i]->id << true;
									playerList[i]->playerSocket.send(packet);
									//Search Other player
									for (int x = 0; x < playerList.size(); x++) {
										if (playerList[x]->currentState == PlayerInfo::STATE::Host && playerList[x]->isInMatch) {
											if (playerList[x]->currentMatch->id == receivedID) {
												playerList[x]->matchReady = true;
												playerList[x]->opponent = playerList[i];
												playerList[i]->opponent = playerList[x];
												packet.clear();
												packet << playerList[x]->id << true;
												playerList[x]->playerSocket.send(packet);
												break;
											}
										}
									}
									//Refresh server info to lobby clients
									for (int k = 0; k < playerList.size(); k++) {
										if (playerList[k]->currentState == PlayerInfo::STATE::Search) {
											packet.clear();
											packet << 1 << matchList[j]->id << matchList[j]->matchName << matchList[j]->currentPlayers;
											playerList[k]->playerSocket.send(packet);
										}
									}
								}
								else {
									packet << false;
									playerList[i]->playerSocket.send(packet);
								}
							}
						}
						if (!matchExists) {
							packet << false;
							playerList[i]->playerSocket.send(packet);
						}
					}
					else if(playerList[i]->matchReady){	//When match is ready
						if (!playerList[i]->isReady) { //When players are not ready yet(grid information)
							packet >> receivedID;
							for (int x = 0; x < MAX_CELLS; x++) {
								for (int y = 0; y < MAX_CELLS; y++) {
									int value;
									packet >> value;
									if(playerList[i]->isHost) playerList[i]->currentMatch->grid.SetCell(sf::Vector2i(y, x), value);	//Copy the information to the player grid
									else playerList[i]->currentMatch->grid2.SetCell(sf::Vector2i(y, x), value);
								}
							}
							if (playerList[i]->isHost) playerList[i]->grid = playerList[i]->currentMatch->grid;
							else playerList[i]->grid = playerList[i]->currentMatch->grid2;
							playerList[i]->isReady = true; //Now player is ready
						}
						//When two players are ready, game starts
						if (playerList[i]->isReady && playerList[i]->opponent->isReady) {
							//Send to the players that game has started
							if(playerList[i]->currentMatch->gameReady){
								//Start to receive the players shots
								packet >> playerList[i]->shotCoords.x >> playerList[i]->shotCoords.y;

								message = "";
																
								//Check the players shots
								if (playerList[i]->opponent->grid.GetCell(playerList[i]->shotCoords) == 0) { //When is 0, is water
									packet.clear();
									playerList[i]->hasTurn = false;	//Lose the turn
									playerList[i]->isImpact = false;
									playerList[i]->opponent->hasTurn = true;	//Opponent have turn now
									playerList[i]->opponent->isImpact = false;
								}
								else {	//Impact whit ship
									packet.clear();
									playerList[i]->hasTurn = true;		//Player still having turn
									playerList[i]->isImpact = true;
									playerList[i]->opponent->hasTurn = false;
									playerList[i]->opponent->isImpact = true;

									int boatId = playerList[i]->opponent->grid.GetCell(playerList[i]->shotCoords); //Get impacted ship ID 
									playerList[i]->opponent->fleet.GetShip(boatId).TakeDamage();	//Take damage to that ship
									//If that ship have no live points...
									if (playerList[i]->opponent->fleet.GetShip(boatId).GetDamage() <= 0) {
										//Then ship destroyed
										playerList[i]->opponent->currentShips--;
										message = playerList[i]->opponent->fleet.GetShip(boatId).GetBoatName(playerList[i]->opponent->fleet.GetShip(boatId).GetType())
											+ " SHIP HAS BEEN DESTROYED !";
									}
								}
								//If there are no ships alive, the game is over
								if (playerList[i]->opponent->currentShips <= 0) {
									message = "GameOver";
								}
								//Put all the information to the packet and send to the player
								packet << playerList[i]->hasTurn << playerList[i]->isImpact << playerList[i]->shotCoords.x << playerList[i]->shotCoords.y << message;
								playerList[i]->playerSocket.send(packet);
								packet.clear();
								//Put all the information to the packet and send to the other player
								packet << playerList[i]->opponent->hasTurn << playerList[i]->opponent->isImpact << playerList[i]->shotCoords.x << playerList[i]->shotCoords.y << message;
								playerList[i]->opponent->playerSocket.send(packet);

							}
							else {
								playerList[i]->currentMatch->gameReady = true;
								packet.clear();
								packet << playerList[i]->currentMatch->gameReady;
								playerList[i]->playerSocket.send(packet);
								playerList[i]->opponent->playerSocket.send(packet);
								playerList[i]->playerSocket.send(packet);
								packet.clear();
							}
						}
					}
				}
				//When player disconnects from the server
				else if (statusReceive == sf::Socket::Disconnected) {
					message = "Disconnection";
					packet << playerList[i]->hasTurn << playerList[i]->isImpact << -15 << -15 << message; //the only important variable is "message"
					playerList[i]->opponent->playerSocket.send(packet);
					//Disconnect and delete sockets  and palyers


					////Refresh server info to lobby clients
					//for (int k = 0; k < playerList.size(); k++) {
					//	if (playerList[k]->currentState == PlayerInfo::STATE::Search  && playerList[k]->id != playerList[i]->id &&playerList[k]->id != playerList[i]->opponent->id) {
					//		packet.clear();
					//		packet << 1 << playerList[i]->currentMatch->id << playerList[i]->currentMatch->matchName << 0;
					//		playerList[k]->playerSocket.send(packet);
					//	}
					//}

					//playerList[i]->playerSocket.disconnect();
					//playerList[i]->opponent->playerSocket.disconnect();
					////Delete match
					//delete playerList[i]->currentMatch;
					//delete playerList[i];
				}
			}
		}
	}
}


