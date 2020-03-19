#include "logicdefs.h"

#include "Entity.h"
#include "PathUtils.h"
#include "MultiMoveUtils.h"

namespace PoL
{

//=====================================================================================//
//                       void MultiMoveUtils::prepareMultiMove()                       //
//=====================================================================================//
void MultiMoveUtils::prepareMultiMove(std::vector<BaseEntity *> &ents,
									  const ipnt2_t &to,
									  std::vector<pnt_vec_t> &paths)
{
	EntityPool *ep = EntityPool::GetInst();
	for(EntityPool::iterator ent = ep->begin(ET_HUMAN,PT_PLAYER); ent != ep->end(); ++ent)
	{
		if(HumanEntity *human = ent->Cast2Human())
		{
			if(!human->GetEntityContext()->GetCrew())
			{
				ents.push_back(&*ent);
				PathUtils::UnlinkEntity(ents.back());
			}
		}
	}

	paths.resize(ents.size());

	pnt_vec_t targets;
	int rad = 0;

	do
	{
		targets.clear();
		PathUtils::CalcPassField(to,rad,&targets);
		++rad;
	}
	while(targets.size() < ents.size());

	for(int i = 0; i != ents.size(); ++i)
	{
		PathUtils::CalcPassField(ents[i]);
		PathUtils::CalcPath(ents[i],targets[i],paths[i]);
	}

	for(int i = 0; i != ents.size(); ++i)
	{
		PathUtils::LinkEntity(ents[i]);
	}
}

}
