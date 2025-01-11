#include "stdafx.h"
#include "Player.h"

Player::Player(const std::vector<std::string>& data, int inviDetected) :
	wanim(stoi(data[6])), sanim(stoi(data[7])), fx(stoi(data[8])),
	canim(stoi(data[10])), name(data[11]), bcr(stoi(data[12])),
	invi(stoi(data[13])), inviDetected(inviDetected),
	Entity(stoi(data[3]), stoi(data[4]), stoi(data[5]), stoi(data[0]), stoi(data[1]), stoi(data[2]))
{
	isDead = false;
}
