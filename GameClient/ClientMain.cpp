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

struct MatchText {
	int matchId;
	sf::Text idText;
	sf::Text matchNameText;
	sf::Text numPlayersText;
	MatchText(int id, std::string matchName, int numPlayers, sf::Font &font, int separator) :idText("", font, 25), matchNameText("", font, 25), numPlayersText("", font,25){
		matchId = id;

		idText.setString("ID:" + std::to_string(id) + " - ");
		idText.setFillColor(sf::Color(0, 160, 0));
		idText.setPosition(10, separator * 50);

		matchNameText.setString(matchName + " - ");
		matchNameText.setFillColor(sf::Color(0, 160, 0));
		matchNameText.setPosition(idText.getGlobalBounds().width + 10, separator * 50);

		numPlayersText.setString(std::to_string(numPlayers) + "/2");
		numPlayersText.setFillColor(sf::Color(0, 160, 0));
		numPlayersText.setPosition(matchNameText.getGlobalBounds().width + 135, separator * 50);
	};
	void MatchText::Draw(sf::RenderWindow &window) {
		window.draw(idText);
		window.draw(matchNameText);
		window.draw(numPlayersText);
	}
};
struct HostText {
	sf::Text banner;
	sf::Text nameMatch;
	sf::Text turns;
	sf::Text maxTime;
	sf::Text maxFleetSize;

	HostText(sf::Font &font): banner("", font, 25),nameMatch("", font, 25 ), turns("", font, 25), maxTime("", font, 25), maxFleetSize("", font, 25){
		banner.setString("---Welcome to Host Customization---");
		banner.setFillColor(sf::Color(0, 160, 160));
		banner.setPosition(0,0);

		nameMatch.setString("Introduce name of the match: ");
		nameMatch.setFillColor(sf::Color(0, 160, 0));
		nameMatch.setPosition(0, 50);

		turns.setString("Introduce number of turns: ");
		turns.setFillColor(sf::Color(0, 160, 0));
		turns.setPosition(0, 100);

		maxTime.setString("Introduce match time: ");
		maxTime.setFillColor(sf::Color(0, 160, 0));
		maxTime.setPosition(0, 150);

		maxFleetSize.setString("Introduce fleet size: ");
		maxFleetSize.setFillColor(sf::Color(0, 160, 0));
		maxFleetSize.setPosition(0, 200);
	}
	void HostText::Draw(sf::RenderWindow &window) {
		window.draw(banner);
		window.draw(nameMatch);
		window.draw(turns);
		window.draw(maxTime);
		window.draw(maxFleetSize);
	}
};

void SendLobbyFunction(sf::RenderWindow &window, sf::Event& evento, sf::String &mensaje, PlayerInfo &player, std::vector<sf::Text> &inputList, sf::Font &font, HostText hostText, int &currentHostInput, bool &hostSetupDone) {

	std::string text;

	while (window.pollEvent(evento)) {
		switch (evento.type)
		{
		case sf::Event::Closed:
			window.close();
			break;
		case sf::Event::KeyPressed:
			if (evento.key.code == sf::Keyboard::Escape) {
				window.close();
				break;
			}
			else if (evento.key.code == sf::Keyboard::Return && mensaje.getSize() > 0) {
				if (player.currentState == PlayerInfo::STATE::Host && !hostSetupDone) {
					sf::Text tempInput(mensaje, font, 25);
					tempInput.setFillColor(sf::Color(0,160,160));
					tempInput.setPosition(450, 50*currentHostInput);
					inputList.push_back(tempInput);
					mensaje.clear();
					currentHostInput++;
					if (currentHostInput >= 5) hostSetupDone = true;
				}
			}
			break;
		case sf::Event::TextEntered:
			if (!hostSetupDone) {
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}
	}
}

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
	bool waitingMatchConfirmation = false;
	bool searchingPartner = false;
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

	//Host Text
	HostText hostText(font);
	std::vector <sf::Text> inputTextList;
	int currentHostInput = 1;
	bool hostSetupDone = false;
	bool setupSended = false;

	//Search Text
	std::vector<MatchText*> matchTextList;
	sf::Text inputMessage("", font, 25);
	inputMessage.setFillColor(sf::Color(0, 160, 160));
	inputMessage.setStyle(sf::Text::Bold);
	sf::String mensaje;

	//Init player
	int stateLector = 0;
	PlayerInfo player1(0, "", ISA, grid1);
	std::cout << "Please, introduce your name: ";
	std::cin >> player1.name;
	std::cout << "type '1' to Search or '2' to Host: ";
	std::cin >> stateLector;
	for (;;) {
		if (stateLector == PlayerInfo::STATE::Search || stateLector == PlayerInfo::STATE::Host) break;
		std::cout << "Enter a valid option: ";
		std::cin >> stateLector;
	}
	std::vector<LobbyMatch*> matchList;

	player1.currentState = (PlayerInfo::STATE)stateLector;
	//Fake coordinate
	player1.coordRegister.push_back(sf::Vector2i(-5, -5));
	//Connect with the server
	if (player1.currentState == PlayerInfo::STATE::Search) {
		tcpSocket->connect(ip, 5001);
		packet << player1.name << player1.currentState;
		tcpSocket->send(packet);
	}
	else player1.isHost = true;


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
			if (!player1.matchReady) {
				if (statusReceive == sf::Socket::Done) {
					if (waitingMatchConfirmation) {
						bool matchConfirmation = false;
						packet >> player1.id  >> matchConfirmation;
						if (matchConfirmation) {
							//GAMEEEEEEE!!!!!!!!!!!!
							player1.matchReady = true;
						}
						else {
							waitingMatchConfirmation = false;
						}
					}
					else if (searchingPartner) {
						bool partnerConfirmation = false;
						packet >> player1.id >> partnerConfirmation;
						if (partnerConfirmation) {
							//GAMEEEEE!!!!!!!!!!!
							player1.matchReady = true;
						}
						else searchingPartner = false;
					}
					else if (player1.currentState == PlayerInfo::STATE::Search) {
						int matchSize = 0;
						packet >> matchSize;
						for (int i = 0; i < matchSize; i++) {
							int id = 0;
							std::string matchName = "";
							int numPlayers = 0;
							int matchExists = false;
							packet >> id >> matchName >> numPlayers;
							for (int j = 0; j < matchList.size(); j++) {
								if (matchList[j]->id == id) {
									if (numPlayers == 0) { //Match finished or deleted
										matchList.erase(matchList.begin()+j);
									}
									matchList[j]->numPlayers = numPlayers;
									matchTextList[j]->numPlayersText.setString(std::to_string(numPlayers) + "/2");
									matchExists = true;
									break;
								}
							}
							if (!matchExists) {
								matchList.push_back(new LobbyMatch(id, matchName, numPlayers));
								matchTextList.push_back(new MatchText(id, matchName, numPlayers, font, matchList.size()));
							}
						}
					}
					//packet >> player1.faction >> player1.hasTurn;
					//player1.fleet.ChangeFaction((Faction)player1.faction);
				}


				SendLobbyFunction(window, evento, mensaje, player1, inputTextList, font, hostText, currentHostInput, hostSetupDone);

				if (player1.currentState == PlayerInfo::STATE::Host && hostSetupDone && !setupSended) {
					packet.clear();
					packet << player1.name << player1.currentState << std::string(inputTextList[0].getString()) << std::stoi(std::string(inputTextList[1].getString())) <<
						std::stoi(std::string(inputTextList[2].getString())) << std::stoi(std::string(inputTextList[3].getString()));
					tcpSocket->connect(ip, 5001);
					tcpSocket->send(packet);
					searchingPartner = true;
					setupSended = true;
				}
				else if (player1.currentState == PlayerInfo::STATE::Search && evento.key.code == sf::Keyboard::Return && mensaje.getSize() > 0) {
					packet.clear();
					packet << std::stoi(std::string(mensaje));
					tcpSocket->send(packet);
					waitingMatchConfirmation = true;
					mensaje.clear();
				}
			}
			else player1.fleet.PlaceFleet(window, evento, mouseEvent, player1.isReady);
		}
		//When is ready, send the all the grid information to the server
		else if (first) {
			packet.clear();
			packet << player1.id;
			for (int i = 0; i < MAX_CELLS; i++) {
				for (int j = 0; j < MAX_CELLS; j++) packet << grid1.GetCell(sf::Vector2i(j, i));
			}
			tcpSocket->send(packet);
			packet.clear();
			first = false;
		}
		//When the grid has been sent...
		else {
			//When we receive something from the server...
			 if (statusReceive == sf::Socket::Done) {
				 if(gameReady) {
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
				 else {
					 packet >> gameReady; //When the two players are ready
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

		//Draw window sprites and text

		if (player1.matchReady) {
			grid1.Render(window);
			grid2.Render(window);
			player1.fleet.Render(window);
			for (int i = 0; i < impactsDot.size(); i++) window.draw(impactsDot[i]); //Impact dots
			window.draw(messageText);
		}
		else{
			if (player1.currentState == PlayerInfo::STATE::Search) {
				sf::Text banner("---Welcome to Server Browser---", font,25);
				banner.setFillColor(sf::Color(0, 160, 160));
				banner.setPosition(0, 0);
				window.draw(banner);
				for (int i = 0; i < matchTextList.size(); i++) matchTextList[i]->Draw(window);
				std::string mensaje2 = mensaje + "_";
				inputMessage.setString("Introduce match ID to join: " + mensaje2);
				inputMessage.setPosition(10, (matchTextList.size()+1) * 50);
				window.draw(inputMessage);
			}
			else if (player1.currentState == PlayerInfo::STATE::Host) {
				hostText.Draw(window);
				for (int i = 0; i < inputTextList.size(); i++) window.draw(inputTextList[i]);
				std::string mensaje2 = mensaje + "_";
				inputMessage.setString(mensaje2);
				inputMessage.setPosition(450, currentHostInput * 50);
				if(!hostSetupDone)window.draw(inputMessage);
				else {
					sf::Text banner("---Searching Player, Please Wait---", font, 25);
					banner.setFillColor(sf::Color(0, 160, 160));
					banner.setPosition(0, currentHostInput*50);
					window.draw(banner);
				}
			}
		}
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
