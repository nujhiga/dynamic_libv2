#pragma once
#include "Entity.h"

class Player : public Entity {
public:
	Player(const std::vector<std::string>& data, int inviDetected);
	
	int wanim;
	int sanim;
	int fx;
	int canim;
	std::string name;
	int bcr;
	int invi;
	int inviDetected;

	const int unk1 = 999;
	const int unk2 = 0;
	const int unk3 = 10;
	bool isDead;
};

//0.body-
// 1.head-
// 2.heading-
// 3.id-
// 4.x-
// 5.y-
// 6.wanim-
// 7.sanim-
// 8.fx-
// 9.999-
// 10.canim-
// 11.name-
// 12.bcr-
// 13.invi-
// 14.unk1-
// 15.unk2