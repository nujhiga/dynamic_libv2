#include "stdafx.h"
#include "Npc.h"

Npc::Npc(const std::vector<std::string>& data, bool isPet) :
	isUserPet(isPet),
	Entity(stoi(data[3]), stoi(data[4]), stoi(data[5]), stoi(data[0]), stoi(data[1]), stoi(data[2]))
{
}
