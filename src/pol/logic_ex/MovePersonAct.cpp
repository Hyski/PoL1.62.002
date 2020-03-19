#include "precomp.h"
#include "MovePersonAct.h"

namespace PoL
{

namespace MovePersonActDetails
{

}

using namespace MovePersonActDetails;

struct PersonState
{
	GraphHuman::pose_type m_pose;
	GraphHuman::move_type m_move;
	GraphHuman::stance_type m_stance;
};

static PersonState g_states[] = 
{
	{GraphHuman::PT_STAND, GraphHuman::MT_WALK, GraphHuman::ST_USUAL},
	{GraphHuman::PT_STAND, GraphHuman::MT_RUN, GraphHuman::ST_USUAL},
	{GraphHuman::PT_SIT, GraphHuman::MT_WALK, GraphHuman::ST_USUAL}
};


//=====================================================================================//
//                           MovePersonAct::MovePersonAct()                            //
//=====================================================================================//
MovePersonAct::MovePersonAct(const std::string &name, const ipnt2_t &where, MoveType mt)
:	m_name(name),
	m_where(where),
	m_moveType(mt),
	m_useZone(false),
	m_entity(0),
	m_vehentity(0),
	m_started(0),
	m_ended(0)
{
}

//=====================================================================================//
//                           MovePersonAct::MovePersonAct()                            //
//=====================================================================================//
MovePersonAct::MovePersonAct(const std::string &name, const std::string &where, MoveType mt)
:	m_name(name),
	m_zone(where),
	m_moveType(mt),
	m_useZone(true),
	m_entity(0),
	m_vehentity(0),
	m_started(0),
	m_ended(0)
{
}

//=====================================================================================//
//                             void MovePersonAct::start()                             //
//=====================================================================================//
void MovePersonAct::start()
{
	assert( !isStarted() );

	BaseEntity *ent = getEntityByName(m_name);
	m_started = true;

	if(!ent)
	{
		ACT_LOG("WARNING: MoveAct: entity [" << m_name << "] not found\n");

		abnormalEnd();
		return;
	}

	m_entity = ent->Cast2Human();
	m_vehentity = ent->Cast2Vehicle();

	if(m_useZone && !getZoneCoord(&m_where))
	{
		ACT_LOG("WARNING: MoveAct: entity [" << m_name << "] zone [" << m_zone << "] not found\n");

		abnormalEnd();
		return;
	}

	if(HexGrid::GetInst()->IsOutOfRange(m_where))
	{
		ACT_LOG("WARNING: MoveAct: entity [" << m_name << "] target point is out of grid\n");

		abnormalEnd();
		return;
	}

	if((m_entity && m_entity->GetGraph()->GetPos2() == m_where)
		|| (m_vehentity && m_vehentity->GetGraph()->GetPos2() == m_where))
	{
		ACT_LOG("WARNING: MoveAct: entity [" << m_name << "] already in its target place\n");

		abnormalEnd();
		return;
	}

	// просчитаем путь
	pnt_vec_t path;

	PathUtils::CalcPassField(ent);
	PathUtils::CalcPath(ent, m_where, path);

	if(path.empty())
	{
		ACT_LOG("WARNING: MoveAct: entity [" << m_name << "] path not found, thus cannot move\n");

		abnormalEnd();
		return;
	}

	if(m_entity)
	{
		//	измяняем тип ходьбы
		m_oldMoveType = m_entity->GetGraph()->Cast2Human()->GetMoveType();
		m_oldPose = m_entity->GetGraph()->Cast2Human()->GetPose();
		m_oldStance = m_entity->GetGraph()->Cast2Human()->GetStance();
		m_entity->GetGraph()->Cast2Human()->ChangeMoveType(g_states[m_moveType].m_move);
		m_entity->GetGraph()->Cast2Human()->ChangePose(g_states[m_moveType].m_pose);
		m_entity->GetGraph()->Cast2Human()->ChangeStance(g_states[m_moveType].m_stance);
	}

	ent->Attach(this,EV_PREPARE_DEATH);

	//	создаем активность
	m_activity.reset(ActivityFactory::CreateMove(ent,path,ActivityFactory::CT_PLAYER_MOVE_ACT));
}

//=====================================================================================//
//                         bool MovePersonAct::getZoneCoord()                          //
//=====================================================================================//
bool MovePersonAct::getZoneCoord(::ipnt2_t *result)
{
	::HexGrid *grid = ::HexGrid::GetInst();

	HexGrid::hg_slice hs = grid->GetSlice();    
	typedef HexGrid::const_cell_iterator Itor_t;
	
	for(Itor_t citor = grid->first_cell(); citor != grid->last_cell(); ++citor)
	{
		HexGrid::const_prop_iterator hex_prop;
		hex_prop = grid->first_prop() + citor->GetProperty();

		if(hex_prop->m_zone == m_zone)
		{
			*result = hs.off2pnt(citor - grid->first_cell());
			return true;
		}
	}

	return false;
}

//=====================================================================================//
//                            void MovePersonAct::update()                             //
//=====================================================================================//
void MovePersonAct::update(float tau)
{
	assert( isStarted() );

	if(!m_activity.get())
	{
		m_ended = true;
	}
	else
	{
		if(!m_activity->Run(AC_TICK))
		{
			if(m_entity)
			{
				//	измяняем тип ходьбы
				m_entity->GetGraph()->Cast2Human()->ChangeMoveType(m_oldMoveType);
				m_entity->GetGraph()->Cast2Human()->ChangePose(m_oldPose);
				m_entity->GetGraph()->Cast2Human()->ChangeStance(m_oldStance);

				::PathUtils::CalcPassField(m_entity);
			}
			else if(m_vehentity)
			{
				::PathUtils::CalcPassField(m_vehentity);
			}

			if(m_entity) m_entity->Detach(this);
			if(m_vehentity) m_vehentity->Detach(this);
			m_entity = 0;
			m_vehentity = 0;
			m_activity.reset();
			m_ended = true;
		}
	}
}

//=====================================================================================//
//                          void MovePersonAct::abnormalEnd()                          //
//=====================================================================================//
void MovePersonAct::abnormalEnd()
{
	m_entity = 0;
	m_vehentity = 0;
	m_activity.reset();
}

//=====================================================================================//
//                            void MovePersonAct::Update()                             //
//=====================================================================================//
void MovePersonAct::Update(BaseEntity *, event_t, info_t)
{
	if(m_entity) m_entity->Detach(this);
	if(m_vehentity) m_vehentity->Detach(this);
	m_entity = 0;
	m_vehentity = 0;
}

}