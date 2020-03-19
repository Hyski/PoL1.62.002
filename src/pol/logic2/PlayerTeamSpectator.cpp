#include "logicdefs.h"

#include "VisMap.h"
#include "Marker.h"
#include "Relation.h"
#include "PlayerTeamSpectator.h"

namespace PlayerTeamSpectatorDetails
{

//=====================================================================================//
//                      class PlayerMSRelation : public Relation                       //
//=====================================================================================//
class PlayerMSRelation : public Relation
{
public:
	PlayerMSRelation(Spectator *s, Marker *m) : Relation(m,s) {}

	VisParams DoUpdateRelation(VisUpdateEvent_t ev/*Marker* marker, Spectator* spectator, int event*/) const
	{
		switch(ev)
		{
			case UE_DEL_MARKER:
				return VisParams::vpInvisible;
			case UE_MARKER_ON:
			case UE_POS_CHANGE:
			case UE_FOS_CHANGE:
			case UE_INS_MARKER:
			case UE_INS_SPECTATOR:
				return GetSpectator()->CalcVisibility(GetMarker());
		}
		return GetSpectator()->CalcVisibility(GetMarker());
	}
};

}

using namespace PlayerTeamSpectatorDetails;

DCTOR_IMP(PlayerTeamSpectator)

//=====================================================================================//
//                     PlayerTeamSpectator::PlayerTeamSpectator()                      //
//=====================================================================================//
PlayerTeamSpectator::PlayerTeamSpectator()
:	m_turnOn(true)
{
	GameEvMessenger::GetInst()->Attach(this, EV_TURN);
}

//=====================================================================================//
//                     PlayerTeamSpectator::~PlayerTeamSpectator()                     //
//=====================================================================================//
PlayerTeamSpectator::~PlayerTeamSpectator()
{ 
	DetachObserver();
	GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                      void PlayerTeamSpectator::MakeSaveLoad()                       //
//=====================================================================================//
void PlayerTeamSpectator::MakeSaveLoad(SavSlot& slot)
{
	if(slot.IsSaving())
	{
	//	slot << m_flags;

	//	//сохраним список всех кого видим
	//	VisMap::MarkIterator_t itor = VisMap::GetInst()->MarkBegin(this, VisParams::vpVisible);
	//	for(; itor != VisMap::GetInst()->MarkEnd(); ++itor)
	//	{
	//		if(itor->GetEntity()) slot << itor->GetEntity()->GetEID();
	//	}         

	//	slot << static_cast<eid_t>(0);
	}
	else
	{
		if(slot.GetStore()->GetVersion() < 6)
		{
			unsigned int flags;
			slot >> flags;

			//прочтем список видимых
			eid_t eid;
			slot >> eid;

			while(eid)
			{
				//m_eids.insert(eid);
				EntityPool::GetInst()->Get(eid)->GetEntityContext()->GetMarker()->ForceVisible(true,PT_PLAYER1);
				slot >> eid;
			}
		}

	//	//пошлем сообщение об изменении моля видимости
	//	VisMap::GetInst()->UpdateSpectator(this/*, VisMap::UE_FOS_CHANGE*/);

	//	//сбросим список видимых
	//	m_eids.clear();
	}
}

//=====================================================================================//
//                   VisParams PlayerTeamSpectator::CalcVisibility()                   //
//=====================================================================================//
VisParams PlayerTeamSpectator::CalcVisibility(Marker* marker)
{
	const player_type CurPlayer = PT_PLAYER;
	BaseEntity* mrk_ent = marker->GetEntity();

	if(!mrk_ent || mrk_ent->GetPlayer() == CurPlayer)
	{
		return VisParams::vpInvisible;
	}

	if(marker->IsVisibleForPlayer(CurPlayer))
	{
		if(m_turnOn)
		{
			return VisParams::vpVisible;
		}
		else
		{
			marker->ResetVisibility(CurPlayer);
			return VisParams::vpInvisible;
		}
	}

	////если попадает в список видимых
	//if(mrk_ent && m_eids.count(mrk_ent->GetEID()))
	//{
	//	return VisParams::vpVisible;
	//}

	//if(m_flags & F_TURN_ON && marker->IsHighlighted())
	//{
	//	return VisParams::vpVisible;
	//}

	//VisMap::SpecIterator_t itor = VisMap::GetInst()->SpecBegin(marker, VisParams::vpVisible);

	//for(; itor != VisMap::GetInst()->SpecEnd(); ++itor)
	//{
	//	if(this != &*itor && itor->GetPlayer() == CurPlayer)
	//	{
	//		return VisParams::vpVisible;
	//	}
	//}

	return VisParams::vpInvisible;
}

//=====================================================================================//
//                   VisParams PlayerTeamSpectator::CalcVisibility()                   //
//=====================================================================================//
VisParams PlayerTeamSpectator::CalcVisibility(const point3& pnt)
{
	return VisParams::vpInvisible;
}

//=====================================================================================//
//          std::auto_ptr<Relation> PlayerTeamSpectator::CreateRelationFor()           //
//=====================================================================================//
std::auto_ptr<Relation> PlayerTeamSpectator::CreateRelationFor(Marker* marker)
{
	if(BaseEntity* entity = marker->GetEntity())
	{
		return std::auto_ptr<Relation>(new PlayerMSRelation(this,marker));
	}

	return std::auto_ptr<Relation>();
}

//=====================================================================================//
//                         void PlayerTeamSpectator::Update()                          //
//=====================================================================================//
void PlayerTeamSpectator::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
	VisMap::GetInst()->UpdateSpectator(this, UE_FOS_CHANGE);
}

//=====================================================================================//
//                         void PlayerTeamSpectator::Update()                          //
//=====================================================================================//
void PlayerTeamSpectator::Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
{
	turn_info* ptr = static_cast<turn_info*>(info);

	//нас итересует только изменеия хода игрока
	if(ptr->m_player != PT_PLAYER)
		return;

	if(ptr->IsBegining())
	{
		//if(m_flags == 0) VisMap::GetInst()->UpdateSpectator(this/*, VisMap::UE_FOS_CHANGE*/);

		m_turnOn = true;
		VisMap::GetInst()->UpdateSpectator(this, UE_FOS_CHANGE);

		//m_flags &= ~F_TURN_OFF;
		//m_flags |=  F_TURN_ON;

		DetachObserver();
	}

	if(ptr->IsEnd())
	{
		m_turnOn = false;

		VisMap::GetInst()->UpdateSpectator(this, UE_FOS_CHANGE);
		VisMap::GetInst()->ResetVisibility(PT_PLAYER1);

		//m_flags &= ~F_TURN_ON;
		//m_flags |=  F_TURN_OFF;

		AttachObserver();

	}            
}

//=====================================================================================//
//                     void PlayerTeamSpectator::AttachObserver()                      //
//=====================================================================================//
void PlayerTeamSpectator::AttachObserver()
{
	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_PLAYER1);
	for(; itor != EntityPool::GetInst()->end(); ++itor)
	{
		itor->Attach(this, EV_FOS_CHANGE);
		itor->Attach(this, EV_DEATH_PLAYED);
	}
}

//=====================================================================================//
//                     void PlayerTeamSpectator::DetachObserver()                      //
//=====================================================================================//
void PlayerTeamSpectator::DetachObserver()
{
	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_PLAYER1);
	for(; itor != EntityPool::GetInst()->end(); ++itor)
	{
		itor->Detach(this);
	}
}
