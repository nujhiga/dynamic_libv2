#include "stdafx.h"
#include "oPlayer.h"

oPlayer::oPlayer()
{
}

oPlayer::oPlayer(const std::vector<std::string>& pinfo)
    : inf0(stoi(pinfo[0])), inf1(stoi(pinfo[1])), inf2(stoi(pinfo[2])),
    id(stoi(pinfo[3])), posX(stoi(pinfo[4])), posY(stoi(pinfo[5])),
    inf6(stoi(pinfo[6])), inf7(stoi(pinfo[7])), inf8(stoi(pinfo[8])),
    inf9(stoi(pinfo[9])), inf10(stoi(pinfo[10])),
    name(pinfo[11]), faction(stoi(pinfo[12])), isInvisible(stoi(pinfo[13]) == 1),
    inviDetected(false), inf14(stoi(pinfo[14])), inf15(stoi(pinfo[15])),
    orgName(pinfo[11]) {
}

oPlayer::~oPlayer()
{
	name.clear();
	orgName.clear();
}
