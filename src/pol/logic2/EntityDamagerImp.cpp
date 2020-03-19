#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "AIUtils.h"
#include "Graphic.h"
#include "Damager.h"
#include "DirtyLinks.h"
#include "GameObserver.h"

namespace EntityDamagerImpDetails
{

//=====================================================================================//
//                             float GetResistanceFactor()                             //
//=====================================================================================//
float GetResistanceFactor(int value)
{
    return 1.0f - static_cast<float>(value)/100.0f;
}

//=====================================================================================//
//                              void ApplyHealthChange()                               //
//=====================================================================================//
void ApplyHealthChange(HumanContext* human, int delta)
{   
    HumanContext::Traits* traits = human->GetTraits();
    HumanContext::Limits* limits = human->GetLimits();
    
    if(traits->GetHealth() != limits->GetHealth())
	{        
        float factor = static_cast<float>(delta)/static_cast<float>(limits->GetHealth());
        
        //traits->AddStrength(factor * limits->GetStrength());
        //traits->AddReaction(factor * limits->GetReaction());
        traits->AddAccuracy(factor * limits->GetAccuracy());
        traits->AddDexterity(factor * limits->GetDexterity());
        //traits->AddMechanics(factor * limits->GetMechanics());
        //traits->AddFrontRadius(factor * limits->GetFrontRadius());
    }
	else
	{
        
        //traits->AddStrength(limits->GetStrength());
        //traits->AddReaction(limits->GetReaction());
        traits->AddAccuracy(limits->GetAccuracy());
        traits->AddDexterity(limits->GetDexterity());
        //traits->AddMechanics(limits->GetMechanics());
        //traits->AddFrontRadius(limits->GetFrontRadius());
    }
}

//=====================================================================================//
//                             void ApplyBodyPartsDamage()                             //
//=====================================================================================//
void ApplyBodyPartsDamage(HumanContext* human, int delta)
{
    if(human->GetTraits()->GetHealth() == human->GetLimits()->GetHealth())
	{
        for(int k = 0; k < MAX_BODY_PARTS; k++)
		{
            body_parts_type bp = static_cast<body_parts_type>(k);
            human->SetBodyPartDamage(bp, 0);
        }

        return;
    }

    int bp_dmg = delta/MAX_BODY_PARTS;        
    if(bp_dmg == 0) bp_dmg = delta > 0 ? 1 : -1;
    
    for(int k = 0; k < MAX_BODY_PARTS; k++)
	{
        body_parts_type bp = static_cast<body_parts_type>(k);
        human->SetBodyPartDamage(bp, human->GetBodyPartDamage(bp) + bp_dmg);
    }
}  

//=====================================================================================//
//                                  class HumanDriver                                  //
//=====================================================================================//
// внутренний драйвер для расчета поражений
class HumanDriver
{
public:
	//проиграть эффект взрыва человека
	virtual void PlayMeat(HumanEntity* human) = 0;
	//проиграть эффект вылета крови
	virtual void PlayBlood(BaseEntity* human) = 0;
	//это спина человека?
	virtual bool IsBack(HumanEntity* human) const = 0;
	//узнать часть тела человека 
	virtual body_parts_type GenBodyPart(HumanEntity* human) const = 0;
};

//=====================================================================================//
//                     class UsualHumanDriver : public HumanDriver                     //
//=====================================================================================//
//драйвер для обычного повреждения
class UsualHumanDriver : public HumanDriver
{
	bool m_fblood;
	point3 m_dir;
	point3 m_from;

public:
	void Init(const point3& from, const point3& to, bool fblood)
	{
		m_dir = to - from;
		m_from = from;
		m_fblood = fblood;
	}

	bool IsBack(HumanEntity* human) const
	{
		float angle = human->GetGraph()->GetAngle() + PId2;
		point3 pnt(cosf(angle), sinf(angle), m_dir.z);
		return pnt.Dot(m_dir) < 0.0f;
	}

	body_parts_type GenBodyPart(HumanEntity* human) const
	{
		float prb = aiNormRand();

		const float head_prb  = PoL::Inv::HeadHitPrb;
		const float hands_prb = PoL::Inv::HandsHitPrb + head_prb;
		const float legs_prb  = PoL::Inv::LegsHitPrb + hands_prb;
		const float body_prb  = PoL::Inv::BodyHitPrb + legs_prb;

		if(prb <= head_prb) return BPT_HEAD;
		if(prb <= hands_prb) return BPT_HANDS;
		if(prb <= legs_prb) return BPT_LEGS;

		return BPT_BODY;
	}

	void PlayBlood(BaseEntity* human)
	{
		if(!m_fblood) return;

		point3 ent_pos = human->GetGraph()->GetPos3();
		ent_pos.z += 1.0f;

		EffectMgr::GetInst()->MakeHumanBlood(m_from, ent_pos);                
	}

	void PlayMeat(HumanEntity* human)
	{
		EffectMgr::GetInst()->MakeMeat(human->GetGraph(), m_from);
	}

	const point3 &GetDir() const { return m_dir; }
};

//=====================================================================================//
//                    class GroundHumanDriver : public HumanDriver                     //
//=====================================================================================//
//драйвер для повреждения с земли
class GroundHumanDriver : public HumanDriver
{
public:
	void PlayMeat(HumanEntity *) {}
	void PlayBlood(BaseEntity *) {}
	bool IsBack(HumanEntity *) const { return false; }
	body_parts_type GenBodyPart(HumanEntity *) const { return BPT_LEGS; }
};

//=====================================================================================//
//        class EntityDamagerImp : public EntityDamager, private EntityVisitor         //
//=====================================================================================//
// алгоритм повреждения существа
class EntityDamagerImp : public EntityDamager, private EntityVisitor
{
	damage_s m_dmg[hit_t::MAX_DMGS];
	std::vector<damage_s> m_takenDmg;
	unsigned int m_flags;
	BaseEntity *m_killer;
	float m_fall_off;

	HumanDriver *m_driver;

	UsualHumanDriver m_usual_drv;
	GroundHumanDriver m_ground_drv;

public:
	enum
	{
		FALL_DAMAGE     = 40,
		NONBLOOD_DAMAGE = 10,
	};

	EntityDamagerImp()
	:	m_killer(0),
		m_flags(0),
		m_fall_off(0.0f)
	{
	}

//=====================================================================================//
//                                    void Damage()                                    //
//=====================================================================================//
	void Damage(BaseEntity* actor, BaseEntity* victim, const hit_t& hit, unsigned flags)
	{
		//запомним парамеры
		memcpy(m_dmg, hit.m_dmg, sizeof(m_dmg));
		m_usual_drv.Init(hit.m_from, hit.m_to, m_flags & FT_MAKE_EFFECTS);

		m_driver = &m_usual_drv;

		m_fall_off = hit.m_fall_off;
		m_flags = flags;
		m_killer = actor;

		m_takenDmg.clear();

		//расчитаем повреждения
		victim->Accept(*this);
	}

//=====================================================================================//
//                                    void Damage()                                    //
//=====================================================================================//
	void Damage(BaseEntity* victim, RiskSite* site, unsigned flags = DEFAULT_FLAGS)
	{
		m_driver = &m_ground_drv;

		m_flags = flags;
		m_killer = 0;

		m_dmg[hit_t::BW_DMG].m_val = 0;
		m_dmg[hit_t::BW_DMG].m_type = DT_NONE;

		m_dmg[hit_t::AW_DMG].m_val = site->GetEntityFlameDmg();
		m_dmg[hit_t::AW_DMG].m_type =  DT_FLAME;

		m_dmg[hit_t::AA_DMG].m_val = 0;        
		m_dmg[hit_t::AA_DMG].m_type = DT_NONE;

		m_takenDmg.clear();

		//расчитаем повреждения
		victim->Accept(*this);
	}

private:
	void addDamage(damage_type type, int amount)
	{
		for(int i = 0; i < m_takenDmg.size(); ++i)
		{
			if(m_takenDmg[i].m_type == type)
			{
				m_takenDmg[i].m_val += amount;
				return;
			}
		}

		damage_s dmg;
		dmg.m_type = type;
		dmg.m_val = amount;
		m_takenDmg.push_back(dmg);
	}

//=====================================================================================//
//                                    void Visit()                                     //
//=====================================================================================//
	void Visit(HumanEntity* human)
	{
		if(human->GetEntityContext()->GetCrew()) return;
		HumanContext* context = human->GetEntityContext();

		int health = context->GetTraits()->GetHealth();

		//попали в спину?
		bool fback = m_driver->IsBack(human);

		//в какую чать тела попали?
		body_parts_type body_part = m_driver->GenBodyPart(human);

		int health_damage = 0;

		ApplyHumanArmorFactor(&m_dmg[hit_t::BW_DMG], context, body_part, fback);
		if(IsValid(m_dmg[hit_t::BW_DMG])) health_damage += ApplyBasicDamage(context, m_dmg[hit_t::BW_DMG]);

		ApplyHumanArmorFactor(&m_dmg[hit_t::AW_DMG], context, body_part, fback);
		if(IsValid(m_dmg[hit_t::AW_DMG])) health_damage += ApplyAdditionalDamage(context, m_dmg[hit_t::AW_DMG]);

		ApplyHumanArmorFactor(&m_dmg[hit_t::AA_DMG], context, body_part, fback);
		if(IsValid(m_dmg[hit_t::AA_DMG])) health_damage += ApplyAdditionalDamage(context, m_dmg[hit_t::AA_DMG]);

		//установим флаг взрывного повреждения
		const float explosionLimit = PoL::Inv::ExplosionDamageBase
			+ static_cast<float>(health_damage)*PoL::Inv::ExplosionDamageFactor;

		if(health_damage >= explosionLimit)
		{
			human->RaiseFlags(EA_EXPLOSIVE_DMG);
		}

		//проиграем повреждения
		if(m_flags & FT_PLAY_ENTITY_HURT && context->GetTraits()->GetHealth())
		{
			human->GetGraph()->ChangeAction(health_damage > FALL_DAMAGE ? GraphHuman::AT_FALL : GraphEntity::AT_HURT);
		}

		if(health != context->GetTraits()->GetHealth())
		{                                
			//записать повреждение части тела
			context->SetBodyPartDamage(body_part, health - context->GetTraits()->GetHealth());
			//учесть падение здровья
			ApplyHealthChange(context, context->GetTraits()->GetHealth() - health);
		}

		//разослать игровое сообщение
		SendGameHurtEvent(human);

		//распечатаем повреждения
		PrintDamage(human);

		// ~~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~            
		//human->Notify(EntityObserver::EV_CRITICAL_HURT);            
		// ~~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~

		if(IsCriticalHurt(body_part))
		{
			PrintCriticalHit(human->GetInfo()->GetName());
			human->Notify(EntityObserver::EV_CRITICAL_HURT);

			if(m_killer)
			{
				EntityObserver::entity_info_s info(human);
				m_killer->Notify(EntityObserver::EV_LUCKY_STRIKE, &info);
			}
		}

		//померло?
		if(m_flags & FT_PLAY_ENTITY_DEATH && !context->GetTraits()->GetHealth())
		{
			if(m_killer && human->GetEID() == m_killer->GetEID())
			{
				m_killer = 0;
			}

			if(human->IsRaised(EA_EXPLOSIVE_DMG))
			{
				m_driver->PlayMeat(human);
			}
			else
			{
				m_driver->PlayBlood(human);        
			}

			human->GetEntityContext()->PlayDeath(m_killer);
			return;
		} 

		if(IsBloodDmg(m_dmg[0]) || IsBloodDmg(m_dmg[1]) || IsBloodDmg(m_dmg[2]))
		{
			m_driver->PlayBlood(human);
		}
	}

//=====================================================================================//
//                                    void Visit()                                     //
//=====================================================================================//
	void Visit(TraderEntity* trader)
	{      
		SendGameHurtEvent(trader);
		PrintDamage(trader);
		m_driver->PlayBlood(trader);  

		if(m_flags & FT_PLAY_ENTITY_HURT) trader->GetGraph()->ChangeAction(GraphEntity::AT_HURT);
		if(m_flags & FT_PLAY_ENTITY_DEATH) trader->GetEntityContext()->PlayDeath(m_killer);
	}

//=====================================================================================//
//                                    void Visit()                                     //
//=====================================================================================//
	void Visit(VehicleEntity* vehicle)
	{
		m_dmg[1].m_type = DT_NONE;
		m_dmg[2].m_type = DT_NONE;

		if(IsValid(m_dmg[0]))
		{
			ApplyBasicDamage(vehicle->GetEntityContext(), m_dmg[0]);
			PrintDamage(vehicle);

			//обновим что нужно
			SendGameHurtEvent(vehicle);
		}

		//проиграем повреждения
		if(m_flags & FT_PLAY_ENTITY_HURT) vehicle->GetGraph()->ChangeAction(GraphEntity::AT_HURT);

		if(m_flags & FT_PLAY_ENTITY_DEATH && !vehicle->GetEntityContext()->GetHealth())
		{
			if(m_killer && vehicle->GetEID() == m_killer->GetEID()) m_killer = 0;

			hit_info_s hit;

			hit.m_dmg[hit_info_s::AW_DMG] = damage_s();
			hit.m_dmg[hit_info_s::AA_DMG] = damage_s();
			hit.m_dmg[hit_info_s::BW_DMG] = vehicle->GetInfo()->GetDeathDmg();

			point3 veh_pos = vehicle->GetGraph()->GetPos3();

			hit.m_from   = veh_pos;
			hit.m_to     = veh_pos;
			hit.m_radius = vehicle->GetInfo()->GetDeathRadius();

			hit.m_fall_off = 0.0f;

			//проиграть стратегию смерти техники
			vehicle->GetEntityContext()->PlayDeath(m_killer);
			//применить повреждения от смерти техники
			Damager::GetInst()->HandleLevelHit(m_killer, hit, m_flags);
			return;
		}
	}

//=====================================================================================//
//                               bool IsBloodDmg() const                               //
//=====================================================================================//
	bool IsBloodDmg(const damage_s& dmg) const
	{
		return (dmg.m_type == DT_STRIKE || dmg.m_type == DT_ENERGY) && dmg.m_val > NONBLOOD_DAMAGE;
	}

//=====================================================================================//
//                                bool IsValid() const                                 //
//=====================================================================================//
	bool IsValid(const damage_s& dmg) const
	{
		return dmg.m_val && dmg.m_type != DT_NONE;
	}

//=====================================================================================//
//                             bool IsCriticalHurt() const                             //
//=====================================================================================//
	bool IsCriticalHurt(body_parts_type bpt) const
	{
		return bpt == BPT_HEAD;
	}

//=====================================================================================//
//                                 void PrintDamage()                                  //
//=====================================================================================//
	void PrintDamage(BaseEntity* entity)
	{        
		if(m_flags & FT_PRINT_MESSAGES)
		{
			AIUtils::DisplayDamages(entity, &m_takenDmg[0], m_takenDmg.size(), false);
		}
	}

//=====================================================================================//
//                               void PrintCriticalHit()                               //
//=====================================================================================//
	void PrintCriticalHit(const std::string& hero_name)
	{
		if(m_flags & FT_PRINT_MESSAGES)
		{
			DirtyLinks::Print(mlprintf(DirtyLinks::GetStrRes("head_shot").c_str(), hero_name.c_str()));
		}
	}

//=====================================================================================//
//                            void ApplyHumanArmorFactor()                             //
//=====================================================================================//
	void ApplyHumanArmorFactor(damage_s* dmg, HumanContext* human, body_parts_type bp, bool fback)
	{
		static ArmorThing defaultArmor("base");

		float basic_damage = dmg->m_val;
		float armor_factor = 1.0f;

		HumanContext::iterator itor = human->begin(HPK_BODY, TT_ARMOUR);
		// Учтем броник на теле
		ArmorThing *armor = static_cast<ArmorThing*>(itor != human->end() ? &*itor : &defaultArmor);
		if(float factor = armor->GetAbsorber(dmg->m_type,bp,fback)) armor_factor *= factor;

		// Учтем броник на голове
		itor = human->begin(HPK_HEAD, TT_ARMOUR);
		if(itor != human->end())
		{
			armor = static_cast<ArmorThing*>(&*itor);
			if(float factor = armor->GetAbsorber(dmg->m_type, bp, fback)) armor_factor *= factor;
		}

		float fall_off = m_fall_off;
		float dist = 0.0f;

		if(m_driver == &m_usual_drv)
		{
			dist = m_usual_drv.GetDir().Length();
		}

		float real_damage = AIUtils::CalcDamage(dist,basic_damage,fall_off) * armor_factor;
		dmg->m_val = static_cast<int>(saturate(real_damage,0.0f,1e+10f));
	}

//=====================================================================================//
//                               int ApplyBasicDamage()                                //
//=====================================================================================//
	int ApplyBasicDamage(HumanContext* human, const damage_s& dmg)
	{
		if(m_flags & FT_MAKE_DAMAGES)
		{
			human->GetTraits()->AddHealth(-dmg.m_val);
			addDamage(dmg.m_type,dmg.m_val);
		}
		return dmg.m_val;
	}

//=====================================================================================//
//                               int ApplyBasicDamage()                                //
//=====================================================================================//
	int ApplyBasicDamage(VehicleContext* vehicle, const damage_s& dmg)
	{
		if(m_flags & FT_MAKE_DAMAGES)
		{
			const float dist = (m_driver == &m_usual_drv ? m_usual_drv.GetDir().Length() : 0.0f);
			const float falloff = m_fall_off;
			const float real_damage = AIUtils::CalcDamage(dist,dmg.m_val,falloff);
			const int val = static_cast<int>(saturate(real_damage,0.0f,1e+10f));
			vehicle->SetHealth(vehicle->GetHealth() - val);
			addDamage(dmg.m_type, val);
		}
		return dmg.m_val;
	}

//=====================================================================================//
//                             int ApplyAdditionalDamage()                             //
//=====================================================================================//
	int ApplyAdditionalDamage(HumanContext* human, const damage_s& dmg)
	{
		if(!(m_flags & FT_MAKE_DAMAGES))
		{
			return 0;
		}

		switch(dmg.m_type)
		{
		case DT_STRIKE:
		case DT_ENERGY:
		case DT_EXPLOSIVE:
			human->GetTraits()->AddHealth(-dmg.m_val);
			addDamage(dmg.m_type, dmg.m_val);
			return dmg.m_val;

		case DT_SHOCK:
			{
				int dmg_val = dmg.m_val * GetResistanceFactor(human->GetTraits()->GetShockRes());
				human->AddLongDamage(DT_SHOCK, dmg_val);
				human->GetTraits()->PoLAddMovepnts(-dmg_val);
				addDamage(dmg.m_type, dmg_val);
			}
			return 0;

		case DT_ELECTRIC:
			{
				int dmg_val = dmg.m_val * GetResistanceFactor(human->GetTraits()->GetElectricRes());
				human->AddLongDamage(DT_ELECTRIC, dmg_val);
				human->GetTraits()->PoLAddAccuracy(-dmg_val);
				human->GetTraits()->PoLAddDexterity(-dmg_val);
				addDamage(dmg.m_type, dmg_val);
			}
			return 0;

		case DT_FLAME:
			{
				int dmg_val = dmg.m_val * GetResistanceFactor(human->GetTraits()->GetFlameRes());
				human->AddLongDamage(DT_FLAME, dmg_val);
				human->GetTraits()->AddHealth(-dmg_val);            
				addDamage(dmg.m_type, dmg_val);
			}
			return 0;
		}    

		return 0;
	}

//=====================================================================================//
//                              void SendGameHurtEvent()                               //
//=====================================================================================//
	void SendGameHurtEvent(BaseEntity* victim)
	{
		if(m_flags & FT_SEND_GAME_EVENTS)
		{
			GameObserver::hurt_info info(m_killer, victim);                
			GameEvMessenger::GetInst()->Notify(GameObserver::EV_HURT, &info);            
		}
	}
};

}

using namespace EntityDamagerImpDetails;

//=====================================================================================//
//                       EntityDamager* EntityDamager::GetInst()                       //
//=====================================================================================//
EntityDamager* EntityDamager::GetInst()
{
	static EntityDamagerImp imp;
	return &imp;
}
