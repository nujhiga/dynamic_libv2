#pragma once
#include "Entity.h"

class User : public Entity
{
public:
	User();
	~User();
	
	std::string name;

	bool paralized = false;
	bool meditando = false;
	int bcr = 0;
	int selectedSpell = 0;

	Entity* selectedEntity = nullptr;

	std::unordered_map<std::string, int> spells;
};

