#pragma once
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <PlayerInfo.h>
#include <Ship.h>
#include <Fleet.h>
#include <Grid.h>

struct LobbyMatch {
	int id;
	std::string name;
	int numPlayers;
	LobbyMatch(int _id, std::string _name, int _numPlayers) :id(_id), name(_name), numPlayers(_numPlayers) {};
};


//Send and manage shot information
void SendFunction(sf::TcpSocket &socket, sf::RenderWindow &window, sf::Event& evento, sf::Mouse& mouseEvent, sf::Socket::Status &statusReceive, PlayerInfo &player1) {
	sf::Packet packet;
	
	bool isValid = false;
	while (!isValid) {
		while (window.pollEvent(evento)) {
			sf::Vector2i relativeMousePosition = sf::Vector2i(int(mouseEvent.getPosition(window).x / CELL_SIZE) * CELL_SIZE,
															  int(mouseEvent.getPosition(window).y / CELL_SIZE) * CELL_SIZE);
			//Check if mouse is inside the grid
			if (mouseEvent.getPosition(window).x > 640 && relativeMousePosition.x < CELL_SIZE * 20 &&
				mouseEvent.getPosition(window).y > 0 && relativeMousePosition.y < CELL_SIZE * 10 ||
				mouseEvent.getPosition(window).x > 640 && relativeMousePosition.x < CELL_SIZE * 20 &&
				mouseEvent.getPosition(window).y > 0 && relativeMousePosition.y < CELL_SIZE * 10) {
				//When the player clicks, save the coords
				if (evento.type == sf::Event::MouseButtonReleased && evento.mouseButton.button == sf::Mouse::Left) {
					player1.shotCoords.x = (int(mouseEvent.getPosition(window).x / CELL_SIZE) - 10);
					player1.shotCoords.y = (int(mouseEvent.getPosition(window).y / CELL_SIZE));
					//Verify if cell is valid
					for (int i = 0; i < player1.coordRegister.size(); i++) {
						if (player1.coordRegister[i] == player1.shotCoords) {
							isValid = false;
							break;
						}
						else isValid = true;
					}
				}
			}
		}
	}
	//When cell click is valid, send the position to server
	if (isValid) {
		player1.coordRegister.push_back(player1.shotCoords);
		player1.hasTurn = false;
		packet << player1.shotCoords.x << player1.shotCoords.y;
		socket.send(packet);
		while (statusReceive == sf::Socket::Partial) { //Handle partial send
			socket.send(packet);	
			statusReceive = socket.receive(packet);
		}
	}
}


int main()
{
	//Init tcp variables
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	sf::TcpSocket* tcpSocket = new sf::TcpSocket;
	sf::Socket::Status statusReceive;
	tcpSocket->setBlocking(false);
	sf::Packet packet;
	//Bool to control flow of the game
	bool first = true;
	bool gameReady = false;
	bool matchLobbyShowed = false;
	//Init impact dots vector
	std::vector<sf::CircleShape> impactsDot;
	sf::String message;
	
	//Init Textures
	sf::Texture shipTexture;
	shipTexture.loadFromFile("./../Resources/Images/Spaceships.png");

	sf::Texture blue_grid;
	blue_grid.loadFromFile("./../Resources/Images/blue_grid.png");
	Grid grid1(sf::Vector2i(0, 0), blue_grid);

	sf::Texture red_grid;
	red_grid.loadFromFile("./../Resources/Images/red_grid.png");
	Grid grid2(sf::Vector2i(640, 0), red_grid);

	//Init font and text...
	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}
	sf::Text messageText("", font, 45);
	messageText.setStyle(sf::Text::Bold);
	messageText.setPosition(300, 700);

	//Init player
	int stateLector = 0;
	PlayerInfo player1("", ISA, grid1);
	std::cout << "Please, introduce your name: ";
	std::cin >> player1.name;
	std::cout <<  "type '1' to Search or '2' to Host: ";
	std::cin >> stateLector;
	for(;;) {
		if (stateLector == PlayerInfo::STATE::Search || stateLector == PlayerInfo::STATE::Host) break;
		std::cout << "Enter a valid option: ";
		std::cin >> stateLector;
	}
	std::vector<LobbyMatch*> matchList;

	player1.currentState = (PlayerInfo::STATE)stateLector;
	//Fake coordinate
	player1.coordRegister.push_back(sf::Vector2i(-5, -5));
	//Connect with the server
	packet << player1.name << player1.currentState;
	if (player1.currentState == PlayerInfo::STATE::Host) {
		std::string matchName;
		int turns;
		int maxTime;
		int maxFleetSize;
		std::cout << "----------MATCH SETTINGS--------------" << std::endl;
		std::cout << "Introduce name of the match: ";
		std::cin >> matchName;
		std::cout << "Introduce number of turns: ";
		std::cin >> turns;
		std::cout << "Introduce match time: ";
		std::cin >> maxTime;
		std::cout << "Introduce fleet size: ";
		std::cin >> maxFleetSize;
		packet << matchName << turns << maxTime << maxFleetSize;
	}
	tcpSocket->connect(ip, 5000);
	tcpSocket->send(packet);

	//Init Windows
	sf::Vector2i screenDimensions(1280, 840);
	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Ultimate Galactic BattleSpaceShip II - Revenge of the Mecha-Putin");

	//Init events
	sf::Event evento;
	sf::Mouse mouseEvent;

	//Start GameLoop
	while (true) {

		packet.clear();
		statusReceive = tcpSocket->receive(packet);

		//When the player is not ready, receive attributes and init spaceship fleet...
		if (!player1.isReady) {
			if (statusReceive == sf::Socket::Done) {
				if (player1.currentState == PlayerInfo::STATE::Search) {
					int numMatch = 0;
					packet >> numMatch;
					for (int i = 0; i < numMatch; i++) {
						int id = 0;
						std::string matchName = "";
						int numPlayers = 0;
						packet >> id >> matchName >> numPlayers;
						matchList.push_back(new LobbyMatch(id, matchName, numPlayers));
					}
				}
				//packet >> player1.faction >> player1.hasTurn;
				//player1.fleet.ChangeFaction((Faction)player1.faction);
			}
			//player1.fleet.PlaceFleet(window, evento, mouseEvent, player1.isReady);
		}
		//When is ready, send the all the grid information to the server
		else if (first) {
			packet.clear();
			for (int i = 0; i < MAX_CELLS; i++) {
				for (int j = 0; j < MAX_CELLS; j++) packet << grid1.GetCell(sf::Vector2i(j, i));
			}
			tcpSocket->send(packet);
			while (statusReceive == sf::Socket::Partial) { //Handle partial send
				tcpSocket->send(packet);
				statusReceive = tcpSocket->receive(packet);
			}
			packet.clear();
			first = false;
		}
		//When the grid has been sent...
		else {
			//When we receive something from the server...
			 if (statusReceive == sf::Socket::Done) {
				 if (!gameReady) {
					 packet >> gameReady; //When the two players are ready
				 }
				 else {
					messageText.setPosition(350, 700);
					message = "";
					//Copy the packet to variables
					packet >> player1.hasTurn >> player1.isImpact >> player1.shotCoords.x >> player1.shotCoords.y >> message;

					//When the other player left the game
					if (message.getSize() > 0 && message == "Disconnection") {
						statusReceive = sf::Socket::Disconnected;
						message = "The enemy left the game, disconnecting";
						messageText.setString(message);
						messageText.setPosition(150, 700);
					}
					else if (statusReceive != sf::Socket::Disconnected) {
						//When the shot is an impact
						if (player1.isImpact) {
							//If player have turn, means that you have hit
							 if (player1.hasTurn) {
								 //Add a red dot to the grid position
								 sf::CircleShape dot(16, 60);
								 dot.setFillColor(sf::Color(159, 93, 100));
								 dot.setPosition(sf::Vector2f((player1.shotCoords.x + 10)*CELL_SIZE + 16, player1.shotCoords.y*CELL_SIZE + 16));
								 impactsDot.push_back(dot);
								 //Check and set message
								 if (message.getSize() > 0) {
									 if (message == "GameOver") {
										 message = "CONGRATULATIONS COMMANDER, YOU WIN!";
										 statusReceive = sf::Socket::Disconnected;
									 }
									 messageText.setPosition(150, 700);
									 messageText.setString(message);
								 }
								 else {
									 messageText.setString("GOOD SHOT SOLDIER !");
								 }
								 messageText.setFillColor(sf::Color(0, 150, 200));
							}
							 //If you don't have turn, means that you have been hit
							 else {
								 //Add a red dot to the grid position
								 sf::CircleShape dot(16, 60);
								 dot.setFillColor(sf::Color(159, 93, 100));
								 dot.setPosition(sf::Vector2f((player1.shotCoords.x)*CELL_SIZE + 16, player1.shotCoords.y*CELL_SIZE + 16));
								 impactsDot.push_back(dot);
								 //Check and set message
								 if (message.getSize() > 0) {
									 if (message == "GameOver") {
										 message = "WITHDRAW COMMANDER, YOU LOSE!";
										 statusReceive = sf::Socket::Disconnected;
									 }
									 messageText.setPosition(150, 700);
									 messageText.setString(message);
								 }
								 else {
									 messageText.setString("ATTENTION, IMPACT !");

								 }
								 messageText.setFillColor(sf::Color(159, 93, 100));
							 }

						 }
						//When the shot is not an impact
						 else {
							 //If you have turn, means that the opponent have missed the shot
							 if (player1.hasTurn) {
								 //Add a blue dot to the grid position
								 sf::CircleShape dot(16, 60);
								 dot.setFillColor(sf::Color(3, 142, 165));
								 dot.setPosition(sf::Vector2f((player1.shotCoords.x)*CELL_SIZE + 16, player1.shotCoords.y*CELL_SIZE + 16));
								 impactsDot.push_back(dot);
								 //Change message to draw
								 messageText.setString("ENEMY MISSED THE SHOT !");
								 messageText.setFillColor(sf::Color(159, 93, 100));

							 }
							 //If not, you have missed the shot
							 else {
								 //Add a blue dot to the grid position
								 sf::CircleShape dot(16, 60);
								 dot.setFillColor(sf::Color(3, 142, 165));
								 dot.setPosition(sf::Vector2f((player1.shotCoords.x + 10)*CELL_SIZE + 16, player1.shotCoords.y*CELL_SIZE + 16));
								 impactsDot.push_back(dot);
								 //Change message to draw
								 messageText.setString("YOU MISSED THE SHOT !");
								 messageText.setFillColor(sf::Color(0, 150, 200));
							 }
						 }
					 }
				 }
			}
			//When we receive nothing from the server...
			else if (statusReceive == sf::Socket::NotReady) {
				window.pollEvent(evento);
				//If player have turn, send shot information...
				if (player1.hasTurn && gameReady) { 
					SendFunction(*tcpSocket, window, evento, mouseEvent, statusReceive, player1);
				}
			}			
		}
		if (player1.currentState == PlayerInfo::STATE::Search && !matchLobbyShowed) {
			for (int i = 0; i < matchList.size(); i++) {
				std::cout << matchList[i]->id << "  -  " << matchList[i]->name << "  " << matchList[i]->numPlayers << "/2" << std::endl;
			}
			matchLobbyShowed = true;
		}
		//Draw window sprites and text
		grid1.Render(window);
		grid2.Render(window);
		player1.fleet.Render(window);
		for (int i = 0; i < impactsDot.size(); i++) window.draw(impactsDot[i]); //Impact dots
		window.draw(messageText);
		window.display();
		window.clear();

		//When disconnection, exit game loop
		if (statusReceive == sf::Socket::Disconnected) {
				std::cout << "Disconnection" << std::endl;
				sf::sleep(sf::milliseconds(5000));
				break;
		}
	}
	//Disconnect and delete the socket
	tcpSocket->disconnect();
	delete tcpSocket;
	return 0;
}
