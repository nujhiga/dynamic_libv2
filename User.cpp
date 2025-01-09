#include "stdafx.h"
#include "User.h"


User::User() : Entity(0,0,0,0,0,0)
{
}


User::~User()
{
	spells.clear();
}