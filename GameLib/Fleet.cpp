#include "Fleet.h"

Fleet::Fleet(Faction _fleetFaction, std::string texturePath, Grid &_grid) : grid(_grid)
{
	shipTexture.loadFromFile(texturePath);
	shipCount = 0;
	//Init all ships of the fleet
	for (int i = 0; i < MAX_SHIPS; i++) {
		if (i == 1) shipType--;						 //in this case the boat becomes the medium
		else if (i == 3) shipType--;				 //and in this case the boat becomes the small
		Ship newShip(sf::Vector2i(0+100*i, 640), (ShipType)shipType, _fleetFaction, shipTexture);
		ships.push_back(newShip);
	}
}

//Place all ships to the grid
void Fleet::PlaceFleet(sf::RenderWindow &window, sf::Event &evento,sf::Mouse &mouseEvent, bool &isPlaced)
{
	while (window.pollEvent(evento)) {
		//Relative mouse position in the grid, to fit ships in the grid
		sf::Vector2i relativeMousePosition = sf::Vector2i(int(mouseEvent.getPosition(window).x / CELL_SIZE) * CELL_SIZE,
														  int(mouseEvent.getPosition(window).y / CELL_SIZE) * CELL_SIZE);

		if (shipCount < MAX_SHIPS) {
			//Check if the ship can be placed or rotated to next position (mouse position)
			for (int i = 0; i < ships[shipCount].GetType() + 2; i++) {
				//When ship is rotated
				if (ships[shipCount].GetRotation()) {
					if (grid.GetCell(sf::Vector2i(relativeMousePosition.x / CELL_SIZE + i, relativeMousePosition.y / CELL_SIZE)) != 0)
						canBePlaced = false;

					if (grid.GetCell(sf::Vector2i(relativeMousePosition.x / CELL_SIZE , relativeMousePosition.y / CELL_SIZE+i)) != 0)
						canBeRotated = false;
				}
				//When not
				else {
					if (grid.GetCell(sf::Vector2i(relativeMousePosition.x / CELL_SIZE, relativeMousePosition.y / CELL_SIZE + i)) != 0)
						canBePlaced = false;

					if (grid.GetCell(sf::Vector2i(relativeMousePosition.x / CELL_SIZE + i, relativeMousePosition.y / CELL_SIZE )) != 0)
						canBeRotated = false;
				}
			}
			//Amazing IF to verify if the ship is inside the grid and can be placed
			if (!ships[shipCount].GetRotation() && mouseEvent.getPosition(window).x > 0 && relativeMousePosition.x < CELL_SIZE*MAX_CELLS &&
				mouseEvent.getPosition(window).y > 0 && relativeMousePosition.y < CELL_SIZE*(MAX_CELLS-ships[shipCount].GetType()-1) && canBePlaced|| 
				ships[shipCount].GetRotation() && mouseEvent.getPosition(window).x > 0 && relativeMousePosition.x < CELL_SIZE*(MAX_CELLS - ships[shipCount].GetType() - 1) &&
				mouseEvent.getPosition(window).y > 0 && relativeMousePosition.y < CELL_SIZE*MAX_CELLS && canBePlaced) 
			{
				isInsideGrid = true;
				ships[shipCount].SetPosition(relativeMousePosition.x, relativeMousePosition.y); //Update ship position
			}
			else isInsideGrid = false;
			//When player rotate the ship with right button...
			if (canBeRotated && evento.type == sf::Event::MouseButtonReleased && evento.mouseButton.button == sf::Mouse::Right &&
				relativeMousePosition.x / CELL_SIZE + ships[shipCount].GetType() + 1 < 10 ) ships[shipCount].SetRotation();
			//When player wants to place the ship in the grid...
			if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left && canBePlaced) {
				ships[shipCount].SetPosition(relativeMousePosition.x, relativeMousePosition.y);
				ships[shipCount].Update();
				ships[shipCount].SetPlaced(true); //We place the ship to the position
				//Put the information to the grid
				for (int i = 0; i <= ships[shipCount].GetType()+1; i++) {
					if(ships[shipCount].GetRotation())
						grid.SetCell(sf::Vector2i(ships[shipCount].GetPosition().x / CELL_SIZE + i, ships[shipCount].GetPosition().y / CELL_SIZE), shipCount+1);
					else
						grid.SetCell(sf::Vector2i(ships[shipCount].GetPosition().x / CELL_SIZE, ships[shipCount].GetPosition().y / CELL_SIZE + i), shipCount+1);
				}
				shipCount++;
			}
			//Reset bools
			canBePlaced = true;
			canBeRotated = true;
		}
		else isPlaced = true;
	}
	if (shipCount < MAX_SHIPS) ships[shipCount].Update();
}

void Fleet::Render(sf::RenderWindow & window)
{
	for (int i = 0; i < ships.size(); i++) ships[i].Render(window);
}

void Fleet::ChangeFaction(Faction faction)
{
	for (int i = 0; i < MAX_SHIPS; i++) ships[i].ChangeSprite(faction);
}

Ship& Fleet::GetShip(int id)
{
	return ships[id-1];
}


Fleet::~Fleet()
{
}
