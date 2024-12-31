#pragma once


class User : public Player
{
public:
	User();
	~User();
	int selected_player_id = 0;
	int selected_npch_id = 0;

};

