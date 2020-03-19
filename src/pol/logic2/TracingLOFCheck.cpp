#include "logicdefs.h"

#include "Entity.h"
#include "Graphic.h"
#include "AIUtils.h"
#include "DirtyLinks.h"
#include "TraceUtils.h"
#include "EnemyDetection.h"
#include "TracingLOFCheck.h"
#include <undercover/gamelevel/GameLevel.h>

#include <boost/format.hpp>

//=====================================================================================//
//                         TracingLOFCheck::TracingLOFCheck()                          //
//=====================================================================================//
TracingLOFCheck::TracingLOFCheck(const ActivityFactory::shoot_info_s &info, bool fprint)
:	m_fprint(fprint),
	m_info(info)
{
}

//=====================================================================================//
//                           bool TracingLOFCheck::IsExist()                           //
//=====================================================================================//
bool TracingLOFCheck::IsExist(BaseEntity* entity, const point3& from, const point3& to)
{ 
    //нет существа - нет проверки
    if(entity == 0) return false;
    
    LOSTracer los(mdGhost|mdDynObjectTransparent);

    los.SetFirst(entity->GetEID(), from);

	if(m_info.IsObject())
	{
		los.SetSecond(m_info.GetObject(),to);
	}
    else
	{
        los.SetSecond(m_info.GetEntityId(), to);
	}

    los.CalcLOS();

	bool safe = true;
    if(los.HaveLOS() && entity->GetPlayer() != PT_PLAYER)
	{
		point3 pnt = from;
		point3 dir = Normalize(to - from);

		point3 pos;
		if(eid_t who = EntityPool::GetInst()->TraceEntities(pnt,dir,entity->GetEID(),&pos))
		{
			if(!EnemyDetector::getInst()->isHeEnemy4Me(entity,EntityPool::GetInst()->Get(who)))
			{
				safe = false;
			}
		}
	}

	bool havelos = los.HaveLOS() && safe;

    //вывод сообщения
    if(!havelos && m_fprint)
	{
		PrintMessage(entity);
    }

	return havelos;
}

//=====================================================================================//
//                        void TracingLOFCheck::PrintMessage()                         //
//=====================================================================================//
void TracingLOFCheck::PrintMessage(BaseEntity *entity)
{
    shot_type shot = SHT_AIMSHOT;
    if(HumanEntity* human = entity->Cast2Human()) shot = human->GetEntityContext()->GetShotType();
    
    entity->Notify(EntityObserver::EV_NO_LOF);
	std::string msg;
	try
	{
		msg = (boost::format(DirtyLinks::GetStrRes("no_lof").c_str()) % entity->GetInfo()->GetName().c_str()/* % AIUtils::Shot2Str(shot)*/).str();
	}
	catch(boost::io::format_error &)
	{
		msg = DirtyLinks::GetStrRes("no_lof");
	}

    DirtyLinks::Print(msg);
}
