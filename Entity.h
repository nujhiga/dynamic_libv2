#pragma once

#include <vector>
#include <string>

class Entity {
public:
	Entity(int id, int posX, int posY, int body, int head, int heading);
	virtual ~Entity();

	int id;
	int posX;
	int posY;
	int body;
	int head;
	int heading;
	
	void updatePos(int posX, int posY);
};