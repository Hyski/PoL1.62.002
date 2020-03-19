#include "logicdefs.h"
#include "AIDefs.h"

const float AIDefs::angle_eps = TORAD(6);
const float AIDefs::alarm_dist = 5.0f;
const float AIDefs::signal_dist = 100.0f;
const float AIDefs::fixed_enemy_dist = 34.0f;
const float AIDefs::fixed_cure_dist = 0.0f;
const int AIDefs::fixed_turns = 3;
const float AIDefs::patrol_cure_dist = 0.0f;
const float AIDefs::patrol_enemy_dist = 22.0f;
const unsigned int AIDefs::patrol_enemy_noLOF_move = 6;
const float AIDefs::assault_cure_dist = 0.0f;
const float AIDefs::assault_enemy_dist = 22.0f;
const int AIDefs::assault_turns = 2;
const float AIDefs::fixed_tech_enemy_dist = 22.0f;
const float AIDefs::patrol_tech_enemy_dist = 22.0f;
const float AIDefs::assault_tech_enemy_dist = 22.0f;

//=====================================================================================//
//                             float AIDefs::norm_angle()                              //
//=====================================================================================//
float AIDefs::norm_angle(float a)
{
	const float result = fmodf(a, PIm2);
	return a < 0.0f ? result+PIm2 : result;
}

//=====================================================================================//
//                      void AIDefs::setDifferenceBetweenLists()                       //
//=====================================================================================//
void AIDefs::setDifferenceBetweenLists(const eidlist_t& lista,
										const eidlist_t& listb,
										eidlist_t* outlist)
{
	eidlist_t result;
	std::set_difference(lista.begin(),lista.end(),listb.begin(),listb.end(),std::inserter(result,result.end()));
	outlist->swap(result);
}
