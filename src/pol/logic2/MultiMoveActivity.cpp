#include "logicdefs.h"

#include "ParMoveActivity.h"
#include "MultiMoveActivity.h"

namespace PoL
{

//=====================================================================================//
//                       MultiMoveActivity::MultiMoveActivity()                        //
//=====================================================================================//
MultiMoveActivity::MultiMoveActivity(const TrackInfos_t &tis)
:	m_infos(tis)
{
	m_activities.reserve(m_infos.size());

	for(TrackInfos_t::iterator i = m_infos.begin(); i != m_infos.end(); ++i)
	{
		ActivityInfo info = {new ParMoveActivity(this,*i), &i->m_path};
		m_activities.push_back(info.m_activity);
		m_actInfos.push_back(info);
	}
}

//=====================================================================================//
//                       MultiMoveActivity::~MultiMoveActivity()                       //
//=====================================================================================//
MultiMoveActivity::~MultiMoveActivity()
{
	for(Activities_t::iterator i = m_activities.begin(); i != m_activities.end(); ++i)
	{
		delete *i;
	}
}

//=====================================================================================//
//                            bool MultiMoveActivity::Run()                            //
//=====================================================================================//
bool MultiMoveActivity::Run(activity_command cmd)
{
	bool result = false;

	for(ActivityInfos_t::iterator i = m_actInfos.begin(); i != m_actInfos.end();)
	{
		const bool locresult = i->m_activity->Run(cmd);
		result = locresult || result;

		if(!locresult)
		{
			m_doneActInfos.push_back(*i);
			m_doneActivities.push_back(i->m_activity);
			i = m_actInfos.erase(i);
		}
		else
		{
			++i;
		}
	}

	return result;
}

//=====================================================================================//
//      MultiMoveActivity::ObstructorType MultiMoveActivity::getObstructorType()       //
//=====================================================================================//
MultiMoveActivity::ObsInfo_t MultiMoveActivity::getObstructorType(ParMoveActivity *act)
{
	ObsInfo_t result(otNothing,0);

	Activities_t::iterator i = std::find(m_activities.begin(),m_activities.end(),act);
	assert( i != m_activities.end() );
	size_t idx = i - m_activities.begin();
	ipnt2_t npos = act->getNextHexPos();

	for(size_t ii = 0; ii != m_activities.size(); ++ii)
	{
		ParMoveActivity *cact = m_activities[ii];
		if(cact == act) continue;

		if(cact->getCurHexPos() == npos)
		{
            if(std::find(m_doneActivities.begin(),m_doneActivities.end(),cact) != m_doneActivities.end())
			{
				result.first = otStopped;
			}
			else
			{
				result.first = otMoving;
			}

			result.second = cact;
			break;
		}
	}

	return result;
}

}