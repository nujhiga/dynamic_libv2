#pragma once
#include "Entity.h"

class Npc : public Entity {
public:
	Npc(const std::vector<std::string>& data, bool isPet);
	bool isUserPet;
};