#include "precomp.h"

#include "ShootDieAct.h"
#include <logic2/Thing.h>

namespace PoL
{

//=====================================================================================//
//                               static bool canShoot()                                //
//=====================================================================================//
static bool canShoot(BaseEntity *entity)
{
	if(HumanEntity *hent = entity->Cast2Human())
	{
		BaseThing *tinh = hent->GetEntityContext()->GetThingInHands();
		if(tinh)
		{
			if(tinh->GetInfo()->GetType() == TT_GRENADE)
			{
				// Гранату не может кидать персонаж из команды игрока
				return entity->GetPlayer() != PT_PLAYER;
			}
			else if(tinh->GetInfo()->GetType() == TT_WEAPON)
			{
				WeaponThing *weap = (WeaponThing *)tinh;
				if(weap->GetAmmo())
				{
					return true;
				}
			}
		}

		return false;
	}
	else if(VehicleEntity *vent = entity->Cast2Vehicle())
	{
		return true;
	}

	return false;
}

//=====================================================================================//
//                             ShootDieAct::ShootDieAct()                              //
//=====================================================================================//
ShootDieAct::ShootDieAct(const std::string &who, const std::string &target,
						 bool shoot, bool die)
:	m_who(who),
	m_target(target),
	m_die(die),
	m_shoot(shoot),
	m_started(false),
	m_ended(false),
	m_shootCreated(false)
{
}

//=====================================================================================//
//                              void ShootDieAct::start()                              //
//=====================================================================================//
void ShootDieAct::start()
{
	if(m_shoot)
	{
		if(BaseEntity *who = getEntityByName(m_who))
		{
			if(BaseEntity *target = getEntityByName(m_target))
			{
				::point3 targpt = target->GetGraph()->GetShotPoint();

				if(!m_die)
				{
					::point3 delta(::aiNormRand()-0.5f,::aiNormRand()-0.5f,::aiNormRand()-0.5f);
					targpt += aiNormRand()*Normalize(delta);
				}

				if(canShoot(who))
				{
					if(who->Cast2Human()
						&& who->Cast2Human()->GetEntityContext()->GetThingInHands(TT_GRENADE))
					{
						m_shootact.reset( ::ActivityFactory::CreateThrow(
							who->Cast2Human(),
							::ActivityFactory::CT_PLAYER_GRENADE_THROW,
							::ActivityFactory::shoot_info_s(target->GetEID()),
							1.0f));
					}
					else
					{
						m_shootact.reset( ::ActivityFactory::CreateShoot(
							who, 
							//targpt, 
							1.0f, 
							::ActivityFactory::CT_PLAYER_SHOOT_ACT,
							ActivityFactory::shoot_info_s(target->GetEID())));
					}
				}
			}
		}
	}

	m_started = true;
}

//=====================================================================================//
//                             void ShootDieAct::update()                              //
//=====================================================================================//
void ShootDieAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(m_shootact.get() && !m_shootact->Run(::AC_TICK))
	{
		m_shootact.reset();
	}

	if(!m_shootact.get())
	{
		if(m_die)
		{
			BaseEntity *who = getEntityByName(m_who);
			BaseEntity *target = getEntityByName(m_target);
			if(target) target->GetEntityContext()->PlayDeath(who);
		}

		m_ended = true;
	}
}


}
