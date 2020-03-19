#pragma once

namespace PoL
{

//=====================================================================================//
//                                    struct GameId                                    //
//=====================================================================================//
struct GameId
{
	long id[4];

	GameId();
	void generate();

	void save(SavSlot &);
	void load(SavSlot &);
};

}