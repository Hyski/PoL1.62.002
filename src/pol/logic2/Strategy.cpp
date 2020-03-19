//
// ���������� ������ ����������
//
#include "logicdefs.h"

#include "thing.h"
#include "entity.h"
#include "VisMap.h"
#include "player.h"
#include "Marker.h"
#include "damager.h"
#include "aiutils.h"
#include "HexGrid.h"
#include "graphic.h"
#include "sndutils.h"
#include "strategy.h"
#include "Relation.h"
#include "xlsreader.h"
#include "Spectator.h"
#include "entityaux.h"
#include "pathutils.h"
#include "DirtyLinks.h"
#include "traceutils.h"
#include "gameobserver.h"
#include "enemydetection.h"

#include <undercover/gamelevel/GameLevel.h>

//======================================================

namespace StrategyDetail
{
}

using namespace StrategyDetail;

namespace StrategyDetail
{

    const char* prostitute1 = "stripper";
    const char* prostitute2 = "high_stripper";

    const char* termoplasma_explosion = "hit_large_plasma_explosion";

    const float vehicle_height = 2.0f;
    const float sitting_human_height = 1.2f;
    const float standing_human_height = 1.8f;
    const float vehicle_back_fos_radius = 1.5f;

    const float shotgun_caliber         = 1.8f;
    const float energy_gun_caliber      = 1.1f;
    const float autocannon_caliber      = 2.0f;
    const float rocket_launcher_caliber = 3.1f;

    const char* panic_xls_name = "scripts/logic/panic.txt";

    static std::istringstream& init(std::istringstream& istr, const std::string& str)
    {
        istr.str(str);
        istr.clear();
        return istr;
    }

//=====================================================================================//
//                      class UsualMSRelation : public MSRelation                      //
//=====================================================================================//
class UsualMSRelation : public Relation
{
public:
	UsualMSRelation(Spectator *s, Marker *m) : Relation(m,s) {}

    VisParams DoUpdateRelation(VisUpdateEvent_t ev/*Marker* marker, Spectator* spectator, int event*/) const
    {
		switch(ev)
		{
			case UE_DEL_MARKER:
				{
					SpectatorObserver::marker_info info(GetMarker());
					GetSpectator()->NotifySpectatorObserver(SpectatorObserver::EV_INVISIBLE_MARKER, &info);
				}
			case UE_DEL_SPECTATOR:
				return VisParams::vpInvisible;
				break;

			case UE_POS_CHANGE:
			case UE_FOS_CHANGE:
			case UE_INS_MARKER:
			case UE_INS_SPECTATOR:
				return GetSpectator()->CalcVisibility(GetMarker());
				break;
		}
		return GetSpectator()->CalcVisibility(GetMarker());
    }
};

//=====================================================================================//
//           class EntitySpectator : public Spectator, public EntityObserver           //
//=====================================================================================//
// ������� ����������
class EntitySpectator : public Spectator, public EntityObserver
{
public:

    EntitySpectator(BaseEntity* entity = 0) : m_entity(entity) {}

    ~EntitySpectator() { m_entity->Detach(this); }

    void OnInsert()
    {
        AIUtils::GetEntityFOS(m_entity, &m_fos);
        m_entity->Attach(this, EV_FOS_CHANGE);
    }

    void OnRemove()
    {
        m_entity->Detach(this);
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){
            slot << m_entity->GetEID();
        }else{            
            eid_t eid; slot >> eid;
            m_entity = EntityPool::GetInst()->Get(eid);
        }
    }

	std::string DescribeSelf() const
	{
		return m_entity->GetInfo()->GetName();
	}

    //��������� ��������� ��� �����
    VisParams CalcVisibility(const point3& test)
    {
        pos_s ent_pos;

        //������ ������� ��������
        GetEntityPos(m_entity, &ent_pos);
		m_fos.setDirAngle(ent_pos.m_angle);

        //���� �������� � FOS
        if(     IsInFOS(test, m_fos, ent_pos)
            &&  HaveLOS(m_entity, ent_pos.m_pos, 0, test))
            return VisParams::vpVisible;
  
        return VisParams::vpInvisible;
    }

    //��������� ��������� ��� �������
    VisParams CalcVisibility(Marker* marker)
    {  
        pos_s ent_pos;
        pnt3_vec_t pnt_vec;  

        //������ ������� ��������
        GetEntityPos(m_entity, &ent_pos);
		m_fos.setDirAngle(ent_pos.m_angle);

        //������ ����������� ����� �������
        marker->GetPoints(pnt_vec);

        BaseEntity* mrk_ent = marker->GetEntity();

        for(size_t k = 0; k < pnt_vec.size(); ++k)
		{
			if(    IsInFOS(pnt_vec[k], m_fos, ent_pos)
				&& HaveLOS(m_entity, ent_pos.m_pos, mrk_ent, pnt_vec[k]))
			{
				return VisParams::vpVisible;
			}
        }

        return VisParams::vpInvisible;
    }

    //������� ����� ����� �������� � ������������
	std::auto_ptr<Relation> CreateRelationFor(Marker* marker)
    {
        //���� � ����� ��������� �� ������������
        if(marker->GetEntity() == m_entity)
		{
			return std::auto_ptr<Relation>();
		}

		return std::auto_ptr<Relation>(new UsualMSRelation(this,marker));
    }

    BaseEntity* GetEntity() const { return m_entity; }
    player_type GetPlayer() const { return m_entity->GetPlayer(); }

protected:

    void Update(BaseEntity* entity, event_t event, info_t info)
    {        
        if(event = EV_FOS_CHANGE)
		{
            AIUtils::GetEntityFOS(m_entity, &m_fos);
            VisMap::GetInst()->UpdateSpectator(this, UE_FOS_CHANGE);
        }
    }

    //�������� ���������� �� ����� ���������?
    virtual bool HaveLOS(BaseEntity* ent1, const point3& pos1, BaseEntity* ent2, const point3& pos2) const
    {        
        LOSTracer tracer(ent1->GetEID(), pos1, ent2 ? ent2->GetEID() : 0, pos2, mdTransparent);
        return tracer.HaveLOS();
    }

private:

    struct pos_s;

    void GetEntityPos(BaseEntity* entity, pos_s* pos)
    {
        pos->m_angle = entity->GetGraph()->GetAngle();
        pos->m_pos   = entity->GetGraph()->GetPos3();

        if(GraphHuman* human = entity->GetGraph()->Cast2Human())
            pos->m_pos.z += human->IsSitPose() ? sitting_human_height : standing_human_height;
        else
            pos->m_pos.z += vehicle_height;
    }

    //�������� �������� �� ����� � ���� ���������
    bool IsInFOS(const point3& test, const AIUtils::fos_s& fos, const pos_s& pos) const
    {
        const float vec_x = test.x - pos.m_pos.x;
		if(fabsf(vec_x) > fos.getFront()) return false;
        const float vec_y = test.y - pos.m_pos.y;
		if(fabsf(vec_y) > fos.getFront()) return false;

		const float sqr_vec_len = vec_x*vec_x + vec_y*vec_y; //(vec_x,vec_y);
		if(sqr_vec_len > fos.getSqrFront()) return false;
		if(sqr_vec_len <= fos.getSqrBack()) return true;

        //�������� ��������� � ������
		const float dir_x = fos.getCosAngle();
		const float dir_y = fos.getSinAngle();

		const float vec_len = sqrtf(sqr_vec_len);
        
        return (vec_x*dir_x + vec_y*dir_y) > fos.getCosHalfAngle() * vec_len;
    }

private:

    struct pos_s{
        point3 m_pos;
        float  m_angle;
    };

    AIUtils::fos_s m_fos;
    BaseEntity*    m_entity;    
};

//=====================================================================================//
//                    class HumanSpectator : public EntitySpectator                    //
//=====================================================================================//
// ����������� ��� ��������
class HumanSpectator : public EntitySpectator
{
public:

    HumanSpectator(HumanEntity* human = 0) : 
        EntitySpectator(human), m_fscanner_mode(0){}

    void OnInsert()
    {
        EntitySpectator::OnInsert();
        GetEntity()->Attach(this, EV_CREW_CHANGE);
    }

    DCTOR_DEF(HumanSpectator)

    VisParams CalcVisibility(Marker* marker)
    {
        VisParams result = VisParams::vpInvisible;
		const player_type CurPlayer = PT_PLAYER;

        HumanEntity* human = GetEntity()->Cast2Human();

		if(human->IsRaised(EA_INCREW))
		{
			BaseEntity* entity = marker->GetEntity();

			//���� �� ����� � ��������, �� �� ��� �����
			if(    entity
				&& entity->Cast2Vehicle()
				//&& entity->GetPlayer() == CurPlayer // gvozdoder: was !=
				&& entity == human->GetEntityContext()->GetCrew())
			{
				result = VisParams::vpVisible;
			}
		}
		else
		{
            //������: ���� �� � ��� � ����� ������?
			ScannerThing *scanner = static_cast<ScannerThing*>(human->GetEntityContext()->GetThingInHands(TT_SCANNER));
            m_fscanner_mode = (scanner && scanner->IsInUse());
            result = EntitySpectator::CalcVisibility(marker);
        }

        return result;
    }

	std::auto_ptr<Relation> CreateRelationFor(Marker* marker)
    {
        //���� ��������� � ����
        if(marker->GetEntity() == GetEntity())
		{
			return std::auto_ptr<Relation>(new Relation(marker,this));
		}
        
		return std::auto_ptr<Relation>(new UsualMSRelation(this,marker));
    }

protected:

    void Update(BaseEntity* entity, event_t event, info_t info)
    {        
        if(event == EV_CREW_CHANGE)
		{
            EntityContext* context = entity->GetEntityContext();
            VisMap::GetInst()->UpdateSpectator(context->GetSpectator(), UE_FOS_CHANGE);
            return;
        }

        EntitySpectator::Update(entity, event, info);
    }

    bool HaveLOS(BaseEntity* ent1, const point3& pos1, BaseEntity* ent2, const point3& pos2) const
    {
        return      (m_fscanner_mode && ent2 && ent2->IsRaised(ET_VEHICLE|ET_HUMAN|ET_TRADER))
                ||  EntitySpectator::HaveLOS(ent1, pos1, ent2, pos2);
    }

private:

    bool m_fscanner_mode;
};

//=====================================================================================//
//                   class VehicleSpectator : public EntitySpectator                   //
//=====================================================================================//
// ����������� ��� �������
class VehicleSpectator : public EntitySpectator
{
public:

    VehicleSpectator(VehicleEntity* vehicle = 0)
	:	EntitySpectator(vehicle), m_counter(0)
	{
	}

    DCTOR_DEF(VehicleSpectator)

    void MakeSaveLoad(SavSlot& slot)
    {
        EntitySpectator::MakeSaveLoad(slot);

        if(slot.IsSaving())
		{
            slot << m_counter;
		}
        else
		{
            slot >> m_counter;
		}
    }

    void OnIncCrew(HumanEntity* human)
    {
        if(human->GetPlayer() != PT_PLAYER)
		{
            return;
		}

        if(m_counter++ == 0)
		{
			VisMap::GetInst()->UpdateSpectator(this, UE_FOS_CHANGE);
		}
    }

    void OnDecCrew(HumanEntity* human)
    { 
        if(human->GetPlayer() != PT_PLAYER)
            return;
        
        if(--m_counter == 0)
		{
			VisMap::GetInst()->UpdateSpectator(this, UE_FOS_CHANGE);
		}
    }

	std::auto_ptr<Relation> CreateRelationFor(Marker* marker)
    {
        //���� ��������� � ����
        if(marker->GetEntity() == GetEntity())
		{
			return std::auto_ptr<Relation>(new Relation(marker,this));
		}
       
		return std::auto_ptr<Relation>(new UsualMSRelation(this,marker));
    }

private:
    bool HavePlayer() { return m_counter || GetEntity()->GetPlayer() == PT_PLAYER; }

private:
    int m_counter;
};

DCTOR_IMP(HumanSpectator)
DCTOR_IMP(VehicleSpectator)

//=====================================================================================//
//                         class EntityMarker : public Marker                          //
//=====================================================================================//
class EntityMarker : public Marker
{
public:

    EntityMarker(BaseEntity* entity = 0)
	:	m_entity(entity)
	{
		if(m_entity && m_entity->GetGraph()) m_entity->GetGraph()->Visible(false);
	}

    DCTOR_DEF(EntityMarker)

    //��������� / ��������� ������
    void MakeSaveLoad(SavSlot& slot)
    {
		Marker::MakeSaveLoad(slot);

        if(slot.IsSaving())
		{
            slot << m_entity->GetEID();
        }
		else
		{
            eid_t eid; slot >> eid;
            m_entity = EntityPool::GetInst()->Get(eid);
        }
    }

    //�������� ������ �� ��������
    BaseEntity* GetEntity() const { return m_entity; }
	eid_t GetEID() const { return m_entity->GetEID(); }

    typedef std::vector<point3> pnt3_vec_t;
   
    //�������� ������ ����� ����������� ������
    void GetPoints(pnt3_vec_t &points) const
    {
        points.push_back(m_entity->GetGraph()->GetShotPoint());
    }

private:
    void DoHighlight(bool flag)
    {
        m_entity->GetGraph()->Visible(flag); 
    }

    BaseEntity* m_entity;
};

//=====================================================================================//
//           class HumanMarker : public EntityMarker, private EntityObserver           //
//=====================================================================================//
class HumanMarker : public EntityMarker, private EntityObserver
{
public:
    HumanMarker(HumanEntity* human = 0) : EntityMarker(human)
    {
        if(GetEntity()) GetEntity()->Attach(this, EV_CREW_CHANGE);
    }

    ~HumanMarker() { GetEntity()->Detach(this); }

    DCTOR_DEF(HumanMarker)
        
    //�������� ������ ����� ����������� ������
    void GetPoints(pnt3_vec_t &points) const
    {
        //���� �������� ��  � �������
        if(GetEntity()->IsRaised(EA_NOT_INCREW)) EntityMarker::GetPoints(points);
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        EntityMarker::MakeSaveLoad(slot);

        if(!slot.IsSaving()) GetEntity()->Attach(this, EV_CREW_CHANGE);
    }

private:

    void Update(subject_t subj, event_t event, info_t info)
    {
		if(event == EV_CREW_CHANGE)
		{
			if(GetEntity()->IsRaised(EA_NOT_INCREW))
			{
				SetInvisible(false);
			}
			if(GetEntity()->IsRaised(EA_INCREW))
			{
				SetInvisible(true);
			}
		}
        VisMap::GetInst()->UpdateMarker(this, UE_POS_CHANGE);
    }
};

//=====================================================================================//
//                      class VehicleMarker : public EntityMarker                      //
//=====================================================================================//
class VehicleMarker : public EntityMarker
{
public:
    VehicleMarker(VehicleEntity* vehicle = 0) : EntityMarker(vehicle) {}

    DCTOR_DEF(VehicleMarker)

    void GetPoints(pnt3_vec_t* points)
    {
        GetEntity()->GetGraph()->GetVisPoints(points);
    }
};

DCTOR_IMP(HumanMarker)
DCTOR_IMP(EntityMarker)
DCTOR_IMP(VehicleMarker)

//=====================================================================================//
//                    class BusyCrewStrategy : public CrewStrategy                     //
//=====================================================================================//
// ��������� ��� ��������� ������� �������
class BusyCrewStrategy : public CrewStrategy
{
public:

    DCTOR_DEF(BusyCrewStrategy)

    bool CanJoin(VehicleEntity* vehicle, HumanEntity* human) const { return false; }
    
    void Inc(VehicleEntity* vehicle, HumanEntity* human) { throw CASUS("BusyCrewStrategy: ������� �������!");}
    void Dec(VehicleEntity* vehicle, HumanEntity* human) { throw CASUS("BusyCrewStrategy: ������� �������!");}
    
    void Enable(VehicleEntity* vehicle, bool flag){}
    void MakeSaveLoad(SavSlot& st) { }
    int GetCrewSize(VehicleEntity* vehicle) const  { return 0; }
    HumanEntity* GetDriver(VehicleEntity* vehicle) { return 0; } 
};

//=====================================================================================//
//                    class UsualCrewStrategy : public CrewStrategy                    //
//=====================================================================================//
// �������� ��� ������� �������
class UsualCrewStrategy : public CrewStrategy
{
    HumanEntity* m_driver;
    int m_crew_size;

public:
    UsualCrewStrategy() : m_driver(0), m_crew_size(0) {}

    DCTOR_DEF(UsualCrewStrategy)

    bool CanJoin(VehicleEntity* vehicle, HumanEntity* human) const 
    {
        return      human->GetEntityContext()->GetCrew() == 0 
                &&  (vehicle->GetPlayer() == PT_NONE || vehicle->GetPlayer() == human->GetPlayer())
                &&  vehicle->GetInfo()->GetCapacity() > vehicle->GetEntityContext()->GetCrewSize();
    }
    
    //�������� � �������
    void Inc(VehicleEntity* vehicle, HumanEntity* human)
    {
        if(vehicle->GetPlayer() == PT_NONE)
		{
            vehicle->DropFlags(PT_NONE);
            vehicle->RaiseFlags(human->GetPlayer());
        }
        
        //�������� ������
        if(m_driver == 0 && CanDrive(vehicle,human))
            m_driver = human;
        
        m_crew_size++;

        human->GetEntityContext()->SetCrew(vehicle);

        static_cast<VehicleSpectator*>(vehicle->GetEntityContext()->GetSpectator())->OnIncCrew(human);
        human->GetEntityContext()->SetCrew(vehicle);
    }

    //������� �� �������
    void Dec(VehicleEntity* vehicle, HumanEntity* human)
    {   
        m_crew_size --;

        //���� ���� ���������, �� ������� �����
        if(m_crew_size == 0)
		{
            vehicle->DropFlags(human->GetPlayer());
            vehicle->RaiseFlags(PT_NONE);            
        }
        
        if(m_driver && m_driver->GetEID() == human->GetEID())
		{
            m_driver = 0;
		}

        human->GetEntityContext()->SetCrew(0);

		if(m_crew_size > 0 && m_driver == 0)
		{
			EntityPool::iterator it = EntityPool::GetInst()->begin(ET_HUMAN);
			for(; it != EntityPool::GetInst()->end(); ++it)
			{
				if(HumanEntity *hum = it->Cast2Human())
				{
					if(hum->GetEntityContext()->GetCrew() == vehicle && CanDrive(vehicle,hum))
					{
						m_driver = hum;
						break;
					}
				}
			}
		}

        static_cast<VehicleSpectator*>(vehicle->GetEntityContext()->GetSpectator())->OnDecCrew(human);
    }

    void Enable(VehicleEntity* vehicle, bool flag){}

    void MakeSaveLoad(SavSlot& st)
    {
        if(st.IsSaving())
		{
            st << m_crew_size;
            st << (m_driver ? m_driver->GetEID() : 0);
        }
		else
		{
            st >> m_crew_size;        
            eid_t eid; st >> eid;
            if(eid) m_driver = EntityPool::GetInst()->Get(eid)->Cast2Human();
        }
    }

    int GetCrewSize(VehicleEntity* vehicle) const
    {
        return m_crew_size; 
    }

    HumanEntity* GetDriver(VehicleEntity* vehicle)
    { 
        return m_driver;
    } 

private:
	bool CanDrive(VehicleEntity *veh, HumanEntity *hum)
	{
		return veh->GetInfo()->GetMechanics() <= hum->GetEntityContext()->GetTraits()->GetMechanics();
	}
};

//=====================================================================================//
//                     class BusCrewStrategy : public CrewStrategy                     //
//=====================================================================================//
// �������� ��� ��������
class BusCrewStrategy : public CrewStrategy
{
public:

    BusCrewStrategy() : m_fenable(true), m_crew_size(0){}

    DCTOR_DEF(BusCrewStrategy)

    bool CanJoin(VehicleEntity* vehicle, HumanEntity* human) const
    {
        return      m_fenable 
                &&  human->GetEntityContext()->GetCrew() == 0
                &&  m_crew_size < vehicle->GetInfo()->GetCapacity();
    }

    //�������� � �������
    void Inc(VehicleEntity* vehicle, HumanEntity* human)
    {
        m_crew_size++;

        human->GetEntityContext()->SetCrew(vehicle);

        static_cast<VehicleSpectator*>(vehicle->GetEntityContext()->GetSpectator())->OnIncCrew(human);
    }

    //������� �� �������
    void Dec(VehicleEntity* vehicle, HumanEntity* human)
    {
        m_crew_size--;

        human->GetEntityContext()->SetCrew(0);

        static_cast<VehicleSpectator*>(vehicle->GetEntityContext()->GetSpectator())->OnDecCrew(human);
    }

    void Enable(VehicleEntity* vehicle, bool flag) { m_fenable = flag; }

    void MakeSaveLoad(SavSlot& st)
    {
        if(st.IsSaving())
		{
            st << m_fenable;
            st << m_crew_size;
        }
		else
		{
            st >> m_fenable;  
            st >> m_crew_size;
        }        
    }

    int GetCrewSize(VehicleEntity* vehicle) const
    {
        return m_crew_size; 
    }

    HumanEntity* GetDriver(VehicleEntity* vehicle)
    { 
        return 0;
    } 

private:

    int  m_crew_size;
    bool m_fenable;   
};

DCTOR_IMP(BusCrewStrategy)
DCTOR_IMP(UsualCrewStrategy)
DCTOR_IMP(BusyCrewStrategy)

//======================================================

//
// ������ ����� ��������� �������� ������
//
class HumanDeathStrategy : public DeathStrategy
{
public:
    HumanDeathStrategy() {}

    DCTOR_DEF(HumanDeathStrategy)

    void MakeSaveLoad(SavSlot& st) { }
    
    bool MakeDeath(BaseEntity* actor, BaseEntity* killer)
    {
        HumanEntity* human = actor->Cast2Human();
        if(!human) throw CASUS("HumanDeathStrategy: ������������� �� ��� ��������!");
       
        GraphHuman*   graph = human->GetGraph();
        HumanContext* context = human->GetEntityContext();
        
        //���� unit � �������
        if(VehicleEntity* vehicle = context->GetCrew())
		{
            vehicle->GetEntityContext()->DecCrew(human);
            human->GetGraph()->SetLoc(vehicle->GetGraph()->GetPos2(), human->GetGraph()->GetAngle());
        }
		else
		{
            PathUtils::UnlinkEntity(human);
            EffectMgr::GetInst()->MakeBloodSpot(graph->GetPos3());
        } 
        
        VisMap::GetInst()->Remove(context->GetMarker());
        VisMap::GetInst()->Remove(context->GetSpectator());

        //������� ���� ��������
        HumanContext::iterator itor = context->begin(HPK_IMPLANTS);
        while(itor != context->end())
		{
            BaseThing* thing = &*itor;
            context->RemoveThing(thing);
            delete thing;
            ++itor;
        }
        
        //�������� ��� items �� �����
        itor = context->begin(HPK_ALL);
        while(itor != context->end())
		{
            BaseThing* thing = &*itor;

            context->RemoveThing(thing);
            Depot::GetInst()->Push(graph->GetPos2(), thing);

            ++itor;
        }

        graph->Visible(true);
        
		if(human->IsRaised(EA_EXPLOSIVE_DMG))
		{
			graph->Destroy();
		}
		else
		{
            graph->ChangeAction(GraphEntity::AT_DEAD);
		}

        return true;
    }
};

class VehicleDeathStrategy : public DeathStrategy
{
public:

    VehicleDeathStrategy() {}

    DCTOR_DEF(VehicleDeathStrategy)

    void MakeSaveLoad(SavSlot& st) { }
    
    bool MakeDeath(BaseEntity* actor, BaseEntity* killer)
    {
        VehicleEntity* vehicle = actor->Cast2Vehicle();
        if(!vehicle) throw CASUS("VehicleDeathStrategy: ������� ������������ �������� �� ��� �������");

        VehicleContext* context = vehicle->GetEntityContext();
        
        VisMap::GetInst()->Remove(context->GetMarker());
        VisMap::GetInst()->Remove(context->GetSpectator());

        PathUtils::UnlinkEntity(vehicle);
        
        //���� � ������ ��� ������, ���������� ���
        EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_ALL_PLAYERS, EA_INCREW);
        while(itor != EntityPool::GetInst()->end()){

            HumanEntity* human = itor->Cast2Human();
            
            if(     human->GetEntityContext()->GetCrew() 
                &&  human->GetEntityContext()->GetCrew()->GetEID() == vehicle->GetEID())
                itor->GetEntityContext()->PlayDeath(killer);                
            
            ++itor;
        }
        
        vehicle->GetGraph()->Visible(true);

        point3 pos = vehicle->GetGraph()->GetPos3();
        EffectMgr::GetInst()->MakeHit(vehicle->GetInfo()->GetDeathEffect(), pos, pos);

        vehicle->GetGraph()->Destroy();
        return true;
    }
};

class TraderDeathStrategy : public DeathStrategy{
public:

    TraderDeathStrategy() { }

    DCTOR_DEF(TraderDeathStrategy)

    void MakeSaveLoad(SavSlot& st) { }
    
    bool MakeDeath(BaseEntity* actor, BaseEntity* killer)
    {
        TraderEntity* trader = actor->Cast2Trader();
        if(!trader) throw CASUS("TraderDeathStrategy: ������������� �������� �� ��� ��������");

        trader->GetGraph()->ChangeAction(GraphEntity::AT_DEAD);
        
        VisMap::GetInst()->Remove(trader->GetEntityContext()->GetMarker());
        PathUtils::UnlinkEntity(trader);
        
        trader->GetGraph()->Visible(true);
        EffectMgr::GetInst()->MakeBloodSpot(trader->GetGraph()->GetPos3());

        return true;
    }
};

class FeralCarDeathStrategy : public DeathStrategy{
public:

    FeralCarDeathStrategy() { }
    
    DCTOR_DEF(FeralCarDeathStrategy)

    //����������/��������
    void MakeSaveLoad(SavSlot& st) { }
    
    //��������� �������� ������
    bool MakeDeath(BaseEntity* actor, BaseEntity* killer)
    {
        VehicleEntity* vehicle = actor->Cast2Vehicle();
        if(!vehicle) throw CASUS("FeralCarDeathStrategy: ������������� �������� �� ��� ������!");

        VisMap::GetInst()->Remove(vehicle->GetEntityContext()->GetMarker());
        VisMap::GetInst()->Remove(vehicle->GetEntityContext()->GetSpectator());

        PathUtils::UnlinkEntity(vehicle);
        
        vehicle->GetGraph()->Visible(false);
        vehicle->GetGraph()->ChangeAction(GraphEntity::AT_DEAD);
       
        vehicle->GetEntityContext()->SetHealth(vehicle->GetInfo()->GetHealth());

        return false;
    }
};

DCTOR_IMP(HumanDeathStrategy)
DCTOR_IMP(TraderDeathStrategy)
DCTOR_IMP(VehicleDeathStrategy)
DCTOR_IMP(FeralCarDeathStrategy)

//======================================================

//
// ��������� �����������
//
class HumanMoveStrategy : public MoveStrategy, private EntityObserver{
public:

    HumanMoveStrategy(HumanEntity* human = 0) : 
        m_human(human), m_hex_cost(0)
    {
        if(m_human){
            AttachObs();
            Update(m_human, EV_DEXTERITY_CHANGE, 0);
        }
    }

    ~HumanMoveStrategy()
    { 
        if(m_human) DetachObs();
    }
    
    DCTOR_DEF(HumanMoveStrategy)

    int GetHexCost() const
    {
        return m_hex_cost;
    }

    int GetSteps() const
    {
        HumanContext* context = m_human->GetEntityContext();

        if(context->GetTraits()->GetWeight() > context->GetLimits()->GetWeight())
            return 0;

        return context->GetTraits()->GetMovepnts() / m_hex_cost;
    }

    void MakeSaveLoad(SavSlot& st)
    {
        if(st.IsSaving())
		{
            st << m_hex_cost;
            st << m_human->GetEID();
        }
		else
		{
            st >> m_hex_cost;

            eid_t eid;
            st >> eid;

            m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();

            AttachObs();
        }
    }

private:

    void AttachObs()
    {
        m_human->Attach(this, EV_PREPARE_DEATH);
        m_human->Attach(this, EV_WEIGHT_CHANGE);
        m_human->Attach(this, EV_STRENGTH_CHANGE);
        m_human->Attach(this, EV_DEXTERITY_CHANGE);
        m_human->Attach(this, EV_MOVE_TYPE_CHANGE);
        m_human->Attach(this, EV_POSE_TYPE_CHANGE);
    }

    void DetachObs()
    {
        m_human->Detach(this);
        m_human = 0;
    }
  
    void Update(BaseEntity* entity, event_t event, info_t info)
    {
        if(event == EV_PREPARE_DEATH)
		{
            DetachObs();
            return;
        }

		float dext = static_cast<float>(m_human->GetEntityContext()->GetTraits()->GetDexterity());
		float hexcost = PoL::Inv::HexCostFactor1 * (PoL::Inv::HexCostFactor2 - dext/PoL::Inv::HexCostFactor3);

        //������� ������� ������ � 2-� ���� ������ movepoints
		if(m_human->GetGraph()->IsSitPose()) hexcost *= PoL::Inv::HexCostCrouchFactor;
        //������� ������� ������ ������ movepnts
		if(m_human->GetGraph()->IsRunMove()) hexcost /= PoL::Inv::HexCostRunFactor; 

        //����������� ��������
        if(hexcost < 1.0f) hexcost = 1.0f;

		m_hex_cost = static_cast<int>(hexcost);
        m_human->Notify(EntityObserver::EV_HEX_COST_CHANGE);
    }

private:
    
    int  m_hex_cost;

    HumanEntity* m_human;
};

class VehicleMoveStrategy : public MoveStrategy{
public:

    VehicleMoveStrategy(VehicleEntity* vehicle = 0) :
        m_vehicle(vehicle), m_hex_cost(0)
    {
        if(m_vehicle) CalcHexCost();
    }
    
    DCTOR_DEF(VehicleMoveStrategy)

    int GetHexCost() const
    {
        return m_hex_cost;
    }

    int GetSteps() const
    {
        return m_vehicle->GetEntityContext()->GetMovepnts() / m_hex_cost;
    }

    void MakeSaveLoad(SavSlot& st)
    {
        if(st.IsSaving()){

            st << m_vehicle->GetEID();
            st << m_hex_cost;

        }else{

            eid_t eid; 
            st >> eid;

            m_vehicle = EntityPool::GetInst()->Get(eid)->Cast2Vehicle();

            st >> m_hex_cost;
        }
    }

private:

    void CalcHexCost()
    {        
        m_hex_cost = 4 - m_vehicle->GetInfo()->GetDexterity()/25;    
        if(m_hex_cost <= 0) m_hex_cost = 1;
    }

private:

    int  m_hex_cost;
    VehicleEntity* m_vehicle;
};

DCTOR_IMP(HumanMoveStrategy)
DCTOR_IMP(VehicleMoveStrategy)

//======================================================

//
// ������� ��� ���������� ������ � ��������������� ������
//
class PanicFunc{
public:

    PanicFunc(const std::string& file_name)
    {
        TxtFilePtr txt(file_name);

        txt.ReadColumns(m_columns, m_columns + MAX_COLUMNS);

        row_s       row;
        std::string str;

        std::istringstream istr;

        //������ �������
        for(int line = 1; line < txt->SizeY(); line++){

            txt->GetCell(line, m_columns[CT_MORALE].m_index, str);
            init(istr, str) >> row.m_morale;

            txt->GetCell(line, m_columns[CT_NONE].m_index, str);
            init(istr, str) >> row.m_probability[HPT_NONE];

            txt->GetCell(line, m_columns[CT_USUAL].m_index, str);
            init(istr, str) >> row.m_probability[HPT_USUAL];

            txt->GetCell(line, m_columns[CT_SHOCK].m_index, str);
            init(istr, str) >> row.m_probability[HPT_SHOCK];

            txt->GetCell(line, m_columns[CT_BERSERK].m_index, str);
            init(istr, str) >> row.m_probability[HPT_BERSERK];

            //���������� ������������� �������
            for(int k = 0; k < MAX_PANICS; k++){
                row.m_probability[k] /= 100.0f;
                if(k) row.m_probability[k] += row.m_probability[k-1];
            }
            
            //�������� ��� ���������?
            if(row.m_probability[MAX_PANICS-1] > 1.0000001f){

                std::ostringstream ostr;

                ostr << "PanicFunc: ����� ������������ �� ����� 1 !"<< std::endl;
                ostr << "�������:\t" << file_name  << std::endl;
                ostr << "������:\t" << row.m_morale << std::endl;

                throw CASUS(ostr.str());
            }

            //�������� ������
            m_table.push_back(row);
        }
    }

    ~PanicFunc(){}

    human_panic_type Calc(int morale)
    {
        for(size_t row = 0; row < m_table.size(); row++)
		{
            if(morale >= m_table[row].m_morale)
			{
                float probability = aiNormRand();

                for(int type = 0; type < MAX_PANICS; type++)
				{
                    if(m_table[row].m_probability[type] >= probability)
                        return static_cast<human_panic_type>(type);
                }
            }
        }

        return HPT_NONE;
    }

private:

    //������ ������� ������������
    struct row_s{
        int   m_morale;
        float m_probability[MAX_PANICS];
    };

    typedef std::vector<row_s> panic_vec_t;
    panic_vec_t m_table;

    enum column_type{
        CT_MORALE,
        CT_NONE,
        CT_USUAL,
        CT_SHOCK,
        CT_BERSERK,

        MAX_COLUMNS,
    };

    static column m_columns[MAX_COLUMNS];
};

column PanicFunc::m_columns[MAX_COLUMNS] =
{
    column("morale",    CT_MORALE),
    column("normal",    CT_NONE),
    column("panic",     CT_USUAL),
    column("shock",     CT_SHOCK),
    column("berserk",   CT_BERSERK),
};

//
// ������� ��������� ������� ������
//
class UsualPanicSelector : public PanicTypeSelector{
public:

    UsualPanicSelector() : m_duration(0), m_type(HPT_NONE){}

    DCTOR_DEF(UsualPanicSelector)

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving())
		{
            slot << m_duration;
            slot << static_cast<int>(m_type);
        }
		else
		{
            slot >> m_duration;

            int tmp; slot >> tmp;
            m_type = static_cast<human_panic_type>(tmp);
        }
    }

    human_panic_type Select(HumanEntity* human)
    {
		if(human->IsAlwaysInPanic()) return SetPanic(human,HPT_USUAL);

#ifdef _HOME_VERSION
		{
			std::string val;
			if(DirtyLinks::GetCmdLineArg("-pb",&val))
			{
				int eid = 0;
				try
				{
					eid = boost::lexical_cast<int>(val);
				}
				catch(std::exception &)
				{
				}
				if(eid != 0 && human->GetEID() == eid)
				{
					return SetPanic(human, HPT_BERSERK);
				}
			}
		}
#endif

        if(human->GetPlayer() == PT_CIVILIAN || human->IsRaised(EA_INCREW))
		{
            return SetPanic(human, HPT_NONE);
		}
       
        
        //~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~~~~
        //return SetPanic(human, HPT_SHOCK);
        //~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~~~~
        

        //���� ����������������� ��� �� �������
        if(GetPanic(human) != HPT_NONE && m_duration-- > 0)
		{
            return GetPanic(human);
		}

        /*
        for(int k = 0; k < MAX_ROWS; k++)
		{
            if(morale <= m_table[k].m_morale)
			{
                return SetPanic(human, CalcPanic(m_table[k].m_probability));
            }
        }
        */

        return SetPanic(human, GetPanicFunc()->Calc(human->GetEntityContext()->GetTraits()->GetMorale()));
    }

    human_panic_type GetPanic(HumanEntity* human) const
	{
		return m_type;
	}

    human_panic_type SetPanic(HumanEntity* human, human_panic_type type)
    {
        m_type = type;
        m_duration = CalcDuration(m_type);

        return m_type;
    }

private:

    int CalcDuration(human_panic_type type)
    {
        switch(type)
		{
        case HPT_SHOCK: return SHOCK_PANIC_DURATION;
        case HPT_USUAL: return USUAL_PANIC_DURATION;
        case HPT_BERSERK: return BERSERK_PANIC_DURATION;
        }

        return 0;
    }

    PanicFunc* GetPanicFunc()
    {
        static PanicFunc imp(panic_xls_name);
        return &imp;
    }

private:

    int m_duration;             
    human_panic_type m_type;   
};

DCTOR_IMP(UsualPanicSelector)

//======================================================

//
// ����� ��������� �����
//
class CommonSoundStrategy : public SoundStrategy{
public:

    CommonSoundStrategy(BaseEntity* entity) :
        m_entity(entity) {}

    //���������/������������ ������ �� �������
    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){
            
            slot << m_entity->GetEID();

        }else{

            eid_t eid; slot >> eid;
            m_entity = EntityPool::GetInst()->Get(eid);
        }
    }

protected:

    //�������� ������ �� ��������
    BaseEntity* GetEntity() { return m_entity; }

private:

    BaseEntity* m_entity;
};

//======================================================

//
// ������������ ����� critical miss
//
class ShootSndPlayer : private GameObserver,
                         private EntityObserver                                
{
public:

    ShootSndPlayer(HumanEntity* human = 0) :
        m_human(human), m_flags(0), m_enemy(0), m_counter(0)
    {
        if(m_human) AttachObservers();
    }

    ~ShootSndPlayer()
    {
        if(m_human) DetachObservers();
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){

            slot << m_enemy;
            slot << m_flags;
            slot << m_counter;

            slot << m_human->GetEID();

        }else{

            slot >> m_enemy;
            slot >> m_flags;
            slot >> m_counter;

            eid_t eid; slot >> eid;
            m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();

            AttachObservers();
        }
    }

private:

    void AttachObservers()
    {
        m_human->Attach(this, EV_ACTIVITY_INFO);

        GameEvMessenger::GetInst()->Attach(this, EV_HIT);
        GameEvMessenger::GetInst()->Attach(this, EV_KILL);
        GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
    }

    void DetachObservers()
    {
        m_human->Detach(this);
        GameEvMessenger::GetInst()->Detach(this);
    }

    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
    {
        activity_info_s* ptr = static_cast<activity_info_s*>(info);

        if(ptr->IsEnd()){

            if(m_flags & F_TARGET_MISS && !(m_flags & F_TARGET_HIT))
                m_counter++;
            else
                m_counter = 0;

            if(m_counter == CRITICAL_MISS_COUNT){
                m_counter = 0;
                m_human->GetGraph()->PlaySound(GraphHuman::SND_CRITICAL_MISS);
            }

            m_flags = F_ACTIVITY_END;
        }

        if(ptr->IsBegining()) m_flags = F_ACTIVITY_BEG;            
    }

    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
    {
        if(event == EV_HIT) OnGameEvent(static_cast<hit_info*>(info));
        if(event == EV_KILL) OnGameEvent(static_cast<kill_info*>(info));
        if(event == EV_SHOOT) OnGameEvent(static_cast<shoot_info*>(info));
    }

    void OnGameEvent(shoot_info* ptr)
    {
        if(     ptr->m_victim
            &&  ptr->m_victim->GetEID() == m_human->GetEID())
            m_human->GetGraph()->PlaySound(GraphHuman::SND_UNDER_FIRE);

        if(     ptr->m_actor == 0
            ||  ptr->m_actor->GetEID() != m_human->GetEID())
            return;

        if(     ptr->m_victim == 0
            ||  ptr->m_victim->GetEID() != m_enemy){
            m_counter = 0;
            m_flags  &= ~(F_TARGET_HIT|F_TARGET_MISS);
            m_enemy   = ptr->m_victim ? ptr->m_victim->GetEID() : 0;
            return;
        }

        m_flags |= F_TARGET_MISS;
    }

    void OnGameEvent(hit_info* ptr)
    {
        if(     !(m_flags & F_ACTIVITY_BEG)
            ||  ptr->m_actor == 0
            ||  ptr->m_actor->GetEID() != m_human->GetEID())
            return;

        if(ptr->m_victim && m_enemy == ptr->m_victim->GetEID())
            m_flags |= F_TARGET_HIT;

        if(     ptr->m_hit_effect == termoplasma_explosion
            &&  m_human->GetEntityContext()->GetSpectator()->CalcVisibility(ptr->m_point).IsVisible())
            m_human->GetGraph()->PlaySound(GraphHuman::SND_TERMOPLASMA_EXPLOSION);
    }

    void OnGameEvent(kill_info* ptr)
    {
        if(     ptr->m_victim->IsRaised(EA_EXPLOSIVE_DMG)
            &&  EnemyDetector::getInst()->isHeEnemy4Me(m_human, ptr->m_victim)
            &&  VisMap::GetInst()->GetVisFlags(
                        m_human->GetEntityContext()->GetSpectator(),
                        ptr->m_victim->GetEntityContext()->GetMarker()).IsVisible())
		{
            m_human->GetGraph()->PlaySound(GraphHuman::SND_ENEMY_EXPLODED);
            return;
        }

        if(     ptr->m_killer
            &&  m_human->GetEntityContext()->IsSelected()
            &&  ptr->m_killer->GetEID() == m_human->GetEID()            
            &&  EnemyDetector::getInst()->isHeEnemy4Me(m_human, ptr->m_victim))
		{
            m_human->GetGraph()->PlaySound(GraphHuman::SND_ENEMY_DIED);
            return;
        }
    }

private:

    enum flag_type { 
        F_ACTIVITY_BEG = 1 << 0,
        F_ACTIVITY_END = 1 << 1,

        F_TARGET_HIT  = 1 << 2,
        F_TARGET_MISS = 1 << 3,

        CRITICAL_MISS_COUNT = 3,
    };

    unsigned m_flags;

    int    m_counter;
    eid_t  m_enemy;

    HumanEntity* m_human;
};

//
// ������������ ������ � ����������� �� ������
//
class VisSndPlayer :  private GameObserver,
                        private SpectatorObserver
{
public:

    VisSndPlayer(HumanEntity* human = 0) :
        m_human(human), m_danger(0)
    {
        if(m_human) AttachObservers();
    }

    ~VisSndPlayer()
    {
        DetachObservers();
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){

            slot << m_human->GetEID();

            //�������� ������ ��� ��������� �������
            eid_set_t::iterator itor = m_already_seen_entites.begin();
            while(itor != m_already_seen_entites.end()){
                slot << *itor;
                ++itor;
            }

            //������� �����
            slot << static_cast<eid_t>(0); 

            slot << m_danger;

        }else{

            eid_t eid;

            slot >> eid;
            m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();

            //�����. ������ ��� ��������� �������
            slot >> eid;
            while(eid){
                m_already_seen_entites.insert(eid);
                slot >> eid;
            }

            AttachObservers();

            slot >> m_danger;
        }
    }

private:

    void AttachObservers()
    {
        Spectator* spectator = m_human->GetEntityContext()->GetSpectator();

        spectator->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
        spectator->AttachSpectatorObserver(this, EV_INVISIBLE_MARKER);

        GameEvMessenger::GetInst()->Attach(this, EV_TURN);
    }

    void DetachObservers()
    {
        GameEvMessenger::GetInst()->Detach(this);
        m_human->GetEntityContext()->GetSpectator()->DetachSpectatorObserver(this);
    }

    void PlaySound(GraphHuman::sound_type sound)
    {
        m_human->GetGraph()->PlaySound(sound);                
    }

    //��������� �� ���� ��� �����/����� �� ���� ���������
    void Update(SpectatorObserver::subject_t subj, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
    {
        BaseEntity* entity = static_cast<marker_info*>(info)->m_marker->GetEntity();

        //��� �� �������� ��� �� ��� ��� ������
        if(entity == 0 || m_already_seen_entites.count(entity->GetEID()))
            return;

        m_already_seen_entites.insert(entity->GetEID());

        if(event == EV_VISIBLE_MARKER){

            //��� ����?
            if(EnemyDetector::getInst()->isHeEnemy4Me(m_human, entity)){

                int danger = CalcDangerPoints();

                if(danger > m_danger)
                    PlaySound(GetDangerSound(m_danger = danger)); 
                else
                    PlaySound(GraphHuman::SND_ENEMY_SIGHTED);
            }
            
            //���  �����������?
            if(IsPrettyGirl(entity)) PlaySound(GraphHuman::SND_PRETTY_GIRL);
        }
    }

    GraphHuman::sound_type GetDangerSound(int danger_points)
    {
        if(danger_points <= DANGER_LOW) return GraphHuman::SND_TREATMENT_LOW;
        if(danger_points <= DANGER_MID) return GraphHuman::SND_TREATMENT_MEDIUM;

        return GraphHuman::SND_TREATMENT_HIGH;
    }

    //��������� �� �����
    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
    {
        turn_info* ptr = static_cast<turn_info*>(info);
        
        if(ptr->m_player == m_human->GetPlayer() && ptr->IsBegining()){
            m_danger = 0;
            m_already_seen_entites.clear();
        }
    }

    bool IsPrettyGirl(BaseEntity* entity) const
    {
        return      entity->GetInfo()->GetRID() == prostitute1
                ||  entity->GetInfo()->GetRID() == prostitute2;
    }

    int CalcDangerPoints()
    {
        if(m_human->GetPlayer() != PT_PLAYER)
            return 0;

        int sum = 0;

        Spectator* spectator = m_human->GetEntityContext()->GetSpectator();
        VisMap::MarkIterator_t itor = VisMap::GetInst()->MarkBegin(spectator, VisParams::vpVisible);

        for(; itor != VisMap::GetInst()->MarkEnd(); ++itor)
		{
            BaseEntity* enemy = itor->GetEntity();

            if(enemy && EnemyDetector::getInst()->isHeEnemy4Me(m_human, enemy))
			{
                if(HumanEntity* hum = enemy->Cast2Human())
				{
                    sum += hum->GetInfo()->GetDangerPoints();
				}
                else if(VehicleEntity* veh = enemy->Cast2Vehicle())
				{
                    sum += veh->GetInfo()->GetDangerPoints();
				}
            }
        }

        return sum;
    }

private:

    enum {
        DANGER_LOW  = 30,
        DANGER_MID  = 80,
    };

    int          m_danger;
    HumanEntity* m_human;

    typedef std::set<eid_t> eid_set_t;
    eid_set_t m_already_seen_entites;    //�������� ������� ��� ������
};

//
// ����� �� ���������� ��������� � ����
//
class HandsPackSndPlayer : private EntityObserver{
public:

    HandsPackSndPlayer(HumanEntity* human = 0) :
        m_human(human), m_flags(0)
    {
        if(m_human) m_human->Attach(this, EV_HANDS_PACK_CHANGE);
    }

    ~HandsPackSndPlayer()
    {
        m_human->Detach(this);
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){

            slot << m_flags;
            slot << m_human->GetEID();

        }else{

            slot >> m_flags;

            eid_t eid; slot >> eid;
            m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();
        }
    }

private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        if(m_human->GetPlayer() != PT_PLAYER)
            return;

        if(m_flags & F_REMOVE_SCANNER){
            m_flags &= ~F_REMOVE_SCANNER;
            PlaySound(GraphHuman::SND_REMOVE_SCANNER);
        }
        
        if(BaseThing* hands_thing = m_human->GetEntityContext()->GetThingInHands()){

            if(hands_thing->GetInfo()->IsWeapon()){
                CheckWeapon(static_cast<WeaponThing*>(hands_thing));        
                return;
            }
            
            if(hands_thing->GetInfo()->IsScanner()){
                m_flags |= F_REMOVE_SCANNER;
                PlaySound(GraphHuman::SND_INSERT_SCANNER);
                return;
            }
        }
    }

    void CheckWeapon(WeaponThing* weapon)
    {
        float caliber = weapon->GetInfo()->GetCaliber();

        if(!(m_flags & F_GOT_SHOTGUN) && caliber == shotgun_caliber){
            m_flags |= F_GOT_SHOTGUN;
            PlaySound(GraphHuman::SND_GOT_SHOTGUN);
            return;
        }
        
        if(!(m_flags & F_GOT_AUTOCANNON) && caliber == energy_gun_caliber){
            m_flags |= F_GOT_AUTOCANNON;
            PlaySound(GraphHuman::SND_GOT_AUTOCANNON);
            return;
        }
        
        if(!(m_flags & F_GOT_ENERGY_GUN) && caliber == autocannon_caliber){
            m_flags |= F_GOT_ENERGY_GUN;
            PlaySound(GraphHuman::SND_GOT_ENERGY_GUN);
            return;
        }

        if(!(m_flags & F_GOT_ROCKET_LAUNCHER) && caliber == rocket_launcher_caliber){
            m_flags |= F_GOT_ROCKET_LAUNCHER;
            PlaySound(GraphHuman::SND_GOT_ROCKET_LAUNCHER);
            return;
        }
    }

    void PlaySound(GraphHuman::sound_type type)
    {
        m_human->GetGraph()->PlaySound(type);
    }

private:

    enum {
        F_GOT_SHOTGUN         = 1 << 0,
        F_GOT_AUTOCANNON      = 1 << 1,
        F_GOT_ENERGY_GUN      = 1 << 2,
        F_GOT_ROCKET_LAUNCHER = 1 << 3,

        F_REMOVE_SCANNER      = 1 << 4,
    };

    unsigned     m_flags;
    HumanEntity* m_human;
};

//
// ��������� ����� ��� �����
//
class HumanSoundStrategy : public  CommonSoundStrategy,
                           private EntityObserver                           
{
public:

    HumanSoundStrategy(HumanEntity* human = 0) : 
         m_shoot_sound(human), m_hands_sound(human),
         CommonSoundStrategy(human), m_vis_sound(human)
    {
        if(GetEntity()) AttachObservers();
    }

    ~HumanSoundStrategy()
    {
        DetachObservers();
    }

    DCTOR_DEF(HumanSoundStrategy)

    //����������/�������� ���������
    void MakeSaveLoad(SavSlot& slot)
    {
        m_vis_sound.MakeSaveLoad(slot);
        m_shoot_sound.MakeSaveLoad(slot);
        m_hands_sound.MakeSaveLoad(slot);

        CommonSoundStrategy::MakeSaveLoad(slot);

        if(!slot.IsSaving()) AttachObservers();
    }

private:

    //������������ ������������
    void AttachObservers()
    {
        GetEntity()->Attach(this, EV_NO_LOF);
        GetEntity()->Attach(this, EV_MOVE_COMMAND);
        GetEntity()->Attach(this, EV_LUCKY_STRIKE);
        GetEntity()->Attach(this, EV_WEAPON_RELOAD);
        GetEntity()->Attach(this, EV_CRITICAL_HURT);
        GetEntity()->Attach(this, EV_TEAMMATE_COME);
        GetEntity()->Attach(this, EV_TEAMMATE_JOIN);
        GetEntity()->Attach(this, EV_ATTACK_COMMAND);
        GetEntity()->Attach(this, EV_SELECT_COMMAND);
        GetEntity()->Attach(this, EV_TEAMMATE_LEAVE);
        GetEntity()->Attach(this, EV_TEAMMATE_DISMISS);

        GetEntity()->Attach(this, EV_LEVELUP);
        GetEntity()->Attach(this, EV_OPEN_FAILED);
        GetEntity()->Attach(this, EV_OPEN_SUCCEED);
        GetEntity()->Attach(this, EV_CAR_DRIVE_FAILED);
        GetEntity()->Attach(this, EV_USE_PLASMA_GRENADE);
    }

    //����������� ������������
    void DetachObservers()
    {
        GetEntity()->Detach(this);
    }

    //�������� ������ �� ��������
    HumanEntity* GetEntity()
    { 
        return static_cast<HumanEntity*>(CommonSoundStrategy::GetEntity());
    }

    //��������� ����
    void PlaySound(GraphHuman::sound_type sound)
    {
        GetEntity()->GetGraph()->PlaySound(sound);                
    }

    //��������� �� ���������
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
    {
        switch(event){
        case EV_LUCKY_STRIKE:
            {
                entity_info_s* ptr = static_cast<entity_info_s*>(info);

                if(EnemyDetector::getInst()->isHeEnemy4Me(GetEntity(), ptr->m_entity))
                    PlaySound(GraphHuman::SND_LUCKY_STRIKE);
            }
            break;

        case EV_CRITICAL_HURT:
            PlaySound(GraphHuman::SND_CRITICAL_HURT);
            break;

        case EV_MOVE_COMMAND:
            PlaySound(GraphHuman::SND_MOVE_COMMAND);
            break;

        case EV_ATTACK_COMMAND:
            PlaySound(GraphHuman::SND_ATTACK_COMMAND);
            break;

        case EV_SELECT_COMMAND:
            PlaySound(GraphHuman::SND_SELECT_COMMAND);
            break;

        case EV_TEAMMATE_COME:
            PlaySound(GraphHuman::SND_TEAMMATE_COME);
            break;

        case EV_TEAMMATE_JOIN:
            PlaySound(GraphHuman::SND_TEAMMATE_NEW);
            break;
       
        case EV_TEAMMATE_LEAVE:
            PlaySound(GraphHuman::SND_TEAMMATE_LEAVE);
            break;

        case EV_TEAMMATE_DISMISS:
            PlaySound(GraphHuman::SND_TEAMMATE_DISMISS);
            break;

        case EV_OPEN_FAILED:
            PlaySound(GraphHuman::SND_OPEN_FAILED);
            break;

        case EV_OPEN_SUCCEED:
            PlaySound(GraphHuman::SND_OPEN_SUCCEED);
            break;

        case EV_LEVELUP:
            PlaySound(GraphHuman::SND_LEVELUP);
            break;

        case EV_CAR_DRIVE_FAILED:
            PlaySound(GraphHuman::SND_CAR_DRIVE_FAILED);
            break;

        case EV_NO_LOF:
            if(GetEntity()->GetPlayer() == PT_PLAYER) PlaySound(GraphHuman::SND_NO_LOF);
            break;

        case EV_USE_PLASMA_GRENADE:
            PlaySound(GraphHuman::SND_USE_PLASMA_GRENADE);
            break;

        case EV_WEAPON_RELOAD:
            if(WeaponThing* weapon = static_cast<WeaponThing*>(GetEntity()->GetEntityContext()->GetThingInHands(TT_WEAPON)))
                SndPlayer::GetInst()->Play(GetEntity()->GetGraph()->GetPos3(), weapon->GetInfo()->GetAmmoLoadSound());            
            break;
        }
    }
 
private:

    VisSndPlayer       m_vis_sound;
    ShootSndPlayer     m_shoot_sound;
    HandsPackSndPlayer m_hands_sound;
};

DCTOR_IMP(HumanSoundStrategy)

//======================================================

class TraderSoundStrategy : public SoundStrategy{
public:

    TraderSoundStrategy(TraderEntity* trader = 0) {}

    DCTOR_DEF(TraderSoundStrategy)

    //����������/�������� ���������
    void MakeSaveLoad(SavSlot& slot) {}
};

DCTOR_IMP(TraderSoundStrategy)

//======================================================

class UsualFastAccessStrategy : public  FastAccessStrategy,
                                private EntityObserver
{
public:

    UsualFastAccessStrategy(HumanEntity* human = 0) :
        m_thing(0), m_human(human)
    {
        if(m_human == 0) return;

        AttachObservers();

        ForwardIterator itor(m_human);
        itor.First(0, TT_ALL_ITEMS);
        
        //��������� ������ ������� �������� �������
        m_thing = itor.Get(); 
    }

    DCTOR_DEF(UsualFastAccessStrategy)

    ~UsualFastAccessStrategy() { if(m_human) m_human->Detach(this); }

    BaseThing* GetThing() { return m_thing; }

    void SetThing(BaseThing* thing)
    {   
        m_thing = thing;       

        if(m_human->GetEntityContext()->GetHandsMode() == HumanContext::HM_FAST_ACCESS)
            m_human->Notify(EV_HANDS_PACK_CHANGE);
    }

    bool CanMakeFastAccess()
    {
        ForwardIterator itor(m_human);
        itor.First(0, TT_ALL_ITEMS);
        return !itor.IsDone();
    }

    Iterator* CreateIterator(iter_type type)
    {
        if(type == IT_FORWARD) return new ForwardIterator(m_human);
        if(type == IT_BACKWARD) return new BackwardIterator(m_human);
        return 0;
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving()){

            slot << m_human->GetEID();
            slot << (m_thing ? m_thing->GetTID() : 0);

        }else{

            eid_t eid; slot >> eid;
            m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();

            AttachObservers();
            
            tid_t tid; slot >> tid;

            if(tid){

                HumanContext* context = m_human->GetEntityContext();
                HumanContext::iterator itor = context->begin(HPK_ALL);
                
                while(itor != context->end()){
                    
                    if(itor->GetTID() == tid){
                        m_thing = &*itor;
                        break;
                    }
                        
                    ++itor;
                }
            }
        }
    }

private:

    class CommonIterator : public FastAccessStrategy::Iterator{
    public:

        CommonIterator(HumanEntity* human) :
          m_human(human), m_mask(TT_ALL_ITEMS), m_thing(0) {}

        void First(BaseThing* from, unsigned mask)
        {
            m_mask = mask;
            m_thing = from;

            Next();
        }
        
        bool IsDone() const { return m_thing == 0; }
        
        BaseThing* Get() { return m_thing; }
        
    protected:

        void SetThing(BaseThing* thing) { m_thing = thing; }

        HumanEntity* GetHuman() { return m_human; }

        bool IsHandsThing(HumanContext::iterator& thing) const
        {
            HumanContext* context = m_human->GetEntityContext();
            HumanContext::iterator itor = context->begin(HPK_HANDS);

            return itor != context->end() && itor->GetTID() == thing->GetTID();
        }

        bool WasIterated(HumanContext::iterator& thing) const
        {
            HumanContext* context = m_human->GetEntityContext();
            HumanContext::iterator first = context->begin(HPK_ALL);
            
            while(first != context->end()){
                
                if(first->GetTID() == thing->GetTID())
                    return false;
                
                if(*first == *thing)
                    return true;
                
                ++ first;
            }
            
            return false;
        }

        bool IsSuitable(HumanContext::iterator& thing) const
        {
            return      thing->GetInfo()->GetType() & m_mask
                    &&  ThingDelivery::IsSuitable(m_human, &*thing)
                    &&  ThingDelivery::CanInsert(ThingDelivery::ST_FASTACCESS, m_human, &*thing, HPK_HANDS);
        }  

    private:

        unsigned     m_mask;
        BaseThing*   m_thing;
        HumanEntity* m_human;
    };

    class ForwardIterator : public CommonIterator
	{
    public:

        ForwardIterator(HumanEntity* human) : CommonIterator(human){}
               
        void Next()
        {
            //����� �� ������������� ������� ������
            bool fcan_set_fast_access_ref = Get() == 0;
            
            HumanContext* context = GetHuman()->GetEntityContext();
            
            //��������� �� ������ ��������� ���������
            HumanContext::iterator itor = context->begin(HPK_ALL);
            while(itor != context->end())
			{
                if(     fcan_set_fast_access_ref
                    &&  IsSuitable(itor)
                    &&  !WasIterated(itor)
                    &&  !IsHandsThing(itor))
				{
                    SetThing(&*itor);
                    return;
                }
                
                if(Get() && Get()->GetTID() == itor->GetTID())
                    fcan_set_fast_access_ref = true;
                
                ++itor;
            }

            SetThing(0);
        }        
    };
    
    class BackwardIterator : public CommonIterator
	{
    public:

        BackwardIterator(HumanEntity* human) : CommonIterator(human){}
               
        void Next()
        {
            BaseThing*    prev_thing = 0;
            HumanContext* context = GetHuman()->GetEntityContext();
            
            //������� �� ���� ���������
            HumanContext::iterator itor = context->begin(HPK_ALL);
            while(itor != context->end())
			{
                if(Get() && itor->GetTID() == Get()->GetTID())
				{
                    SetThing(prev_thing);
                    return;
                }
                
                if(     IsSuitable(itor)
                    &&  !WasIterated(itor)
                    &&  !IsHandsThing(itor))
                    prev_thing = &*itor;
                
                ++itor;
            }
            
            //���������� � ���� ����� ��������� �������
            SetThing(prev_thing);
        }        
    };

    void Update(subject_t subj, event_t event, info_t info)
    {
		if(info)
		{
			BaseThing* thing = static_cast<thing_info_s*>(info)->m_thing;

			if(event == EV_INSERT_THING) HandleInsertThing(thing);
			if(event == EV_REMOVE_THING) HandleRemoveThing(thing);
		}
    }

    void HandleInsertThing(BaseThing* thing)
    {
        //���� ������� ��� ���� �����
        if(m_thing) return;

        ForwardIterator itor(m_human);

        //������ � �������� ������ �������
        itor.First(0, TT_ALL_ITEMS);
        SetThing(itor.Get());

        if(GetThing()) m_human->Notify(EV_CAN_MAKE_FAST_ACCESS);
    }

    void HandleRemoveThing(BaseThing* thing)
    {
        //���� ��� ���� � �� �� ������
        if(thing != m_thing) return;

        ForwardIterator itor(m_human);
        BaseThing*      thing4insert = itor.Get();

        //������ � �������� ������ �������
        itor.First(0, TT_ALL_ITEMS);
        while(!itor.IsDone())
		{
            if(*(itor.Get()) == *thing)
			{
                thing4insert = itor.Get();
                break;
            }

            if(thing4insert == 0) thing4insert = itor.Get();

            itor.Next();
        } 
        
        SetThing(thing4insert);

        //�������� ��� �������� �� ����
        if(GetThing() == 0)
		{
            m_human->Notify(EV_CAN_MAKE_FAST_ACCESS);
            m_human->GetEntityContext()->SetHandsMode(HumanContext::HM_HANDS);
        }
    }

    void AttachObservers()
    {
        m_human->Attach(this, EV_INSERT_THING);
        m_human->Attach(this, EV_REMOVE_THING);
    }

private:

    BaseThing*   m_thing;
    HumanEntity* m_human;
};

DCTOR_IMP(UsualFastAccessStrategy)

//======================================================

} // namespace StrategyDetail

//======================================================

//=====================================================================================//
//                    Spectator* StrategyFactory::CreateSpectator()                    //
//=====================================================================================//
Spectator* StrategyFactory::CreateSpectator(spectator_type type, BaseEntity* entity)
{
    if(type == ST_USUAL)
	{
        if(HumanEntity* human = entity->Cast2Human())
		{
            return new HumanSpectator(human);
		}

        if(VehicleEntity* vehicle = entity->Cast2Vehicle())
		{
            return new VehicleSpectator(vehicle);
		}
    }

    return 0;
}
 
//=====================================================================================//
//                     CrewStrategy* StrategyFactory::CreateCrew()                     //
//=====================================================================================//
CrewStrategy* StrategyFactory::CreateCrew(crew_type type, VehicleEntity* vehicle)
{
	switch(type)
	{
		case CT_BUS: return new BusCrewStrategy;
		case CT_BUSY: return new BusyCrewStrategy;
		case CT_USUAL: return new UsualCrewStrategy;
		default: return 0;
	}
}

//=====================================================================================//
//                    DeathStrategy* StrategyFactory::CreateDeath()                    //
//=====================================================================================//
DeathStrategy* StrategyFactory::CreateDeath(death_type type, BaseEntity* entity)
{
	switch(type)
	{
		case DT_HUMAN: return new HumanDeathStrategy;
		case DT_TRADER: return new TraderDeathStrategy;
		case DT_VEHICLE: return new VehicleDeathStrategy;
		case DT_FERALCAR: return new FeralCarDeathStrategy;
		default: return 0;
	}
}

//=====================================================================================//
//                     MoveStrategy* StrategyFactory::CreateMove()                     //
//=====================================================================================//
MoveStrategy* StrategyFactory::CreateMove(move_type type, BaseEntity* entity)
{
	switch(type)
	{
		case MT_HUMAN: return new HumanMoveStrategy(entity->Cast2Human());
		case MT_VEHICLE: return new VehicleMoveStrategy(entity->Cast2Vehicle());
		default: return 0;
	}
}

//=====================================================================================//
//                       Marker* StrategyFactory::CreateMarker()                       //
//=====================================================================================//
Marker* StrategyFactory::CreateMarker(marker_type type, BaseEntity* entity)
{
    if(type == MRK_USUAL)
	{
        if(HumanEntity* human = entity->Cast2Human())
		{
            return new HumanMarker(human);
		}

        if(VehicleEntity* vehicle = entity->Cast2Vehicle())
		{
            return new VehicleMarker(vehicle);
		}

        return new EntityMarker(entity);
    }
    return 0;
}

//=====================================================================================//
//              PanicTypeSelector* StrategyFactory::CreatePanicSelector()              //
//=====================================================================================//
PanicTypeSelector* StrategyFactory::CreatePanicSelector(panic_selector_type type)
{
    if(type == PST_USUAL) return new UsualPanicSelector;
    return 0;
}

//=====================================================================================//
//                    SoundStrategy* StrategyFactory::CreateSound()                    //
//=====================================================================================//
SoundStrategy* StrategyFactory::CreateSound(sound_type type, BaseEntity* entity)
{
    if(type == SND_HUMAN) return new HumanSoundStrategy(entity->Cast2Human());
    if(type == SND_TRADER) return new TraderSoundStrategy(entity->Cast2Trader());

    return 0;
}

//=====================================================================================//
//               FastAccessStrategy* StrategyFactory::CreateFastAccess()               //
//=====================================================================================//
FastAccessStrategy* StrategyFactory::CreateFastAccess(fast_access_type type, HumanEntity* human)
{
    if(type == FAT_USUAL) return new UsualFastAccessStrategy(human);

    return 0;
}
