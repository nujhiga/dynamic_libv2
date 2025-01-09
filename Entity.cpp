#include "stdafx.h";
#include "Entity.h";

Entity::Entity(int id, int posX, int posY, int body, int head, int heading) :
	id(id), posX(posX), posY(posY), body(body), head(head), heading(heading)
{
}

Entity::~Entity() {}

void Entity::updatePos(int _posX, int _posY) {
	posX = _posX;
	posY = _posY;
}