#ifndef __WORLD_H_
#define __WORLD_H_

#include "iworld.h"

class GraphPipe;

#define SINGLETON(Var,Class) (Var?Var:(Var=new Class))

class IExplosionAPI
{
public:
	enum damage_type{
		DT_NONE,
		DT_STRIKE,
		DT_SHOCK,
		DT_ENERGY,
		DT_ELECTRIC,
		DT_EXPLOSIVE,
		DT_FLAME,
	};
	struct Damage
	{
		damage_type Type;
		float				Power;
	};
public:
	virtual void DamageEntity(const point3 &Pos, const point3& dir, const Damage dam[3], unsigned Handle)=0;
};


#endif