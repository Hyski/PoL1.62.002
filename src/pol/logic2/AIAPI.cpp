/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description: ����� API ��� ������������� ������� � ������� ��������


Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"
#include "AIAPI.h"
#include "entity.h"
#include "Thing.h"
#include "VisMap.h"
#include "Marker.h"
#include "EnemyDetection.h"
#include "CivilianAI.h"
#include "Graphic.h"
#include "Activity.h"
#include "HexGrid.h"
#include "AIUtils.h"
#include "AIContext.h"
#include "ThingFactory.h"

// using for debug purpose
#include <undercover/interface/console.h>


namespace	/*--- Grom ---------*/
{
	bool CanUseWeaponThing(BaseEntity *be,BaseThing *t)
	{
		//bool GotCannon=false;
		//bool MeInScuba = false;

		//if(!be->Cast2Human()) return true;
		//HumanEntity *he = be->Cast2Human();

		//HumanContext::iterator itor = he->GetEntityContext()->begin(HPK_BODY);

		//ArmorThing *arm=(itor!=he->GetEntityContext()->end() && itor->GetInfo()->IsArmor())?static_cast<ArmorThing*>(&*itor):NULL;
		//if(arm && (arm->GetInfo()->GetSuit()==ARM_SCUBA))
		//	MeInScuba=true;

		//if(t && t->GetInfo()->IsWeapon())
		//{
		//	WeaponThing *wth = static_cast<WeaponThing*>(t);
		//	if(wth->GetInfo()->IsCannon())
		//		GotCannon=true;
		//}
		//else if(t->GetInfo()->IsGrenade())
		//{
		//	if(MeInScuba) return false;
		//}
		//if(GotCannon && !MeInScuba) return false;
		//if(!GotCannon && MeInScuba) return false;

		return true;
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ����� � ����� ������ ������������� �����
//
/////////////////////////////////////////////////////////////////////////////////
//(human&vehicle)
bool FixedWeaponComparator::operator() (const WeaponThing& first, const WeaponThing& second) const
{
	// ����� ������� true ���� first ����� second

	// ������� ��������� ��������
	int f = first.GetInfo()->GetRange();
	if(first.GetAmmo()) f += first.GetAmmo()->GetInfo()->GetRange();
	int s = second.GetInfo()->GetRange(); 
	if(second.GetAmmo()) s += second.GetAmmo()->GetInfo()->GetRange();
	if(f > s) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ����� � ����� ������ ����������� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
bool PatrolWeaponComparator::operator() (const WeaponThing& first, const WeaponThing& second) const
{
	// ����� ������� true ���� first ����� second

	// ������� ����������������
	int f = first.GetShotCount(SHT_AUTOSHOT);
	int s = second.GetShotCount(SHT_AUTOSHOT);
	if(f > s) return true;
	if(f == s)
	{
		// ���������� ������ ����������� 
		int f2 = first.GetInfo()->GetBDmg().m_val;
		if(first.GetAmmo()) f2 += first.GetAmmo()->GetInfo()->GetBDmg().m_val;
		int s2 = second.GetInfo()->GetBDmg().m_val;
		if(second.GetAmmo()) s2 += second.GetAmmo()->GetInfo()->GetBDmg().m_val;
		if(f2 > s2) return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ����� � ����� ������ ���������� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
bool AssaultWeaponComparator::operator() (const WeaponThing& first, const WeaponThing& second) const
{
	// ���������� ������ ����������� 
	int f = first.GetInfo()->GetBDmg().m_val;
	if(first.GetAmmo()) f += first.GetAmmo()->GetInfo()->GetBDmg().m_val;
	int s = second.GetInfo()->GetBDmg().m_val;
	if(second.GetAmmo()) s += second.GetAmmo()->GetInfo()->GetBDmg().m_val;
	if(f > s) return true;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ 
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
// ���������� true, ���� ������ ������� ������� (human&vehicle)
bool EnemyComparator::operator() (eid_t id1, eid_t id2) const
{
	BaseEntity *e = AIAPI::getPtr(m_id);
	BaseEntity *e1 = AIAPI::getPtr(id1);
	BaseEntity *e2 = AIAPI::getPtr(id2);

	if(!e) return false;
	if(!e1 && !e2) return false;
	if(!e1) return false;
	if(!e2) return true;

	return (e->GetGraph()->GetPos3() - e1->GetGraph()->GetPos3()).SqrLength()
		< (e->GetGraph()->GetPos3() - e2->GetGraph()->GetPos3()).SqrLength();

	//return getEntityDanger(id1) > getEntityDanger(id2);
}

// ���������� �������� ��������� ��������� �������� (human&vehicle)
float EnemyComparator::getEntityDanger(eid_t entity) const
{
	if(!entity) return 0.0f;
	float sum = 0.0f;
	float weap = 0.0f;
	float dang = 0.0f;
	float dist = 0.0f;
	float level = 0.0f;
	// ��������� ����� ������ ������ ��������
	BaseEntity* be = EntityPool::GetInst()->Get(entity);
	if(HumanEntity* he = be->Cast2Human())
	{
		// ��� �������
		if(BaseThing* weapon = he->GetEntityContext()->GetThingInHands())
		{
			// � ���� ���-�� ���� � �����
			if(weapon->GetInfo()->GetType() == TT_GRENADE)
			{
				// � �������� � ����� �������
				weap = 10.0f;
			}
			if(weapon->GetInfo()->GetType() == TT_WEAPON)
			{
				// � �������� � ����� ������
				weap = static_cast<WeaponThing*>(weapon)->GetInfo()->GetCaliber();
			}
		}
		// ������ ������� �������� ��������
		level = he->GetInfo()->GetLevelupRise();
	}

	if(VehicleEntity* ve = be->Cast2Vehicle())
	{
		// ��� ������� - ������ ������� - ������� ���������
		dang = ve->GetInfo()->GetDangerPoints();
	}

	// ������ ���������
	dist = (be->GetGraph()->GetPos3() - EntityPool::GetInst()->Get(m_id)->GetGraph()->GetPos3()).Length();
	dist = static_cast<float>(static_cast<int>(dist));

	sum = weap*10000.0f + dang*10000.0f + dist*100.0f + level;
	return sum;
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ (�����������)
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
// ����������� � id �������� � id �����
HexComparator::HexComparator(eid_t im, eid_t enemy)
{
	m_im = im;
	m_enemy = enemy;
	if(BaseEntity* be = EntityPool::GetInst()->Get(im))
	{
		m_im_pos = be->GetGraph()->GetPos3();
	}
	if(BaseEntity* be = EntityPool::GetInst()->Get(enemy))
	{
		m_enemy_pos = be->GetGraph()->GetPos3();
	}
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
FixedHexComparator::FixedHexComparator(eid_t im, eid_t enemy) : HexComparator(im, enemy)
{
}

// ���������� true, ���� ������ ����� �������
// (human&vehicle)
bool FixedHexComparator::operator() (const ipnt2_t& p1, const ipnt2_t& p2) const
{
	// 1. ����� ��� ���� �� �������� � ���� ���������� �� �����
	// 2. ���� �� ����� ����� ��� �� ����� ������ - ����� ���, ���
	//    ����� �� ���

	// ������� ��������� �������� ����� ������
	int range = 0;
	if(HumanEntity* im = EntityPool::GetInst()->Get(getIm())->Cast2Human())
	{
		BaseThing* bth = im->GetEntityContext()->GetThingInHands(TT_WEAPON|TT_GRENADE);

		if(!bth) return false;
		// ���� � ����� ������
		if(bth->GetInfo()->GetType() == TT_WEAPON)
		{
			WeaponThing* weapon = static_cast<WeaponThing*>(bth);
			if(weapon->GetAmmo())
			{
				range = weapon->GetInfo()->GetRange();
				range += weapon->GetAmmo()->GetInfo()->GetRange();
			}
		}
		// ���� � ����� �������
		if(bth->GetInfo()->GetType() == TT_GRENADE)
		{
			GrenadeThing* weapon = static_cast<GrenadeThing*>(bth);
			range = AIUtils::CalcThrowRange(im);
		}
	}
	if(VehicleEntity* ve = EntityPool::GetInst()->Get(getIm())->Cast2Vehicle())
	{
		range = ve->GetInfo()->GetRange();
	}
	// ��������� ��������� �������� �� �� 1
	if(range > 1) range -= 1;

	// ������� ����� � ����� ������ ������� �������� �� ����� ������
	if( (AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getEnemyPos()) <= range) &&
		(AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getEnemyPos()) > range)
		)
	{
		// �� ������� ����� �������� - �� ������� ������
		return true;
	}
	if( (AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getEnemyPos()) > range) &&
		(AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getEnemyPos()) <= range)
		)
	{
		// �� �������� ����� �������� - �� ������� ������
		return false;
	}

	// ����� � ����� ������ ������� �������� �����������
	// ������� ���������� �� ���� �� ���
	return AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getImPos()) < AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getImPos());
}
/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ����������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
PatrolHexComparator::PatrolHexComparator(eid_t im, eid_t enemy) : HexComparator(im, enemy)
{
}

// ���������� true, ���� ������ ����� �������
// (human&vehicle)
bool PatrolHexComparator::operator() (const ipnt2_t& p1, const ipnt2_t& p2) const
{
	// 1. ����� ��� ���� �� �������� � ���� ���������� �� �����
	// 2. ���� �� ����� ����� ��� �� ����� ������ - ����� ���, ���
	//    ������ �� ����

	// ������� ��������� �������� ����� ������
	int range = 0;
	if(HumanEntity* im = EntityPool::GetInst()->Get(getIm())->Cast2Human())
	{
		BaseThing* bth = im->GetEntityContext()->GetThingInHands(
			TT_WEAPON|TT_GRENADE);
		if(!bth) return false;
		// ���� � ����� ������
		if(bth->GetInfo()->GetType() == TT_WEAPON)
		{
			WeaponThing* weapon = static_cast<WeaponThing*>(bth);
			if(weapon->GetAmmo())
			{
				range = weapon->GetInfo()->GetRange();
				range += weapon->GetAmmo()->GetInfo()->GetRange();
			}
		}
		// ���� � ����� �������
		if(bth->GetInfo()->GetType() == TT_GRENADE)
		{
			GrenadeThing* weapon = static_cast<GrenadeThing*>(bth);
			range = AIUtils::CalcThrowRange(im);
		}
	}
	if(VehicleEntity* ve = EntityPool::GetInst()->Get(getIm())->Cast2Vehicle())
	{
		range = ve->GetInfo()->GetRange();
	}
	// ��������� ��������� �������� �� �� 1
	if(range > 1) range -= 1;

	// ������� ����� � ����� ������ ������� �������� �� ����� ������
	if( (AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getEnemyPos()) <= range) &&
		(AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getEnemyPos()) > range)
		)
	{
		// �� ������� ����� �������� - �� ������� ������
		return true;
	}
	if( (AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getEnemyPos()) > range) &&
		(AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getEnemyPos()) <= range)
		)
	{
		// �� �������� ����� �������� - �� ������� ������
		return false;
	}

	if( (AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getEnemyPos()) > range) &&
		(AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getEnemyPos()) > range)
		)
	{
		// �� ����� �������� ������ - ����� ���, ��� ������ �� ����
		return AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getEnemyPos()) >= AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getEnemyPos());
	}

	// �� ����� �������� ����� - ����� ���, ��� ������ �� ����
	// ������� ���������� �� ���� �� ���
	return AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getImPos()) >= AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getImPos());
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ���������� ���� �����
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
AssaultHexComparator::AssaultHexComparator(eid_t im, eid_t enemy) : HexComparator(im, enemy)
{
}

// ���������� true, ���� ������ ����� �������
// (human&vehicle)
bool AssaultHexComparator::operator() (const ipnt2_t& p1, const ipnt2_t& p2) const
{
	// 1. ����� ��� ����, �� �������� � ���� ���������� �� ����� (lof, range)
	// 1a ����� ��� ����, �� �������� ��������� ����� ������ min_damage_percent �� �������������
	// 1b ����� ��� ����, �� �������� ���� ������ min_accuracy
	// 2. ����� ��� ����, ������� ����� � �����, �� �� ����� XX ������ � ����

	// ������ �������
	// 1. ����� ��� ���� �� �������� � ���� ���������� �� �����
	// 2. ���� �� ����� ����� ��� �� ����� ������ - ����� ���, ���
	//    ����� � �����, �� �� ����� XX ������ � ����
	const int XX = 5;

	bool canshoot1 = false, canshoot2 = false;
	bool candamage1 = false, candamage2 = false;
	bool canhit1 = false, canhit2 = false;

	BaseEntity *entity = EntityPool::GetInst()->Get(getIm());
	point3 pt1 = HexGrid::GetInst()->Get(p1);
	point3 pt2 = HexGrid::GetInst()->Get(p2);
	float range1 = AIUtils::dist2D(pt1, getEnemyPos());
	float range2 = AIUtils::dist2D(pt2, getEnemyPos());

	// ������� ��������� �������� ����� ������
	if(HumanEntity* im = entity->Cast2Human())
	{
		BaseThing* bth = im->GetEntityContext()->GetThingInHands(TT_WEAPON|TT_GRENADE);
		if(bth)
		{
			// ���� � ����� ������
			if(bth->GetInfo()->GetType() == TT_WEAPON)
			{
				WeaponThing* weapon = static_cast<WeaponThing*>(bth);
				if(weapon->GetAmmo())
				{
					if(weapon->GetAmmo()->GetInfo()->GetDamageFalloff() != 0.0f)
					{
						float damage = weapon->GetInfo()->GetBDmg().m_val + weapon->GetAmmo()->GetInfo()->GetBDmg().m_val;
						canshoot1 = true; canshoot2 = true;
						float dmg1 = AIUtils::CalcDamage(range1,damage,weapon->GetAmmo()->GetInfo()->GetDamageFalloff());
						float dmg2 = AIUtils::CalcDamage(range2,damage,weapon->GetAmmo()->GetInfo()->GetDamageFalloff());
						candamage1 = (dmg1 > damage*im->GetInfo()->GetAiMinDamage());
						candamage2 = (dmg2 > damage*im->GetInfo()->GetAiMinDamage());
					}
					else
					{
						candamage1 = true; candamage2 = true;
					}

					if(candamage1 != candamage2)
					{
						int acc1 = AIUtils::CalcShootAccuracy(im,weapon,pt1);
						int acc2 = AIUtils::CalcShootAccuracy(im,weapon,pt2);
						canhit1 = (acc1 >= im->GetInfo()->GetAiMinAccuracy());
						canhit2 = (acc2 >= im->GetInfo()->GetAiMinAccuracy());
					}
				}
			}
			// ���� � ����� �������
			if(bth->GetInfo()->GetType() == TT_GRENADE)
			{
				GrenadeThing* weapon = static_cast<GrenadeThing*>(bth);
				float range = std::max(AIUtils::CalcThrowRange(im)-1.0f,1.0f);
				canshoot1 = range1 < range;
				canshoot2 = range2 < range;

				if(canshoot1 && canshoot2)
				{
					candamage1 = true; candamage2 = true;
				}
			}
		}
	}
	else if(VehicleEntity* ve = entity->Cast2Vehicle())
	{
		canshoot1 = true; canshoot2 = true;

		float damage = ve->GetInfo()->GetBDmg().m_val;
		canshoot1 = true; canshoot2 = true;
		float dmg1 = AIUtils::CalcDamage(range1,damage,ve->GetInfo()->GetFallOff());
		float dmg2 = AIUtils::CalcDamage(range2,damage,ve->GetInfo()->GetFallOff());
		candamage1 = (dmg1 > damage*im->GetInfo()->GetAiMinDamage());
		candamage2 = (dmg2 > damage*im->GetInfo()->GetAiMinDamage());
	}

	if(canshoot1 != canshoot2) return canshoot1;
	if(candamage1 != candamage2) return candamage1;
	if(canhit1 != canhit2) return canhit1;

	// ����� �����������
	// ������� ���������� �� ����� �� ���
	if( (range1 > XX) && (range2 > XX) )
	{
		// ��� ����� ������ �� ������
		if(range1 < range2) return true;
		return false;
	}
	if( (range1 <= XX) && (range2 <= XX) )
	{
		// ��� ����� ����� �� ������
		if(range1 < range2) return false;
		return true;
	}
	// ������ ����� ���� ���� ������ ��, � ���� �����
	if(range2 > XX)
	{
		// ������ ������
		return true;
	}
	// ������ ������
	return false;
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������� �������
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
WalkingTechHexComparator::WalkingTechHexComparator(eid_t im, eid_t enemy) : HexComparator(im, enemy)
{
}

//=====================================================================================//
//                   bool WalkingTechHexComparator::operator() const                   //
//=====================================================================================//
// ���������� true, ���� ������ ����� �������
// (vehicle)
bool WalkingTechHexComparator::operator() (const ipnt2_t& p1, const ipnt2_t& p2) const
{
	// 1. ����� ��� ���� �� �������� � ���� ���������� �� �����
	// 2. ���� �� ����� ����� - ����� ���, ��� ����� �� ���
	// 3. ���� �� ����� ������ - ����� ���, ��� ����� � �����

	// ������� ��������� �������� ����� ������
	int range = 0;
	if(VehicleEntity* ve = EntityPool::GetInst()->Get(getIm())->Cast2Vehicle())
	{
		range = ve->GetInfo()->GetRange();
	}
	// ��������� ��������� �������� �� �� 1
	if(range > 1) range -= 1;

	const float range1 = AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getEnemyPos());
	const float range2 = AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getEnemyPos());

	// ������� ����� � ����� ������ ������� �������� �� ����� ������
	if((range1 <= range || range2 <= range) && !(range1 <= range && range2 <= range))
	{
		return range1 <= range;
	}

	if(range1 > range && range2 > range)
	{
		// �� ����� �������� ������ - ����� ���, ��� ����� � �����
		return range1 < range2;
	}

	// �� ����� �������� ����� - ����� ���, ��� ����� �� ���
	// ������� ���������� �� ���� �� ���
	const float irange1 = AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getImPos());
	const float irange2 = AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getImPos());
	return irange1 < irange2;
}

/////////////////////////////////////////////////////////////////////////////////
//
// ����� ��� ��������� ���� ������ ��� ������� � �����
// � ������ ���������� ����� ��������
//
/////////////////////////////////////////////////////////////////////////////////
// (human&vehicle)
NoLineOfFireHexComparator::NoLineOfFireHexComparator(eid_t im, eid_t enemy) : HexComparator(im, enemy)
{
}

// ���������� true, ���� ������ ����� �������
// (human&vehicle)
bool NoLineOfFireHexComparator::operator() (const ipnt2_t& p1, const ipnt2_t& p2) const
{
	// 1. ����� ��� ����, ������� ����� �� ���
	// ������� ���������� �� ���� �� ���
	if( AIUtils::dist2D(HexGrid::GetInst()->Get(p1), getImPos()) < AIUtils::dist2D(HexGrid::GetInst()->Get(p2), getImPos()) ) return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//
// �����, ��������������� API (������ ��������)
//
///////////////////////////////////////////////////////////////////////////////

// �������� ��������� �� ������������ ������ ������
//AIAPI* AIAPI::getInst()
//{
//	static AIAPI inst;
//	return &inst;
//}
//
//// �����������
//AIAPI::AIAPI() 
//{
//}
//
//// ����������
//AIAPI::~AIAPI()
//{
//}

// �������� ������� �� ������������� ��������, ����������� � ��������
float AIAPI::getHealthPercent(eid_t id) // (human&vehicle)
{
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			float max = static_cast<float>(he->GetEntityContext()->GetLimits()->GetHealth());
			float cur = static_cast<float>(he->GetEntityContext()->GetTraits()->GetHealth());
			return cur*100.0f/max;
		}
		if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			float max = static_cast<float>(ve->GetInfo()->GetHealth());
			float cur = static_cast<float>(ve->GetEntityContext()->GetHealth());
			return cur*100.0f/max;
		}
	}
	return 0.0f;
}

// �������� ������� �� ����������, ���������� � ��������
float AIAPI::getMovePointsPercent(eid_t id) // (human&vehicle)
{
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			float max = static_cast<float>(he->GetEntityContext()->GetLimits()->GetMovepnts());
			float cur = static_cast<float>(he->GetEntityContext()->GetTraits()->GetMovepnts());
			return cur*100.0f/max;
		}
		if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			float max = static_cast<float>(ve->GetInfo()->GetMovepnts());
			float cur = static_cast<float>(ve->GetEntityContext()->GetMovepnts());
			return cur*100.0f/max;
		}
	}
	return 0.0f;
}

// �������� ���������� ����������, ���������� � ��������
int AIAPI::getMovePoints(eid_t id) // (human&vehicle)
{
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			return he->GetEntityContext()->GetTraits()->GetMovepnts();
		}
		if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			return ve->GetEntityContext()->GetMovepnts();
		}
	}
	return 0;
}

// �������� ���������� �����, ���������� � ��������
int AIAPI::getSteps(eid_t id) // (human&vehicle)
{
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			return he->GetEntityContext()->GetStepsCount();
		}
		if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			return ve->GetEntityContext()->GetStepsCount();
		}
	}
	return 0;
}

// �������� ���������� �����, ������� ����� ������� ��������
// � �������� ����������� ����������
int AIAPI::getSteps(eid_t id, int movepoints) // (human&vehicle)
{
	if(movepoints < 1) return 0;
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			return movepoints / he->GetEntityContext()->GetHexCost();
		}
		if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			return movepoints / ve->GetEntityContext()->GetHexCost();
		}
	}
	return 0;
}

//=====================================================================================//
//                             void AIAPI::setShootType()                              //
//=====================================================================================//
// ���������� ��� �������� � �������� (����������� ����������� aim,
// � snap ����� ��������)
void AIAPI::setShootType(eid_t id, float aim, float snap) // (human only)
{
	BaseEntity* be = EntityPool::GetInst()->Get(id);
	if(!be) return;

	if(HumanEntity* he = be->Cast2Human())
	{
		float r = aiNormRand()*100.0f;
		if(r < aim)
		{
			// aim ��������
			he->GetEntityContext()->SetShotType(SHT_AIMSHOT);
		}
		else if(r < (aim + snap))
		{
			// snap ��������
			he->GetEntityContext()->SetShotType(SHT_SNAPSHOT);
		}
		else
		{
			// auto ��������
			he->GetEntityContext()->SetShotType(SHT_AUTOSHOT);
		}
	}
}

// ����� � ���� ������ ���������� ������ (���� ���������� �����
// ���, ������ false) �� ���� ���� ����������� ����� ����� ����������
// ���������� ��������� � ������ ������ - ������������
// ������ �������� - ����� ��������� ���� �����
bool AIAPI::takeBestWeapon(eid_t id, const WeaponComparator& wc) // (human only)
{
	BaseEntity* be = EntityPool::GetInst()->Get(id);
	HumanEntity* he = be->Cast2Human();
	HumanContext::iterator cur_weapon = he->GetEntityContext()->begin(HPK_ALL,
		TT_WEAPON);



	WeaponThing* best_weapon = 0;
	while(cur_weapon != he->GetEntityContext()->end())
	{
		WeaponThing* wth = static_cast<WeaponThing*>(&*cur_weapon);
		// ������� ������ ���� ����� � �������� ������� �� ��� �������
		if( !reloadWeapon(id, wth))
		{
			// ������ �� ����������
			++cur_weapon;
			continue;
		}

		// ������� ������ ��������
		if(best_weapon)
		{
			// ���������� �� ������ ����� ��� ����, ������� � ������� ������
			if(wc(*wth, *best_weapon)) best_weapon = wth;
		}
		else best_weapon = wth;
		++cur_weapon;
	}

	if(best_weapon)
	{
		// ������� ���������� ������ - ��������� ��� � ����, ���� ���
		// ����� ���-�� ���

		// ������� ������ �� ��, ��� � �����
		BaseThing * oldthing = he->GetEntityContext()->PoLGetThingInHands();
		if(oldthing)
		{
			// � ����� ���-�� ����
			if(oldthing->GetTID() == best_weapon->GetTID())
			{
				// � ����� ������ ������
				return true;
			}
			// ��������� �� ��� � ������ ������������ ����
			he->GetEntityContext()->RemoveThing(oldthing);
			he->GetEntityContext()->InsertThing(HPK_BACKPACK, oldthing);
		}
		// � ����� ������ ��� - ������� ���� ������ �����
		he->GetEntityContext()->RemoveThing(best_weapon);
		he->GetEntityContext()->InsertThing(HPK_HANDS, best_weapon);
		return true;
	}

	// ����������� ������ ������ ���
	return false;
}

// ����� � ���� ������� (���� ��� - ������ false)
bool AIAPI::takeGrenade(eid_t id) // (human only)
{
	if(!id) return false;
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	HumanContext::iterator itor = he->GetEntityContext()->begin(HPK_ALL,
		TT_GRENADE);
	if(itor != he->GetEntityContext()->end())
	{
		// � ���� ���� �������
		GrenadeThing* grenade = static_cast<GrenadeThing*>(&*itor);
		// ���������, ��� � ��� � �����
		BaseThing * oldthing = he->GetEntityContext()->PoLGetThingInHands();
		if(oldthing)
		{
			// � ����� ���-�� ����
			if(oldthing->GetTID() == grenade->GetTID())
			{
				// � ����� �� ����� �������
				return true;
			}
			// ��������� �� ��� � ������ ������������ ����
			he->GetEntityContext()->RemoveThing(oldthing);
			he->GetEntityContext()->InsertThing(HPK_BACKPACK, oldthing);
		}
		// � ����� ������ ��� - ������� ���� �������
		he->GetEntityContext()->RemoveThing(grenade);
		he->GetEntityContext()->InsertThing(HPK_HANDS, grenade);
		return true;
	}
	return false;
}

// ����� � ���� ���� (���� ��� - ������ false)
// (human only)
bool AIAPI::takeShield(eid_t id)
{
	if(!id) return false;
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	HumanContext::iterator itor = he->GetEntityContext()->begin(HPK_ALL,
		TT_SHIELD);
	if(itor != he->GetEntityContext()->end())
	{
		// � ���� ���� �������� ����
		ShieldThing* shield = static_cast<ShieldThing*>(&*itor);
		// ���������, ��� � ��� � �����
		BaseThing * oldthing = he->GetEntityContext()->PoLGetThingInHands();
		if(oldthing)
		{
			// � ����� ���-�� ����
			if(oldthing->GetTID() == shield->GetTID())
			{
				// � ����� �� ����� ����
				return true;
			}
			// ��������� �� ��� � ������ ������������ ����
			he->GetEntityContext()->RemoveThing(oldthing);
			he->GetEntityContext()->InsertThing(HPK_BACKPACK, oldthing);
		}
		// � ����� ������ ��� - ������� ���� �������� ����
		he->GetEntityContext()->RemoveThing(shield);
		he->GetEntityContext()->InsertThing(HPK_HANDS, shield);
		return true;
	}
	return false;
}

// ���� �� � ������� ���������� ������
bool AIAPI::haveTechWeapon(eid_t id) // (vehicle only)
{
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			if(ve->GetInfo()->GetAmmoCount()) return true;
		}
	}
	return false;
}

// �������� ���������� ����� ������������� ���� � ��������
// (human only)
int AIAPI::getThingCount(eid_t id, thing_type tt)
{
	int num = 0;
	if(id)
	{
		if(HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human())
		{
			HumanContext::iterator itor = he->GetEntityContext()->begin(HPK_ALL,
				tt);
			while(itor != he->GetEntityContext()->end())
			{
				++num;
				++itor;
			}
		}
	}
	return num;
}

// ���������, �������� �� ����� � ���� � ��� ������ ������ - �������� ��
bool AIAPI::isWeaponLoaded(WeaponThing* wth) // (human&vehicle)
{
	if(AmmoThing* ath = wth->GetAmmo())
	{
		if(ath->GetBulletCount()) return true;
		// � ������ ������ ������ - ������� ��
		delete wth->Load(0);
	}
	return false;
}

// �������� ������ ���� ��� ����� � �������� (���������� false ����
// ������ �������� ������������)
bool AIAPI::reloadWeapon(eid_t id, WeaponThing* wth) // (human only)
{
	// ���� ������ �������� - �������
	if(isWeaponLoaded(wth)) return true;
	// ������ ����� �� �������� - ������ � �������� �������, ���������� �� �������
	BaseEntity* be = EntityPool::GetInst()->Get(id);
	HumanEntity* he = be->Cast2Human();
	HumanContext::iterator ammo = he->GetEntityContext()->begin(HPK_ALL,
		TT_AMMO);

	while(ammo != he->GetEntityContext()->end())
	{
		AmmoThing* an_ammo = static_cast<AmmoThing*>(&*ammo);
		if(an_ammo->GetInfo()->GetCaliber() == wth->GetInfo()->GetCaliber())
		{
            //������ ������� ��������
            if(an_ammo->GetBulletCount() > wth->GetInfo()->GetQuantity())
			{
				AmmoThing *new_ammo = static_cast<AmmoThing*>(ThingFactory::GetInst()->CreateAmmo(an_ammo->GetInfo()->GetRID(),wth->GetInfo()->GetQuantity()));
                an_ammo->SetBulletCount(an_ammo->GetBulletCount()-new_ammo->GetBulletCount());
				wth->Load(new_ammo);
			}
			else
			{
				he->GetEntityContext()->RemoveThing(an_ammo);
				wth->Load(&*an_ammo);
			}

			if(isWeaponLoaded(wth)) return true; // ������� � ����� ������ ����
			// � ���� �� ��� - ��������� �����
		}
		++ammo;
	}

	// ����� �������� �� �������
	return false;
}

//=====================================================================================//
//                                bool AIAPI::canHurt()                                //
//=====================================================================================//
bool AIAPI::canHurt(BaseEntity *aggressor, BaseEntity *victim)
{
	VehicleEntity *vevict = victim->Cast2Vehicle();
	if(!vevict) return true;

	return aggressor->GetEntityContext()->GetDamage(DT_STRIKE).m_val > 0
		|| aggressor->GetEntityContext()->GetDamage(DT_ENERGY).m_val > 0;
}

//=====================================================================================//
//                       bool AIAPI::getEnemiesVisibleByEntity()                       //
//=====================================================================================//
// �������� ���� ������� ������ ������
// ���������� true ���� ���� ���� �� ���� ����
// ���� ����������� �� ������ �� �����, �� ���� ������������ �� ������
bool AIAPI::getEnemiesVisibleByEntity(eid_t id, entity_list_t* enemies)// (human&vehicle)
{
	BaseEntity* entity = EntityPool::GetInst()->Get(id);
	if(!entity) return false;

	// ������� �������� �� ������� ������
	VisMap::MarkIterator_t it = VisMap::GetInst()->MarkBegin(entity->GetEntityContext()->GetSpectator(), VisParams::vpVisible);

	// �������� �� ���� ������� �����������
	for(; it != VisMap::GetInst()->MarkEnd(); ++it)
	{
		// �������� ���� �� ���
		BaseEntity* cur_entity = it->GetEntity();

		if(cur_entity && EnemyDetector::getInst()->isHeEnemy4Me(entity, cur_entity) && canHurt(entity,cur_entity))
		{
			// ��� ���� ��� ����
			enemies->insert(cur_entity->GetEID());
		}
	}

	return !enemies->empty();
}

// �������� ���� ������� ������ �������
// ���������� true ���� ���� ���� �� ����� ��������
// ��������� �������� - ��� �������
// (���� 0 - � ����� �������)
// (human&vehicle)
bool AIAPI::getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities,
									   entity_type et)
{
	BaseEntity* entity = EntityPool::GetInst()->Get(id);
	if(!entity) return false;

	// ������� ������
	entities->clear();

	// ������� �������� �� ������� ������
	VisMap::MarkIterator_t it = VisMap::GetInst()->MarkBegin(entity->GetEntityContext()->GetSpectator(),VisParams::vpVisible);

	// �������� �� ���� ������� �����������
	for(; it != VisMap::GetInst()->MarkEnd(); ++it)
	{
		// �������� ��������� �� ��� �������
		BaseEntity* cur_entity = it->GetEntity();

		if(cur_entity && 
			(cur_entity->GetType() & et)
			)
		{
			// �������� ��������
			entities->push_back(cur_entity->GetEID());
		}
	}
	if(entities->empty()) return false;
	return true;
}

// �������� ���� ������� ������ �������
// ���������� true ���� ���� ���� �� ����� ��������
// ������������� �������� - ��� �������
// ��������� �������� - ����, � ������� ������ ���������� ��������
// (���� 0 - � ����� �������)
// (human&vehicle)
bool AIAPI::getEntitiesVisibleByEntity(eid_t id, entity_vector_t* entities,
									   entity_type et, const pnt_vec_t& field)
{
	BaseEntity* entity = EntityPool::GetInst()->Get(id);
	if(!entity) return false;

	// ������� ������
	entities->clear();

	// ������� �������� �� ������� ������
	VisMap::MarkIterator_t it = VisMap::GetInst()->MarkBegin(entity->GetEntityContext()->GetSpectator(),
		VisParams::vpVisible);

	// �������� �� ���� ������� �����������
	for(; it != VisMap::GetInst()->MarkEnd(); ++it)
	{
		// �������� ��������� �� ��� ������� � �������� �� ��� � ����
		BaseEntity* cur_entity = it->GetEntity();

		if(cur_entity && 
			(cur_entity->GetType() & et) &&
			AIAPI::isEntityInBaseField(cur_entity->GetEID(), field)
			)
		{
			// �������� ��������
			// ���� ����� - �������� ������
			entities->push_back(cur_entity->GetEID());
		}
	}
	if(entities->empty()) return false;
	return true;
}

// �������� ���� ������� ����������� ������
// ���������� true ���� ���� ���� �� ���� ����
bool AIAPI::getEnemiesVisibleBySubteam(const std::string& subteam, entity_list_t* enemies)// (human&vehicle)
{
	// ����� �� ���� ��������� �� ������
	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_ALL_ENTS,
		PT_ENEMY | PT_CIVILIAN);
	while(itor != EntityPool::GetInst()->end())
	{
		// ����� ������ ������� ��� ���������� � ������� �� ������ ���������
		if( (itor->GetPlayer() == PT_ENEMY) ||
			(itor->GetPlayer() == PT_CIVILIAN)
			)
		{
			// ������� ������ ��� ����������
			AIContext* context = itor->GetAIContext();
			if(!context)
			{
				// ��� ����������
				++itor;
				continue;
			}
			if(context->getSubteam() != subteam)
			{
				// �� �� ����������
				++itor;
				continue;
			}
		}

		// �������� �������� �� ������ ���������� - ������� � ������ ����
		// ������, ������� ��� �����
		VisMap::MarkIterator_t it = VisMap::GetInst()->MarkBegin(itor->GetEntityContext()->GetSpectator(), VisParams::vpVisible);

		// �������� �� ���� ������� �����������
		for(; it != VisMap::GetInst()->MarkEnd(); ++it)
		{
			// �������� ���� �� ���
			BaseEntity* cur_entity = it->GetEntity();

			if(cur_entity && EnemyDetector::getInst()->isHeEnemy4Me(&*itor, cur_entity))
			{
				// ��� ���� ��� ����
				enemies->insert(cur_entity->GetEID());
			}
		}
		// ��������� �������� �� ������
		++itor;
	}
	// ���������, ��� �����
	if(enemies->empty()) return false;
	return true;
}

// �������� ���� ������� ����������� ������
// ���������� true ���� ���� ���� �� ���� ����
// (human&vehicle)
bool AIAPI::isEntityVisible(eid_t im, eid_t entity)
{
	BaseEntity *pim = EntityPool::GetInst()->Get(im);
	if(!pim) return false;
	BaseEntity *pentity = EntityPool::GetInst()->Get(entity);
	if(!pentity) return false;
	VisParams flags = VisMap::GetInst()->GetVisFlags(pim->GetEntityContext()->GetSpectator(),
		pentity->GetEntityContext()->GetMarker());
	return flags.IsVisible();
}

//=====================================================================================//
//                        eid_t AIAPI::getMostDangerousEnemy()                         //
//=====================================================================================//
// �������� id ������ �������� �� ������� ������
// ������ �������� - ����� ��� ��������� ���� ������ �� ������� ������
// ���������� 0 ���� ��� ������� ������
// (human&vehicle)
eid_t AIAPI::getMostDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec)
{
	return *std::min_element(enemies.begin(),enemies.end(),ec);
}

// �������� id ������ ���������� �� ������� ������
// ������ �������� - ����� ��� ��������� ���� ������ �� ������� ������
// ���������� 0 ���� ��� ������� ������
// (human&vehicle)
eid_t AIAPI::getLeastDangerousEnemy(const entity_list_t& enemies, const EnemyComparator& ec)
{
	eid_t id = 0;
	entity_list_t::const_iterator i = enemies.begin();
	entity_list_t::const_iterator end = enemies.end();
	while(i != end)
	{
		if(!id)
		{
			id = (*i);
			++i;
			continue;
		}
		if( !ec((*i), id) ) id = (*i);
		++i;
	}
	return id;
}

// �������� id ���������� �������� �� ������
// ���������� 0 ���� ��� �������
// (human&vehicle)
eid_t AIAPI::getRandomEntity(const entity_vector_t& entities)
{
	if(entities.empty()) return 0;
	int i = aiRangeRand(entities.size());
	return entities[i];
}

// ��������� �������� �����
void AIAPI::setSitPose(eid_t id) // (human only)
{
	if(id)
	{
		if(HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human())
		{
			if(!he->GetGraph()->IsSitPose())
			{
				he->GetGraph()->ChangePose(GraphHuman::PT_SIT);
			}
		}
	}
}

// ��������� �������� ������
void AIAPI::setStandPose(eid_t id) // (human only)
{
	if(id)
	{
		if(HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human())
		{
			if(he->GetGraph()->IsSitPose())
			{
				he->GetGraph()->ChangePose(GraphHuman::PT_STAND);
			}
		}
	}
}

// ����������� �� ���
void AIAPI::setRun(eid_t id) // (human only)
{
	if(id)
	{
		if(HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human())
		{
			if(!he->GetGraph()->IsRunMove())
			{
				he->GetGraph()->ChangeMoveType(GraphHuman::MT_RUN);
			}
		}
	}
}

// ����������� �� ������
void AIAPI::setWalk(eid_t id) // (human only)
{
	if(id)
	{
		if(HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human())
		{
			if(he->GetGraph()->IsRunMove())
			{
				he->GetGraph()->ChangeMoveType(GraphHuman::MT_WALK);
			}
		}
	}
}

// ���������, ����� �� �������
// (human only)
bool AIAPI::isSitPose(eid_t id)
{
	if(id)
	{
		if(HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human())
		{
			if(he->GetGraph()->IsSitPose())
			{
				return true;
			}
		}
	}
	return false;
}

// ���������, ����� �� �������
// (human only)
bool AIAPI::isStandPose(eid_t id)
{
	if(id)
	{
		if(HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human())
		{
			if(!he->GetGraph()->IsSitPose())
			{
				return true;
			}
		}
	}
	return false;
}

//=====================================================================================//
//                              bool AIAPI::isSafeShot()                               //
//=====================================================================================//
// ���������, �������� �� ������� ������������ ����������
// (human&vehicle)
bool AIAPI::isSafeShot(eid_t id, WeaponThing* weapon, const point3& target)
{
	float radius = 0.0f;
	float dist = 0.0f;
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			// ������� ������ ��������� ���������� ������
			radius = weapon->GetAmmo()->GetInfo()->GetDmgRadius();
			radius *= 1.2f;
		}
		else if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			// ������� ������ ��������� ���������� ������
			radius = ve->GetInfo()->GetDmgRadius();
			radius *= 1.2f;
		}
		dist = (be->GetGraph()->GetPos3() - target).Length();
	}
	// �������� �� ��������� ��� ����
	if(dist < radius) return false;

	return true;
}

// ���������, �������� �� ������ ������� ������������ ����������
// (human only)
bool AIAPI::isSafeThrow(eid_t id, GrenadeThing* grenade, const point3& target)
{
	float radius = 0.0f;
	float dist = 0.0f;
	BaseEntity* be = EntityPool::GetInst()->Get(id);
	if(be)
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			// ������� ������ ��������� ���������� ������
			radius = grenade->GetInfo()->GetDmgRadius();
			radius *= 1.2;
		}
		dist = (be->GetGraph()->GetPos3() - target).Length();
	}
	// �������� �� ��������� ��� ����
	if(dist < radius) return false;

	// �������� ��������� ��� ����� ����������
	std::string subteam = "";
	if(AIContext* context = be->GetAIContext())
	{
		if( (be->GetPlayer() == PT_CIVILIAN) ||
			(be->GetPlayer() == PT_ENEMY)
			)
		{
			subteam = context->getSubteam();
		}
	}
	// ���� ���������� ���, �� ������ ������ - ����� ������
	if(subteam == "") return true;
	// ���������� �� ���� ���������, �������� ���, ������� ������ ����� � ������ ��
	// �������������� � ����� ����������
	EntityPool::iterator i = EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_ENEMY | PT_CIVILIAN);
	EntityPool::iterator end = EntityPool::GetInst()->end();
	while(i != end)
	{
		if(BaseEntity* entity = &*i)
		{
			if((entity->GetGraph()->GetPos3() - target).Length() < radius)
			{
				// �������� ��������� � ������� �������� �� ����� ��������������� ������
				if(AIContext* context = be->GetAIContext())
				{
					if( (entity->GetPlayer() == PT_CIVILIAN) ||
						(entity->GetPlayer() == PT_ENEMY)
						)
					{
						if(subteam == context->getSubteam()) return false;
					}
				}
			}
		}
		++i;
	}
	return true;
}

// ���������� �� �������� ���� ��� ��������
// (human only)
bool AIAPI::shoot(eid_t im, eid_t enemy, Activity** activity, std::string* reason)
{
	if(!im || !enemy)
	{
		*reason = "bad entity";
		return false;
	}
	HumanEntity* he = EntityPool::GetInst()->Get(im)->Cast2Human();
	if(!he)
	{
		*reason = "not human";
		return false;
	}
	// ������� ��, ��� ����� � �����
	BaseThing* bth = he->GetEntityContext()->GetThingInHands();
	if(!bth)
	{
		*reason = "empty hands";
		return false;
	}

	// ��� ������?
	if(bth->GetInfo()->GetType() == TT_WEAPON)
	{
		// ������� ������
		WeaponThing* weapon = static_cast<WeaponThing*>(bth);
		if(!weapon)
		{
			*reason = "no weapon";
			return false;
		}
		// ��������, ����� �� ���������� �� �����
		AIUtils::reason_type rt;
		point3 to = EntityPool::GetInst()->Get(enemy)->GetGraph()->GetShotPoint();
		if(!AIUtils::CanShoot(he, weapon, to, &rt))
		{
			// ������
			*reason = AIUtils::Reason2Str(rt);
			return false;
		}

		// �������� �������� �� ������������ ��� ���� � ����� �������
		if(!isSafeShot(im, weapon, to))
		{
			// �������� �����������
			*reason = "not safe shoot";
			return false;
		}

		if(AIUtils::CalcEfficiency(he,to) == 0)
		{
			*reason = "no effect";
			return false;
		}

		// ��������
		int accuracy_i = AIUtils::CalcShootAccuracy(he, weapon, to);
		float accuracy = AIUtils::NormalizeAccuracy(he, accuracy_i);
		ActivityFactory::controller_type ct = ActivityFactory::CT_ENEMY_SHOOT;
		if(weapon->GetInfo()->GetCaliber() == 2.1f)
		{
			// ��� ����������� ����� ����
			ct = ActivityFactory::CT_BERSERK_SHOOT;
		}
		*activity = ActivityFactory::CreateShoot(he,
			//to,
			accuracy,
			ct,
			ActivityFactory::shoot_info_s(enemy));
		return true;
	}

	// ��� ������ ���� �������
	if(bth->GetInfo()->GetType() != TT_GRENADE) return false;
	// ������� �������
	GrenadeThing* grenade = static_cast<GrenadeThing*>(bth);
	if(!grenade)
	{
		*reason = "bad grenade";
		return false;
	}
	// ��������, ����� �� ������ ������� �� �����
	AIUtils::reason_type rt;
	point3 to = EntityPool::GetInst()->Get(enemy)->GetGraph()->GetPos3();
	if(!AIUtils::CanThrow(he,
		grenade,
		to,
		&rt))
	{
		// ������
		*reason = AIUtils::Reason2Str(rt);
		return false;
	}

	// �������� ������ �� ������������ ��� ���� � ����� �������
	if(!isSafeThrow(im, grenade, to))
	{
		// ������� �����������
		*reason = "not safe throw";
		return false;
	}

	// ������
	{
		const int acc = AIUtils::CalcGrenadeAccuracy(he, (he->GetGraph()->GetPos3() - to).Length());
		const float accuracy = static_cast<float>(acc) * 1e-2f;
		*activity = ActivityFactory::CreateThrow(he,
			//to,
			ActivityFactory::CT_ENEMY_THROW,
			ActivityFactory::shoot_info_s(enemy),
			accuracy);
	}
	return true;
}

//=====================================================================================//
//                                 bool AIAPI::shoot()                                 //
//=====================================================================================//
bool AIAPI::shoot(eid_t im, eid_t enemy, ActivityHolder *activity, std::string* reason)
{
	Activity *act = 0;
	const bool result = shoot(im,enemy,&act,reason);
	activity->Reset(act);
	return result;
}

//=====================================================================================//
//                              bool AIAPI::throwShield()                              //
//=====================================================================================//
// ������ ���� ������ �����, ���� ��� ��������
bool AIAPI::throwShield(eid_t im, eid_t enemy, Activity** activity)
{
	if(!im || !enemy)
	{
		return false;
	}
	HumanEntity* he = EntityPool::GetInst()->Get(im)->Cast2Human();
	if(!he)
	{
		return false;
	}
	// ������� ��, ��� ����� � �����
	BaseThing* bth = he->GetEntityContext()->GetThingInHands();
	if(!bth)
	{
		return false;
	}

	// ��� ������� ����?
	if(bth->GetInfo()->GetType() != TT_SHIELD) return false;
	// ������� ������� ����
	ShieldThing* shield = static_cast<ShieldThing*>(bth);
	if(!shield)
	{
		return false;
	}

	// �������� �����, ���� ����� ������� ������� ����
	point3 to = he->GetGraph()->GetPos3();
	point3 dir = Normalize(EntityPool::GetInst()->Get(enemy)->GetGraph()->GetPos3() - to);
	point3 b_to(to);
	int nt=0;
	do
	{
		to = b_to+dir*(0.2+0.8*frand());


		/*// ��������, ����� �� ������ ���� ����, ���� ����
		if(!AIUtils::CanThrow(he,
		shield,
		to))
		{
		// ������
		return false;
		}	*/
		nt++;
		if(nt>20) {to=b_to+point3(0,0,1);break;}
	}while (!AIUtils::CanThrow(he,		shield,		to));	

	// ������
	{
		const int acc = AIUtils::CalcGrenadeAccuracy(he, (he->GetGraph()->GetPos3() - to).Length());
		const float accuracy = static_cast<float>(acc) * 1e-2f;
		*activity = ActivityFactory::CreateThrow(he,
			//to,
			ActivityFactory::CT_ENEMY_THROW,
			ActivityFactory::shoot_info_s(to),
			accuracy);
	}

	return true;
}

//=====================================================================================//
//                              bool AIAPI::throwShield()                              //
//=====================================================================================//
bool AIAPI::throwShield(eid_t im, eid_t enemy, ActivityHolder *activity)
{
	Activity *act = 0;
	const bool result = throwShield(im,enemy,&act);
	activity->Reset(act);
	return result;
}

// ���������� � ��������� �����, ���� ��� ��������
// (human only)
bool AIAPI::shootRandom(eid_t im, Activity** activity)
{
	if(!im) return false;

	HumanEntity* he = EntityPool::GetInst()->Get(im)->Cast2Human();
	if(!he) return false;
	// ������� ��, ��� ����� � �����
	BaseThing* bth = he->GetEntityContext()->GetThingInHands();
	if(!bth) return false;

	// ��������, �������� �� ��� �������
	if(bth->GetInfo()->GetType() != TT_WEAPON) return false;

	// ������� ������
	WeaponThing* weapon = static_cast<WeaponThing*>(bth);
	if(!weapon) return false;

	// ������� ��������� ����� ������ ���� � �������� �������� �� ������
	float radius = weapon->GetInfo()->GetRange();
	if(!weapon->GetAmmo()) return false;
	radius += weapon->GetAmmo()->GetInfo()->GetRange();
	radius *= aiNormRand();
	float alpha = aiNormRand() * (PI / 16.0f);
	float beta = aiNormRand() * PIm2;

	point3 to(1.0f,0.0f,0.0f);
	Quaternion qt;
	qt.FromAngleAxis(alpha,AXISY);
	to = qt * to;
	qt.FromAngleAxis(beta,AXISZ);
	to = qt * to;
	to = radius * to;
	to += he->GetGraph()->GetPos3();

	// �������� ����� �� � ��� ����������
	if(!AIUtils::CanShoot(he, weapon, to)) return false;

	// ��������
	int accuracy_i = AIUtils::CalcShootAccuracy(he, weapon, to);
	float accuracy = AIUtils::NormalizeAccuracy(he, accuracy_i);
	*activity = ActivityFactory::CreateShoot(he,
		//to,
		accuracy,
		ActivityFactory::CT_BERSERK_SHOOT,
		ActivityFactory::shoot_info_s(to));
	return true;
}

//=====================================================================================//
//                              bool AIAPI::shootRandom()                              //
//=====================================================================================//
bool AIAPI::shootRandom(eid_t im, ActivityHolder *activity)
{
	Activity *act = 0;
	const bool result = shootRandom(im,&act);
	activity->Reset(act);
	return result;
}

// (vehicle only)
bool AIAPI::shootByVehicle(eid_t im, eid_t enemy, Activity** activity, std::string* reason)
{
	if(!im || !enemy)
	{
		*reason = "bad entity";
		return false;
	}
	VehicleEntity* ve = EntityPool::GetInst()->Get(im)->Cast2Vehicle();
	if(!ve)
	{
		*reason = "bad vehicle";
		return false;
	}

	// � ������� ���� ���������� ������?
	if(ve->GetInfo()->GetAmmoCount())
	{
		// ��������, ����� �� ���������� �� �����
		AIUtils::reason_type rt;
		point3 to = EntityPool::GetInst()->Get(enemy)->GetGraph()->GetShotPoint();
		if(!AIUtils::CanShoot(ve, to, &rt))
		{
			// ������
			*reason = AIUtils::Reason2Str(rt);
			return false;
		}

		// �������� �������� �� ������������ ��� ���� � ����� �������
		if(!isSafeShot(im, 0, to))
		{
			// �������� �����������
			*reason = "not safe shoot";
			return false;
		}
		// ��������
		int accuracy_i = ve->GetInfo()->GetAccuracy();
		float accuracy = AIUtils::NormalizeAccuracy(ve, accuracy_i);
		*activity = ActivityFactory::CreateShoot(ve,
			//to,
			accuracy,
			ActivityFactory::CT_ENEMY_SHOOT,
			ActivityFactory::shoot_info_s(enemy));
		return true;
	}
	*reason = "no weapon or ammo";
	return false;
}

bool AIAPI::shootByVehicle(eid_t im, const point3& target, Activity** activity, std::string* reason)
{
	if(!im)
	{
		*reason = "bad entity";
		return false;
	}
	VehicleEntity* ve = EntityPool::GetInst()->Get(im)->Cast2Vehicle();
	if(!ve)
	{
		*reason = "bad vehicle";
		return false;
	}

	// � ������� ���� ���������� ������?
	if(ve->GetInfo()->GetAmmoCount())
	{
		// ��������, ����� �� ���������� �� �����
		AIUtils::reason_type rt;
		if(!AIUtils::CanShoot(ve, target, &rt))
		{
			// ������
			*reason = AIUtils::Reason2Str(rt);
			return false;
		}

		// �������� �������� �� ������������ ��� ���� � ����� �������
		if(!isSafeShot(im, 0, target))
		{
			// �������� �����������
			*reason = "not safe shoot";
			return false;
		}
		// ��������
		int accuracy_i = ve->GetInfo()->GetAccuracy();
		float accuracy = AIUtils::NormalizeAccuracy(ve, accuracy_i);
		*activity = ActivityFactory::CreateShoot(ve,
			//target,
			accuracy,
			ActivityFactory::CT_ENEMY_SHOOT2,
			ActivityFactory::shoot_info_s(im));
		return true;
	}
	*reason = "no weapon or ammo";
	return false;
}

// ������� ��� �������, ������� ����� � ������
void AIAPI::pickupAllNearAmmo(eid_t id) // (human only)
{
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	if(!he) return;

	Depot::iterator itor = Depot::GetInst()->begin(he->GetGraph()->GetPos2(),
		1.5f, TT_AMMO);
	while(itor != Depot::GetInst()->end())
	{
		BaseThing* thing = &*itor;
		// ������ ������� ����� ���������� ��� ����, ����� �������
		int cost = AIUtils::CalcMps4ThingPickUp(thing);
		if(cost > he->GetEntityContext()->GetTraits()->GetMovepnts()) break;
		// �������� �������
		Depot::GetInst()->Remove(itor++);
		he->GetEntityContext()->InsertThing(HPK_BACKPACK, thing);
		// �������� ���������� ����������
		he->GetEntityContext()->GetTraits()->PoLAddMovepnts(-cost);
	}
}

// ������� ��� �������, ������� ����� � ������
void AIAPI::pickupAllNearMedikit(eid_t id) // (human only)
{
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	if(!he) return;

	Depot::iterator itor = Depot::GetInst()->begin(he->GetGraph()->GetPos2(),
		1.5f, TT_MEDIKIT);
	while(itor != Depot::GetInst()->end())
	{
		BaseThing* thing = &*itor;
		// ������ ������� ����� ���������� ��� ����, ����� �������
		int cost = AIUtils::CalcMps4ThingPickUp(thing);
		if(cost > he->GetEntityContext()->GetTraits()->GetMovepnts()) break;
		// �������� �������
		Depot::GetInst()->Remove(itor++);
		he->GetEntityContext()->InsertThing(HPK_BACKPACK, thing);
		// �������� ���������� ����������
		he->GetEntityContext()->GetTraits()->PoLAddMovepnts(-cost);
	}
}

// ������� ��� ������, ������� ����� � ������
void AIAPI::pickupAllNearWeapon(eid_t id) // (human only)
{
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	if(!he) return;

	Depot::iterator itor = Depot::GetInst()->begin(he->GetGraph()->GetPos2(),
		1.5f, TT_WEAPON | TT_GRENADE);
	while(itor != Depot::GetInst()->end())
	{
		BaseThing* thing = &*itor;
		if(!CanUseWeaponThing(he,thing)) {itor++;continue;}
		// ������ ������� ����� ���������� ��� ����, ����� �������
		int cost = AIUtils::CalcMps4ThingPickUp(thing);
		if(cost > he->GetEntityContext()->GetTraits()->GetMovepnts()) break;
		// �������� ������
		Depot::GetInst()->Remove(itor++);
		he->GetEntityContext()->InsertThing(HPK_BACKPACK, thing);
		// �������� ���������� ����������
		he->GetEntityContext()->GetTraits()->PoLAddMovepnts(-cost);
	}
}

// �������� ��� ������ ������ � �������� �� ����� ���� ������ �����
void AIAPI::dropUselessWeapon(eid_t id, const WeaponComparator& wc) // (human only)
{
	// �������� ������ ���������� �� �����, ��������� � �����
	typedef std::list<WeaponThing*> WeaponList_t;
	WeaponList_t weapons;
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	HumanContext::iterator cur_weapon = he->GetEntityContext()->begin(HPK_ALL,
		TT_WEAPON);
	HumanContext::iterator end = he->GetEntityContext()->end();
	while(cur_weapon != end)
	{
		weapons.push_back(static_cast<WeaponThing*>(&*cur_weapon));
		++cur_weapon;
	}
	// ����� ���������� ����� �� �����
	while(weapons.size() > 2)
	{
		// ������ ������ �����
		WeaponList_t::iterator bad = weapons.begin();
		WeaponList_t::iterator cur = weapons.begin();
		++cur;
		WeaponList_t::iterator end = weapons.end();
		while(cur != end)
		{
			if(isWeaponLoaded(*cur) &&
				(!isWeaponLoaded(*bad))
				)
			{
				// ������� ����� ��������, � ������ ���
				++cur;
				continue;
			}
			if((!isWeaponLoaded(*cur)) &&
				!isWeaponLoaded(*bad)
				)
			{
				// ������� ����� ����������, � ������ ��������
				bad = cur;
				++cur;
				continue;
			}
			// ��� ����� ���� ��������, ���� ���
			if( !wc( *(*cur), *(*bad) ) )
			{
				bad = cur;
			}
			++cur;
		}
		// ������� ������ �����
		WeaponThing* wth = *bad;
		he->GetEntityContext()->RemoveThing(wth);
		ipnt2_t pos = he->GetGraph()->GetPos2();
		Depot::GetInst()->Push(pos, wth);
		// ������ ������ ����� �� ������
		weapons.erase(bad);
	}
}

// ������� ������ ���������� �����, ������� ����� � ������
// ���������� true ���� ������� ������� ���-������
bool AIAPI::pickupNearWeapon(eid_t id) // (human only)
{
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	if(!he) return false;

	Depot::iterator itor = Depot::GetInst()->begin(he->GetGraph()->GetPos2(),
		1.5f, TT_WEAPON | TT_GRENADE);
	BaseThing* thing = &*itor;
	while(itor!=Depot::GetInst()->end() && !CanUseWeaponThing(he,thing)) {++itor;thing = &*itor;}
	if(itor != Depot::GetInst()->end())
	{

		// ������ ������� ����� ���������� ��� ����, ����� �������
		int cost = AIUtils::CalcMps4ThingPickUp(thing);
		if(cost > he->GetEntityContext()->GetTraits()->GetMovepnts() && !UnlimitedMoves())
			return false;
		// �������� �����
		Depot::GetInst()->Remove(itor);
		he->GetEntityContext()->InsertThing(HPK_BACKPACK, thing);
		// �������� ���������� ����������
		he->GetEntityContext()->GetTraits()->PoLAddMovepnts(-cost);
		return true;
	}
	return false;
}

// ��������� ���� �� � ������� ��������� ����� ������ ��� �������
// ���� ���� - ���������� true � ��������� �����
bool AIAPI::getWeaponLocation(eid_t id, ipnt2_t *pnt) // (human&vehicle)
{
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		Depot::iterator itor = Depot::GetInst()->begin(be,
			TT_WEAPON|TT_AMMO|TT_GRENADE);
		if(itor != Depot::GetInst()->end())
		{
			float min_dist = 10000.0f;
			ipnt2_t point;
			while(itor != Depot::GetInst()->end())
			{
				float dst = dist(be->GetGraph()->GetPos2(),
					itor.GetGraph()->GetPos2());
				if(min_dist > dst && CanUseWeaponThing(be,&*itor))
				{
					min_dist = dst;
					point = itor.GetGraph()->GetPos2();
				}
				++itor;
			}
			*pnt = point;
			return true;
		}
	}
	return false;
}

// ��������� ���� �� � ������� ��������� ����� ���� ���������������� ����
// ���� ���� - ���������� true � ��������� �����
bool AIAPI::getThingLocation(eid_t id, ipnt2_t *pnt, int type) // (human&vehicle)
{
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		Depot::iterator itor = Depot::GetInst()->begin(be, type);
		if(itor != Depot::GetInst()->end())
		{
			float min_dist = 10000.0f;
			ipnt2_t point;
			while(itor != Depot::GetInst()->end())
			{
				float dst = dist(be->GetGraph()->GetPos2(),
					itor.GetGraph()->GetPos2());
				if(min_dist > dst && CanUseWeaponThing(be,&*itor))
				{
					min_dist = dst;
					point = itor.GetGraph()->GetPos2();
				}
				++itor;
			}
			if(min_dist==10000.f) return false; //���� ������ �� ������ �������
			*pnt = point;
			return true;
		}
	}
	return false;
}

// �������� ������ ������� ������, ����������� �� ���������� �� �����
// ������� �� ����� �����������
// (human&vehicle)
void AIAPI::getBaseField(const ipnt2_t &center, int radius, pnt_vec_t *vector)
{
	for(int i = center.x - radius; i < (center.x + radius); i++)
	{
		for(int j = center.y - radius; j < (center.y + radius); j++)
		{
			if(dist(center, ipnt2_t(i, j)) > radius)
			{
				// �������� �� ����������
				continue;
			}
			if(HexGrid::GetInst()->IsOutOfRange(ipnt2_t(i, j)))
			{
				// �������� �� ���������� ��������
				continue;
			}
			vector->push_back(ipnt2_t(i, j));
		}
	}
}

// �������� �� ������� ������ ������ ������, � ������� ����� �����
// (human&vehicle)
void AIAPI::getReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector, bool busaware)
{
	for(unsigned int i = 0; i < invector.size(); i++)
	{
		// ���� ������ ����� - ���������� ����
		if(HexGrid::GetInst()->Get(invector[i]).IsDefPnt())
			continue;
		// ���� ����� ������� ��������� � ���� �� �������� �������� - ����������
		if(busaware && HexGrid::GetInst()->Get(invector[i]).IsBusRoute())
			continue;
		// ����� ����
		outvector->push_back(invector[i]);
	}
}

// �������� �� ������� ������ ������ ������, � ������� ����� �������� � ������
// (human&vehicle)
void AIAPI::getPanicReachableField(const pnt_vec_t &invector, pnt_vec_t *outvector)
{
	for(unsigned int i = 0; i < invector.size(); i++)
	{
		if(!HexGrid::GetInst()->Get(invector[i]).IsDefPnt())
		{
			outvector->push_back(invector[i]);
		}
	}
}

// �������� ��������� ���� �� ������� ������
ipnt2_t AIAPI::getRandomPoint(const pnt_vec_t &vector)// (human&vehicle)
{
	if(vector.empty()) return ipnt2_t(0, 0);
	int i = aiRangeRand(vector.size());
	return vector[i];
}

// �������� ������ ���� �� ������� ������
// (human&vehicle)
ipnt2_t AIAPI::getBestPoint(const pnt_vec_t &vector, const HexComparator& hc)
{
	int size = vector.size();
	if(size < 1) return ipnt2_t(0, 0);
	if(size == 1) return vector[0];
	ipnt2_t res = vector[0];
	for(int i = 1; i < size; i++)
	{
		if(hc(vector[i], res))
		{
			res = vector[i];
		}
	}
	return res;
}

// ���������, ��������� �� �������� � ������� ����
bool AIAPI::isEntityInBaseField(eid_t id, const pnt_vec_t &vector)// (human&vehicle)
{
	for(unsigned int i = 0; i < vector.size(); i++)
	{
		if(vector[i] == EntityPool::GetInst()->Get(id)->GetGraph()->GetPos2())
			return true;
	}
	return false;
}

// �������� ������� �������� �� ���������
// (human&vehicle)
ipnt2_t AIAPI::getPos2(BaseEntity* be)
{
	if(be)
	{
		return be->GetGraph()->GetPos2();
	}
	return ipnt2_t(0, 0);
}

// �������� ������� �������� �� ���������
// (human&vehicle)
ipnt2_t AIAPI::getPos2(eid_t id)
{
	if(id)
	{
		return getPos2(EntityPool::GetInst()->Get(id));
	}
	return ipnt2_t(0, 0);
}

// �������� ���������� ������� ��������
// (human&vehicle)
point3 AIAPI::getPos3(eid_t id)
{
	if(id)
	{
		if(BaseEntity* be = EntityPool::GetInst()->Get(id))
		{
			return be->GetGraph()->GetPos3();
		}
	}
	return point3(0.0f, 0.0f, 0.0f);
}

// �������� ����, �� ������� ��������� ��������
// (human&vehicle)
float AIAPI::getAngle(eid_t id)
{
	if(id)
	{
		if(BaseEntity* be = EntityPool::GetInst()->Get(id))
		{
			return be->GetGraph()->GetAngle();
		}
	}
	return 0.0f;
}

// �������� ���������� ���������� ����������� �� �������
// ������ 0 ���� ��� ������
int AIAPI::getShootMovePoints(eid_t id, shot_type st) // (human&vehicle)
{
	if(!id) return 0;
	if(BaseEntity* be = EntityPool::GetInst()->Get(id))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			// ������� ������
			BaseThing* bth = he->GetEntityContext()->GetThingInHands();
			if(bth->GetInfo()->GetType() == TT_WEAPON)
			{
				WeaponThing* wth = static_cast<WeaponThing*>(bth);
				return wth->GetMovepnts(st);
			}
			if(bth->GetInfo()->GetType() == TT_GRENADE)
			{
				GrenadeThing* gth = static_cast<GrenadeThing*>(bth);
				return gth->GetInfo()->GetMp2Act();
			}
		}
		if(VehicleEntity* ve = be->Cast2Vehicle())
		{
			return ve->GetInfo()->GetMp4Shot();
		}
	}
	return 0;
}

// ���� �� � �������� �������
bool AIAPI::haveMedikit(eid_t id) // (human only)
{
	if(!id) return false;
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	if(!he) return false;
	// ������� �������
	HumanContext::iterator medikit = he->GetEntityContext()->begin(HPK_ALL,
		TT_MEDIKIT);
	while(medikit != he->GetEntityContext()->end())
	{
		MedikitThing* mt = static_cast<MedikitThing*>(&*medikit);
		if(mt->GetCharge())
		{
			// ���� �� ������ �������
			return true;
		}
		// ��� ������ �������
		he->GetEntityContext()->RemoveThing(mt);
		delete mt;
		++medikit;
	}
	return false;
}

// ����� � ���� �������
void AIAPI::takeMedikit(eid_t id) // (human only)
{
	if(!id) return;
	HumanEntity* he = EntityPool::GetInst()->Get(id)->Cast2Human();
	if(!he) return;
	// ������� �������
	HumanContext::iterator medikit = he->GetEntityContext()->begin(HPK_ALL,
		TT_MEDIKIT);
	if(medikit != he->GetEntityContext()->end())
	{
		// ������� ����
		// ������� ��, ��� ����� � �����
		BaseThing* bth = he->GetEntityContext()->GetThingInHands();
		if(bth)
		{
			// � ����� ���-�� ����
			if(bth->GetInfo()->GetType() == TT_MEDIKIT)
			{
				// � ����� ������� - ��� ������
				return;
			}
			// � ����� �� ������� - ������ ��� � ������
			he->GetEntityContext()->RemoveThing(bth);
			he->GetEntityContext()->InsertThing(HPK_BACKPACK, bth);
		}
		// ������ � ����� ������ ��� - ��������� ���� �������
		MedikitThing* mt = static_cast<MedikitThing*>(&*medikit);
		if(!mt) return;
		he->GetEntityContext()->RemoveThing(mt);
		he->GetEntityContext()->InsertThing(HPK_HANDS, mt);
	}
}

// ������� �����
void AIAPI::cure(eid_t doctor, eid_t sick) // (human only)
{
	if( (!doctor) || (!sick) ) return;
	HumanEntity* doctor_ = EntityPool::GetInst()->Get(doctor)->Cast2Human();
	HumanEntity* sick_ = EntityPool::GetInst()->Get(sick)->Cast2Human();
	// ������� ��, ��� ����� � �����
	BaseThing* bth = doctor_->GetEntityContext()->GetThingInHands();
	if( (!bth) || (bth->GetInfo()->GetType() != TT_MEDIKIT) ) return;
	MedikitThing* mt = static_cast<MedikitThing*>(bth);

	AIUtils::MakeTreatment(doctor_, sick_, mt);
}

// ���� ��������� �������� ����� ����� ���� �����
// (human only)
bool AIAPI::canAttackByGrenadeMultipleTarget(eid_t im, eid_t enemy, const entity_list_t& enemies)
{
	if(!im) return 0;
	if(!enemy) return 0;
	if(BaseEntity* be = EntityPool::GetInst()->Get(im))
	{
		if(HumanEntity* he = be->Cast2Human())
		{
			BaseThing* bth = he->GetEntityContext()->GetThingInHands();
			if(!bth) return 0;
			if(bth->GetInfo()->GetType() != TT_GRENADE) return 0;
			GrenadeThing* grenade = static_cast<GrenadeThing*>(bth);
			float radius = grenade->GetInfo()->GetDmgRadius();
			ipnt2_t enemy_pos = EntityPool::GetInst()->Get(enemy)->GetGraph()->GetPos2();
			entity_list_t::const_iterator i = enemies.begin();
			entity_list_t::const_iterator end = enemies.end();
			while(i != end)
			{
				if(enemy == (*i)) { ++i; continue; }
				ipnt2_t tmp_pos = EntityPool::GetInst()->Get(*i)->GetGraph()->GetPos2();
				if(dist(tmp_pos, enemy_pos) < radius) return true;
				++i;
			}
		}
	}
	return 0;
}

// �������� ���������� ���������� �� ��������
point3 AIAPI::convertPos2ToPos3(const ipnt2_t& pnt)
{
	return HexGrid::GetInst()->Get(pnt);
}

// �������� ��������� �� ��������
BaseEntity* AIAPI::getPtr(eid_t id)
{
	if(id) return EntityPool::GetInst()->Get(id);
	return 0;
}

// ������� � ������� ���������
void AIAPI::print(eid_t id, const std::string& str)
{
	if(Console::IsAvailable())
	{
		BaseEntity* be = AIAPI::getPtr(id);
		std::ostringstream os;
		os << be->GetInfo()->GetName();
		os << "(" << id << ")";
		os << str;
		Console::AddString(os.str().c_str());
	}
}

//=====================================================================================//
//                               bool AIAPI::mayShoot()                                //
//=====================================================================================//
bool AIAPI::mayShoot(eid_t victim_id, eid_t killer_id)
{
	BaseEntity *victim = EntityPool::GetInst()->Get(victim_id);
	BaseEntity *killer = EntityPool::GetInst()->Get(killer_id);
	point3 victim_pos = victim->GetGraph()->GetPos3();
	point3 killer_pos = killer->GetGraph()->GetPos3();
	float range = AIUtils::dist2D(victim_pos, killer_pos);

	// ������� ��������� �������� ����� ������
	if(HumanEntity* im = killer->Cast2Human())
	{
		BaseThing* bth = im->GetEntityContext()->GetThingInHands(TT_WEAPON|TT_GRENADE);
		if(bth)
		{
			// ���� � ����� ������
			if(bth->GetInfo()->GetType() == TT_WEAPON)
			{
				WeaponThing* weapon = static_cast<WeaponThing*>(bth);
				if(weapon->GetAmmo())
				{
					if(weapon->GetAmmo()->GetInfo()->GetDamageFalloff() != 0.0f)
					{
						float damage = weapon->GetInfo()->GetBDmg().m_val + weapon->GetAmmo()->GetInfo()->GetBDmg().m_val;
						float dmg = AIUtils::CalcDamage(range,damage,weapon->GetAmmo()->GetInfo()->GetDamageFalloff());
						if(dmg < damage*im->GetInfo()->GetAiMinDamage()) return false;
					}

					if(AIUtils::CalcShootAccuracy(im,weapon,victim_pos) < im->GetInfo()->GetAiMinAccuracy()) return false;
				}
				else
				{
					return false;
				}
			}

			// ���� � ����� �������
			if(bth->GetInfo()->GetType() == TT_GRENADE)
			{
				GrenadeThing* weapon = static_cast<GrenadeThing*>(bth);
				float thrrange = std::max(AIUtils::CalcThrowRange(im)-1.0f,1.0f);
				if(range > thrrange) return false; 
			}
		}
	}

	return true;
}
