//
// ������ ����
//
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "Entity.h"
#include "Graphic.h"
#include "HexGrid.h"
#include "HexUtils.h"
#include "PathUtils.h"
#include "entityaux.h"
#include "DirtyLinks.h"
#include "EnemyDetection.h"

namespace PathUtilsDetail
{
}

using namespace PathUtilsDetail;

namespace PathUtilsDetail{

    void ClearGrid()
    {
        HexGrid::cell_iterator  first = HexGrid::GetInst()->first_cell(),
                                last  = HexGrid::GetInst()->last_cell();
        
        while(first != last)(first++)->Clear();
    }

//=========================================================

//
//��� ������� ���� ��������� ��������
//
class Field4Human{
public:

    Field4Human(HumanEntity* human, unsigned flags, PathUtils::near_pnt_t* pnts):
        m_pnts(pnts), m_fcalc_near_pnts((flags & PathUtils::F_CALC_NEAR_PNTS) != 0),
        m_hum(human), m_fcalc_over_invisible((flags & PathUtils::F_CALC_OVER_INVISIBLE) != 0){ }

    bool operator()(ipnt2_t from, ipnt2_t to, int dir, int cost)
    {
        //���� ���� �� ���������
        if(!HexGrid::GetInst()->Get(from).IsPassable(dir))    
            return false;

        if(BaseEntity* entity = HexGrid::GetInst()->Get(to).GetEntity()){
                
            //���� ��� �� ��� ��� �� ������
            if(     entity == m_hum
                || (m_fcalc_over_invisible && !entity->GetGraph()->IsVisible()))
                return true;
            
            if(m_fcalc_near_pnts){
                
                eid_t ent_id = entity->GetEID();
                
                //�������� ����� �������
                if(m_pnts[ent_id].IsDefPnt() || m_pnts[ent_id].m_cost > cost){
                    m_pnts[ent_id].m_cost = cost;
                    m_pnts[ent_id].m_pnt  = from;
                }
            }

            return false;
        }

        return true;
    }
    
private:

    bool m_fcalc_near_pnts;
    bool m_fcalc_over_invisible;

    HumanEntity*            m_hum;
    PathUtils::near_pnt_t*  m_pnts;
};

//=========================================================

//
//��� ������� ���� ��������� ��������
//
class Field4GrenadeExplosion{
public:

    Field4GrenadeExplosion(const point3& point, float rad):
        m_center(point), m_explosion_height(3.0f),
        m_cell_height_difference(1.5f), m_radius2(rad*rad){}

    bool operator()(ipnt2_t from, ipnt2_t to, int dir, int cost)
    {
        //���� ���� �� ���������
        if(     HexGrid::GetInst()->IsOutOfRange(to)
            ||  !HexGrid::GetInst()->Get(from).IsPassable(dir))    
            return false;

        point3 pos = HexGrid::GetInst()->Get(to);

        float dx = pos.x - m_center.x,
              dy = pos.y - m_center.y,
              dz = pos.z - m_center.z,
              cell_dz = pos.z - HexGrid::GetInst()->Get(from).z;
        
        return      dz < m_explosion_height
                &&  cell_dz < m_cell_height_difference
                &&  dx*dx + dy*dy < m_radius2;
    }
    
private:

    const float m_explosion_height;
    const float m_cell_height_difference;

    point3 m_center;
    float  m_radius2;
};

//=========================================================

//
//��� ������� ���� �������� ��� �������
//
class Field4Vehicle{
public:

    Field4Vehicle(VehicleEntity* vehicle, unsigned flags, PathUtils::near_pnt_t* pnts):
      m_fcalc_over_humans(vehicle->GetInfo()->IsTech()), m_pnts(pnts),
      m_fcan_run_over_entities((flags & PathUtils::F_PASS_ALL_ENTITIES) == 0),
      m_veh(vehicle), m_fcalc_near_points((flags & PathUtils::F_CALC_NEAR_PNTS) != 0),
      m_fcalc_over_invisible((flags & PathUtils::F_CALC_OVER_INVISIBLE) && !(flags & PathUtils::F_PASS_ALL_ENTITIES)){}

    ~Field4Vehicle(){}

    bool operator()(ipnt2_t from, ipnt2_t to, int dir, int cost)
    {
        if(m_veh->GetInfo()->IsTinySize()){
            return CanPass(from, dir, cost);
        }        
        
        ipnt2_t pnts[5];

        if(m_veh->GetInfo()->IsSmallSize()){
            HexUtils::GetFrontPnts1(from, dir, pnts);
            return     CanPass(pnts[0], dir, cost)
                    && CanPass(pnts[1], dir, cost)
                    && CanPass(pnts[2], dir, cost);
        }

        //������� ������� ������
        HexUtils::GetFrontPnts2(from, dir, pnts);
        return     CanPass(pnts[0], dir, cost)
                && CanPass(pnts[1], dir, cost)
                && CanPass(pnts[2], dir, cost)
                && CanPass(pnts[3], dir, cost)
                && CanPass(pnts[4], dir, cost);
    }

private:

    bool CanPass(const ipnt2_t& from, int dir, int cost)
    {
        ipnt2_t pnt;

        //���� hex ����� �� ��������� ���� ��� �� �������� �� �����
        if(HexGrid::GetInst()->IsOutOfRange(from) || !HexGrid::GetInst()->Get(from).IsPassable(dir))
            return false;

        //�������� ����� �� hex �������
        HexUtils::GetFrontPnts0(from, dir, &pnt);
        
        if(BaseEntity* entity = HexGrid::GetInst()->Get(pnt).GetEntity())
		{
            //���� ��� �� ��� ��� �� ������
            if(entity == m_veh || (m_fcalc_over_invisible && !entity->GetGraph()->IsVisible()))
                return true;
            
            if(m_fcalc_near_points)
			{
                eid_t ent_id = entity->GetEID();
                
                //�������� ����� �������
                if(m_pnts[ent_id].IsDefPnt() || m_pnts[ent_id].m_cost > cost){
                    m_pnts[ent_id].m_cost = cost;
                    m_pnts[ent_id].m_pnt  = from;
                }
            }
            
            //������ ������:
            //              - ������� ����� �������
			//				- gvozdoder: ������������� �������
            //              - �������
            //              - ����� ���� m_fcalc_over_humans == false
            
            return      m_fcan_run_over_entities
                    &&  entity->GetPlayer() != m_veh->GetPlayer()
					&&  (m_veh->GetPlayer() == PT_PLAYER && EnemyDetector::getInst()->isHeEnemy(entity))
                    &&  (m_fcalc_over_humans && !entity->IsRaised(ET_VEHICLE));
        }

        return true;
    }

private:

    bool m_fcalc_near_points;
    bool m_fcalc_over_humans;
    bool m_fcalc_over_invisible;
    bool m_fcan_run_over_entities;

    VehicleEntity*         m_veh;
    PathUtils::near_pnt_t* m_pnts;
};

//=========================================================

//
//������� ���� ����������
//
class MinDist: public std::binary_function<ipnt2_t, ipnt2_t, bool>{
public:
    
    MinDist(ipnt2_t to): m_to(to){}

    bool operator()(ipnt2_t p1, ipnt2_t p2)
    {
        ipnt2_t d1 = p1 - m_to,
                d2 = p2 - m_to;

        return d1.x*d1.x + d1.y*d1.y < d2.x*d2.x  + d2.y * d2.y;
    }

private:

    ipnt2_t m_to;
};

//=========================================================

//
// ��������� �������� � ������� hex'�� ��������. ���������
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// �������� ������ ���������:
//
//          void operator()(const ipnt2_t& pos, BaseEntity* entity)
//
template<class Functor>
class EntityTraceOperator : private EntityVisitor
{
public:

    EntityTraceOperator(const ipnt2_t& pos, const Functor& oper = Functor())
	:	m_functor(oper),
		m_pos(pos),
		m_vsize(VS_UNKNOWN)
		{
		}

	EntityTraceOperator(const ipnt2_t& pos, VehicleInfo::vehicle_size vsize, const Functor& oper = Functor())
	:	m_functor(oper),
		m_pos(pos),
		m_vsize(vsize)
		{
		}

	void DoVisit(BaseEntity *entity)
	{
		if(entity)
		{
			entity->Accept(*this);
		}
		else
		{
			m_functor(m_pos,entity);
		}

		if(m_vsize != VS_UNKNOWN)
		{
			ApplySize(entity);
		}
	}

	void ApplySize(BaseEntity *entity)
	{
        if(m_vsize == VS_TINY) //vehicle->GetInfo()->IsTinySize())
            return;

        ipnt2_t pnts[HexTraits::border2_size];

        //��� ��������� �������
        HexUtils::GetBorderPnts1(m_pos, pnts);

        m_functor(pnts[0], entity);
        m_functor(pnts[1], entity);
        m_functor(pnts[2], entity);
        m_functor(pnts[3], entity);
        m_functor(pnts[4], entity);
        m_functor(pnts[5], entity);
        
        if(m_vsize == VS_SMALL) //vehicle->GetInfo()->IsSmallSize()) 
            return;
        
        //��� ������� �������
        HexUtils::GetBorderPnts2(m_pos, pnts);

        m_functor(pnts[0], entity);
        m_functor(pnts[1], entity);
        m_functor(pnts[2], entity);
        m_functor(pnts[3], entity);
        m_functor(pnts[4], entity);
        m_functor(pnts[5], entity);
        m_functor(pnts[6], entity);
        m_functor(pnts[7], entity);
        m_functor(pnts[8], entity);
        m_functor(pnts[9], entity);
        m_functor(pnts[10], entity);
        m_functor(pnts[11], entity);
	}

    void Visit(TraderEntity* trader)
    {
        m_functor(m_pos, trader);
    }

    void Visit(HumanEntity* human)
    {
        m_functor(m_pos, human);
    }

    void Visit(VehicleEntity* vehicle)
    {          
        //����������� hex
        m_functor(m_pos, vehicle);
		m_vsize = vehicle->GetInfo()->GetVehicleSize();
	}

    void SetPos(const ipnt2_t& pos) { m_pos = pos; }

    const Functor& GetFunctor() const { return m_functor; }

private:

    ipnt2_t m_pos;
    Functor m_functor;
	VehicleInfo::vehicle_size m_vsize;
};

//=========================================================

//
// �������� �������. �������� � ����
//
class LinkFunctor
{
    HexGrid::hg_slice m_slice;
    HexGrid::cell_iterator m_first;

public:
    LinkFunctor()
	:	m_first(HexGrid::GetInst()->first_cell()),
		m_slice(HexGrid::GetInst()->GetSlice())
	{}

    void operator() (const ipnt2_t& pos, BaseEntity* entity)
    {
        (m_first + m_slice(pos))->LinkEntity(entity);        
    }
};

//
// �������� ������ �������� �� ����
//
class UnlinkFunctor
{
    HexGrid::hg_slice m_slice;
    HexGrid::cell_iterator m_first;

public:
    UnlinkFunctor()
	:	m_first(HexGrid::GetInst()->first_cell()),
		m_slice(HexGrid::GetInst()->GetSlice())
	{}

    void operator() (const ipnt2_t& pos, BaseEntity* entity)
    {
        (m_first + m_slice(pos))->UnlinkEntity();        
    }
};

//
// ���� hex'��� ��� ���������
//
class CollectHexesFunctor{
public:
    
    CollectHexesFunctor(pnt_vec_t* vec_to):
      m_vec(vec_to){}

    void operator() (const ipnt2_t& pos, BaseEntity* entity)
    {
        m_vec->push_back(pos);
    }

private:

    pnt_vec_t* m_vec;
};

//
// �������� �� ��������� �����
//
class IsEmptyFunctor{
public:

    IsEmptyFunctor() :  
     m_first(HexGrid::GetInst()->first_cell()),
     m_fempty(true), m_slice(HexGrid::GetInst()->GetSlice()){}

    void operator() (const ipnt2_t& pos, BaseEntity* entity = 0)
    {
        if(m_fempty) m_fempty = (m_first + m_slice(pos))->GetEntity() == 0;        
    }

    bool IsEmpty() const { return m_fempty; }

private:

    bool m_fempty;

    HexGrid::hg_slice m_slice;
    HexGrid::cell_iterator m_first;
};

//
// �������� ���������� ����
//
class MarkRouteFunctor{
public:

    MarkRouteFunctor(unsigned id) :
     m_first(HexGrid::GetInst()->first_cell()),
     m_id(id), m_slice(HexGrid::GetInst()->GetSlice()){}

    void operator() (const ipnt2_t& pos, BaseEntity* entity)
    {
        (m_first + m_slice(pos))->MarkBusRoute(m_id);
    }

private:

    unsigned m_id;

    HexGrid::hg_slice m_slice;
    HexGrid::cell_iterator m_first;
};

//
// ��������� ���������� ����
//
class UnmarkRouteFunctor
{
public:

    UnmarkRouteFunctor(unsigned id) :
     m_first(HexGrid::GetInst()->first_cell()),
     m_id(id), m_slice(HexGrid::GetInst()->GetSlice()){}

    void operator() (const ipnt2_t& pos, BaseEntity* entity)
    {
        (m_first + m_slice(pos))->UnmarkBusRoute(m_id);
    }

private:

    unsigned m_id;

    HexGrid::hg_slice m_slice;
    HexGrid::cell_iterator m_first;
};

//=========================================================

//
// ���������� ���� ������������
//
class PassFieldCalcer : public EntityVisitor
{
public:
    PassFieldCalcer(pnt_vec_t* front, pnt_vec_t* pass, unsigned flags, PathUtils::near_pnt_t* near_pnts) :
        m_front(front), m_pass(pass), m_near_pnts(near_pnts), m_flags(flags)
    {
    }

    void Visit(HumanEntity* human)
    {
        CalcField(human->GetGraph()->GetPos2(), human->GetEntityContext()->GetStepsCount(), Field4Human(human, m_flags, m_near_pnts), m_front, m_pass);
    }

    void Visit(VehicleEntity* vehicle)
    {
        CalcField(vehicle->GetGraph()->GetPos2(), vehicle->GetEntityContext()->GetStepsCount(), Field4Vehicle(vehicle, m_flags, m_near_pnts), m_front, m_pass);
    }

    void Visit(TraderEntity* trader)
    {
        throw CASUS("PathUtils: ������ ������� ���� ������������ ��� ��������");
    }

    void Calc(const ipnt2_t& center, float radius)
    {
        //��������� ����������� hex � ����� ������
        if(m_pass) m_pass->push_back(center);

        CalcField(center, 0, Field4GrenadeExplosion(HexGrid::GetInst()->Get(center), radius), m_front, m_pass);
    }

private:

    //�������� ��� ���������� ���� ������������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������� �� ������ ����� ������? 
    //
    //  bool operator()(ipnt2_t from, ipnt2_t to, int dir, int cost)
    //
    template<class Cond>
    void CalcField(ipnt2_t from, int steps, Cond cond, pnt_vec_t* front, pnt_vec_t* pass)
    {
        ipnt2_t  cur,  pnt;
        int      cost, exits;
       
        //�������� ���
        ClearGrid();   
        //������ ��������� �����
        for(int i = 0; i < MAX_ENTITIES; m_near_pnts[i++].m_cost = HexGrid::cell::NONE);
        
        //������� ��������� �����
        m_second.clear();
        m_second.push_back(from);

        HexGrid::hg_slice      hs = HexGrid::GetInst()->GetSlice(); 
        HexGrid::cell_iterator cell = HexGrid::GetInst()->first_cell() + hs(from);

        cell->SetCost(0);
        
        //���� ����� ��������� ������������ ����� �����
        while(m_second.size())
		{
            //������ ������ �������
            m_first.swap(m_second);
            //������� ������ ������� ������
            m_second.clear();
            
            //������ �� ����� ������
            for(size_t i = 0; i < m_first.size(); i++)
			{
                cur   = m_first[i];
                exits = 0;
                
                //�������� ����������� ������ �� ������� ����.
                for(int dir = 0; dir < HexTraits::num_sides; dir++)
				{
                    //����� ����� � ��� ���� ������                    
                    HexUtils::GetFrontPnts0(cur, dir, &pnt);
					if(pnt.x < 0 || pnt.y < 0) continue;
					//assert( pnt.x >= 0 && pnt.y >= 0 );

                    //��������� ��������� ����
                    cost = HexGrid::GetInst()->Get(cur).GetCost() + 1;
                    cell = HexGrid::GetInst()->first_cell() + hs(pnt);
                                            
                    // ���� ����������� �������
                    if(cond(cur, pnt, dir, cost))
					{
                        //������� ������
                        exits ++;

                        //����� ��� �� ���� ��� ���� ���� ����� �������� ����
                        if(cell->IsDefPnt() || cell->GetCost() > cost)
						{                        
                            //�������� ���������
                            cell->SetCost(cost);
                            //��������� � ������ ���������� ������
                            m_second.push_back(pnt);
                            
                            //�������� ������ ����������
                            if(front && steps && cost == steps)
                                front->push_back(pnt);

                            //�������� ���������� �����
                            if(pass) pass->push_back(pnt);
                        }
                    }
                }
                
                //�������� �������� � ������� ����������
                if(front && steps && exits < HexTraits::num_sides && cost <= steps)
                    front->push_back(cur);
            }
        }
    }

private:

    unsigned m_flags;

    enum{PNTS_VEC_SIZE = 256};

    //������� ����� � ��� �������� ������ ��������� ��������
    static pnt_vec_t m_first;
    static pnt_vec_t m_second;

    pnt_vec_t* m_front;
    pnt_vec_t* m_pass;
    PathUtils::near_pnt_t* m_near_pnts;
};

pnt_vec_t PassFieldCalcer::m_first;
pnt_vec_t PassFieldCalcer::m_second;

//=========================================================

//
// ���������� ����
//
class PathCalcer : public EntityVisitor{
public:

    PathCalcer(ipnt2_t to, pnt_vec_t& pnts) :
      m_to(to), m_pnts(pnts)
    {
    }

    void Visit(HumanEntity* human)
    {
        GetPath(m_to, m_pnts, MinDist(m_to));
    }

    void Visit(TraderEntity* trader)
    {
        throw CASUS("����� ���� ��� ��������");
    }

    void Visit(VehicleEntity* vehicle)
    {
        GetPath(m_to, m_pnts, MinDist(m_to));
    }

private:

    //�������� ���������� ���� �� ��� ������������ ���� ������������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Cond ������ ������������ ������� ��� ������� 
    // ����� � ���������� ���������� ������� �����:
    //
    //      bool operator() (ipnt2_t pnt1, ipnt2_t pnt2) == true, �� pnt1 ���������������� pnt2 
    //
    // Pnts - ��������� �����, ������ ��������� push_back
    //
    // �������:
    //   path - �������� ����� ����� - ���� 
    //
    // ��������: ���� ������� � ���������� ������� �.�. � ������ ���������� 
    //           �����. �������� �����, � � ����� ������ ����
    //
    template<class Cond, class Pnts>
    void GetPath(ipnt2_t to, Pnts& path, Cond cond)
    {
        ipnt2_t  cur_pos = to;

        //���� �� ���� ������
        if(HexGrid::GetInst()->Get(cur_pos).IsDefPnt())
            return;

        ipnt2_t pnt, sav_pnt; 
        int cost = HexGrid::GetInst()->Get(cur_pos).GetCost();

        HexGrid::hg_slice  hs = HexGrid::GetInst()->GetSlice();        

        HexGrid::const_cell_iterator cell,
                                     last_cell  = HexGrid::GetInst()->last_cell(),
                                     first_cell = HexGrid::GetInst()->first_cell();
		while(cost)
		{

			path.push_back(cur_pos);

			bool found = false;
			//����������� ���� ������ �����
			for(int i = 0; i < HexTraits::num_sides && cost; i++)
			{

				HexUtils::GetFrontPnts0(cur_pos, i, &pnt);
				if(!HexUtils::IsValidHexPos(pnt)) continue;

				cell = first_cell + hs(pnt);

				bool validCell = cell < last_cell && !cell->IsDefPnt(); 
				if(    validCell
					&& (       (cell->GetCost() + 1) == cost 
					||  (cell->GetCost() == cost && cond(pnt, sav_pnt))))
				{
					//����� ���������
					cost = cell->GetCost();
					//�������� �����
					sav_pnt = pnt;               
					found = true;
				}
			}
			if(!found) break;
			cur_pos = sav_pnt;
		}
    }

private:

    ipnt2_t    m_to;
    pnt_vec_t& m_pnts;
};

//=========================================================

} // namespace PathUtilsDetail

//=========================================================

PathUtils::near_pnt_t PathUtils::m_near_pnts[MAX_ENTITIES];

//=====================================================================================//
//                    bool PathUtils::near_pnt_t::IsDefPnt() const                     //
//=====================================================================================//
bool PathUtils::near_pnt_t::IsDefPnt() const
{
    return m_cost == HexGrid::cell::NONE;
}

//=====================================================================================//
//                              bool PathUtils::IsNear()                               //
//=====================================================================================//
bool PathUtils::IsNear(BaseEntity* entity)
{
    return !m_near_pnts[entity->GetEID()].IsDefPnt();
}

//=====================================================================================//
//                const PathUtils::near_pnt_t& PathUtils::GetNearPnt()                 //
//=====================================================================================//
const PathUtils::near_pnt_t& PathUtils::GetNearPnt(BaseEntity* entity)
{
    return m_near_pnts[entity->GetEID()];
}

//=====================================================================================//
//                            void PathUtils::LinkEntity()                             //
//=====================================================================================//
void PathUtils::LinkEntity(BaseEntity* entity)
{
    EntityTraceOperator<LinkFunctor> linker(entity->GetGraph()->GetPos2());
	linker.DoVisit(entity);
    //entity->Accept(linker);
}

//=====================================================================================//
//                            void PathUtils::LinkEntity()                             //
//=====================================================================================//
void PathUtils::LinkEntity(BaseEntity* entity, const ipnt2_t &pos)
{
    EntityTraceOperator<LinkFunctor> linker(pos);
	linker.DoVisit(entity);
    //entity->Accept(linker);
}

//=====================================================================================//
//                           void PathUtils::UnlinkEntity()                            //
//=====================================================================================//
void PathUtils::UnlinkEntity(BaseEntity* entity)
{
    EntityTraceOperator<UnlinkFunctor> unlinker(entity->GetGraph()->GetPos2());
	unlinker.DoVisit(entity);
    //entity->Accept(unlinker);
}

//=====================================================================================//
//                           void PathUtils::CalcPassField()                           //
//=====================================================================================//
void PathUtils::CalcPassField(BaseEntity* entity, pnt_vec_t* front, pnt_vec_t* pass, unsigned flags)
{
    entity->Accept(PassFieldCalcer(front, pass, flags, m_near_pnts));
}

//=====================================================================================//
//                           void PathUtils::CalcPassField()                           //
//=====================================================================================//
void PathUtils::CalcPassField(const ipnt2_t& center, float radius, pnt_vec_t* pass)
{
    PassFieldCalcer calcer(0, pass, 0, m_near_pnts);
    calcer.Calc(center, radius);
}

//=====================================================================================//
//                             void PathUtils::CalcPath()                              //
//=====================================================================================//
void PathUtils::CalcPath(BaseEntity* entity, ipnt2_t to, pnt_vec_t& pnts)
{
    PathCalcer calcer(to, pnts);
    entity->Accept(calcer);
}

//=====================================================================================//
//                           void PathUtils::CalcLandField()                           //
//=====================================================================================//
void PathUtils::CalcLandField(VehicleEntity* vehicle, pnt_vec_t* front)
{
    ipnt2_t pnts[HexTraits::border2_size];

    ClearGrid();

    for(int i = 0; i < MAX_ENTITIES; m_near_pnts[i++].m_cost = HexGrid::cell::NONE);

    if(vehicle->GetInfo()->IsTinySize()){
        pnts[0] = vehicle->GetGraph()->GetPos2();
        FindLandPnts(pnts, pnts + 1, front);
        return;
    }

    if(vehicle->GetInfo()->IsSmallSize()){
        HexUtils::GetBorderPnts1(vehicle->GetGraph()->GetPos2(), pnts);
        FindLandPnts(pnts, pnts + HexTraits::border1_size, front);
        return;
    }

    HexUtils::GetBorderPnts2(vehicle->GetGraph()->GetPos2(), pnts);
    FindLandPnts(pnts, pnts + HexTraits::border2_size, front);
}

//=====================================================================================//
//                           void PathUtils::FindLandPnts()                            //
//=====================================================================================//
void PathUtils::FindLandPnts(ipnt2_t* first, ipnt2_t* last, pnt_vec_t* front)
{
    ipnt2_t pnt;

    HexGrid::cell_iterator cell;
    HexGrid::hg_slice hs = HexGrid::GetInst()->GetSlice();

    while(first != last){
       
        //�������� ����������� ������ �� hex'c�
        for(int dir = 0; dir < HexTraits::num_sides; dir++){

            HexUtils::GetFrontPnts0(*first, dir, &pnt);
            cell = HexGrid::GetInst()->first_cell() + hs(pnt);

            if(     HexGrid::GetInst()->IsOutOfRange(pnt)
                ||  cell->GetEntity()
                ||  !HexGrid::GetInst()->Get(*first).IsPassable(dir))
                continue;

            if(front) front->push_back(pnt);

            cell->LandMark();
        }

        ++first;
    }
}

//=====================================================================================//
//                          void PathUtils::CalcTrackField()                           //
//=====================================================================================//
void PathUtils::CalcTrackField(BaseEntity* entity, ipnt2_t pos, pnt_vec_t* vec)
{
    EntityTraceOperator<CollectHexesFunctor> collector(pos, CollectHexesFunctor(vec));
	collector.DoVisit(entity);
    //entity->Accept(collector);
}

//=====================================================================================//
//                           bool PathUtils::IsEmptyPlace()                            //
//=====================================================================================//
bool PathUtils::IsEmptyPlace(const ipnt2_t& pos, VehicleInfo::vehicle_size vsize)
{
	EntityTraceOperator<IsEmptyFunctor> is_empty_op(pos,vsize);
	is_empty_op.DoVisit(0);
	return is_empty_op.GetFunctor().IsEmpty();

	//IsEmptyFunctor is_empty_op;
	//is_empty_op(pos);
	//return is_empty_op.IsEmpty();

   //EntityTraceOperator<IsEmptyFunctor> is_empty_op(pos);
   //entity->Accept(is_empty_op);
   //return is_empty_op.GetFunctor().IsEmpty();
}

//=====================================================================================//
//                const PathUtils::near_pnt_t& PathUtils::GetNearPnt()                 //
//=====================================================================================//
const PathUtils::near_pnt_t& PathUtils::GetNearPnt(const pnt_vec_t& vec)
{
    static near_pnt_t pnt;
   
    HexGrid::hg_slice  hs = HexGrid::GetInst()->GetSlice();            
    HexGrid::const_cell_iterator cell,
                                 last_cell  = HexGrid::GetInst()->last_cell(),
                                 first_cell = HexGrid::GetInst()->first_cell();    

    pnt.m_cost = HexGrid::cell::NONE;
    for(size_t k = 0; k < vec.size(); k++){

        //�������� ��������� �� ������
        cell = first_cell + hs(vec[k]);
        
        //������ ��������� ����� � ��� �����
        if(      cell < last_cell
            &&  !cell->IsDefPnt() 
            &&  (pnt.m_cost > cell->GetCost() || pnt.IsDefPnt())){
            pnt.m_pnt  = vec[k];
            pnt.m_cost = cell->GetCost();
        }
    } 
    
    return pnt;
}

//=======================================================

eid_t BusDispatcher::m_buses[MAX_BUSES] = 
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//=====================================================================================//
//                             void BusDispatcher::Reset()                             //
//=====================================================================================//
void BusDispatcher::Reset()
{
	std::fill(m_buses,m_buses+MAX_BUSES,0);
}

//=====================================================================================//
//                            void BusDispatcher::Insert()                             //
//=====================================================================================//
void BusDispatcher::Insert(VehicleEntity* bus, BusRoute* route)
{
    typedef EntityTraceOperator<MarkRouteFunctor> marker_t;

    for(int j = 0; j < MAX_BUSES; j++)
	{
        if(m_buses[j] == 0)
		{
            m_buses[j] = bus->GetEID();
            BusRouteWalker(bus, route, marker_t(ipnt2_t(0,0), MarkRouteFunctor(j)));
            break;
        }
    }
}

//=====================================================================================//
//                            void BusDispatcher::Remove()                             //
//=====================================================================================//
void BusDispatcher::Remove(VehicleEntity* bus, BusRoute* route)
{
    typedef EntityTraceOperator<UnmarkRouteFunctor> unmarker_t;
    if(!HexGrid::GetInst()) return;//fixme:Grom ����������� ������ ��� ������ �� ���� ����� ������� ����

    for(int j = 0; j < MAX_BUSES; j++)
	{
        if(m_buses[j] == bus->GetEID())
		{
            m_buses[j] = 0;
            BusRouteWalker(bus, route, unmarker_t(ipnt2_t(0,0), UnmarkRouteFunctor(j)));
            break;
        }
    }
}

//=======================================================

AreaManager::AreaManager(const pnt_vec_t& vec) :
    m_area(vec)
{
}

AreaManager::~AreaManager()
{
}

AreaManager::iterator AreaManager::begin(unsigned entity, unsigned player, unsigned attrs)
{
    return iterator(0,  entity, player, attrs, &m_area);
}

AreaManager::iterator AreaManager::end()
{
    return iterator(m_area.size());
}

AreaManager::Iterator::Iterator(size_t first, unsigned etype, unsigned ptype, unsigned attrs, pnt_vec_t* vec) :
    m_etype(etype), m_ptype(ptype), m_attrs(attrs), m_first(first), m_area(vec)
{
    if(m_area && m_area->size() && !IsSuitable((*m_area)[m_first]))
        operator++ ();
}
        
AreaManager::Iterator& AreaManager::Iterator::operator ++()
{
    for(m_first++; m_first < m_area->size(); m_first++)
	{
        if(IsSuitable((*m_area)[m_first])) return *this;        
    }

    m_first = m_area->size();
    return *this;
}

bool AreaManager::Iterator::IsSuitable(const ipnt2_t hex_pnt)
{
    if(BaseEntity* entity = HexGrid::GetInst()->Get(hex_pnt).GetEntity())
	{
        if(m_eids.count(entity->GetEID()))
            return false;
        
        m_eids.insert(entity->GetEID());
        
        return  entity->GetType() & m_etype
            &&  entity->GetPlayer() & m_ptype
            &&  entity->GetAttributes() & m_attrs;           
    }

    return false;
}
       
BaseEntity* AreaManager::Iterator::operator->()
{
    return HexGrid::GetInst()->Get((*m_area)[m_first]).GetEntity();
}
       
