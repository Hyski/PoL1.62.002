#include "precomp.h"
#include "SpawnAct.h"
#include <logic2/GameObserver.h>
#include <logic2/EntityFactory.h>

namespace PoL
{

namespace SpawnActDetails
{

//=====================================================================================//
//                  class SpawnOnlyWithSysId : public ::Spawner::Rule                  //
//=====================================================================================//
class SpawnOnlyWithSysId : public ::Spawner::Rule
{
	std::string m_sysId;

public:
	SpawnOnlyWithSysId(const std::string &sysId)
	:	m_sysId(sysId)
	{
	}

	virtual bool IsSuitable(const SpawnTag& tag) const
	{
		return tag.GetSysName() != m_sysId;
	}

	virtual int CalcCount(const ::SpawnTag& tag, int xls_count)
	{
		return 0;
	}
};

}

using namespace SpawnActDetails;

//=====================================================================================//
//                                SpawnAct::SpawnAct()                                 //
//=====================================================================================//
SpawnAct::SpawnAct(const std::string &who)
:	m_who(who),
	m_started(false),
	m_ended(false)
{
}

//=====================================================================================//
//                               void SpawnAct::start()                                //
//=====================================================================================//
void SpawnAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                               void SpawnAct::update()                               //
//=====================================================================================//
void SpawnAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(!m_who.empty())
	{
		::EntityBuilder builder;
		::Spawner::RuleSet rules;

		SpawnOnlyWithSysId rule1(m_who);
		rules.Insert(&rule1);

		try
		{
			::Spawner::GetInst()->PolSpawn(&builder,&rules);
		}
		catch(SpawnError &e)
		{
			ACT_LOG("ERROR: Spawner reported error: " << e.what() << "\n");
			throw;
		}

		::GameEvMessenger::GetInst()->Notify( ::GameObserver::EV_UPDATE_UNLIMITED_VISIBILITY);
	}

	m_ended = true;
}

}
