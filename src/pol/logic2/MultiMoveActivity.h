#pragma once

#include "Graphic.h"
#include "BaseActivity.h"
#include "ActivityStatusSender.h"

namespace PoL
{

class ParMoveActivity;

//=====================================================================================//
//                                  struct TrackInfo                                   //
//=====================================================================================//
struct TrackInfo
{
	BaseEntity *m_entity;
	pnt_vec_t m_path;
	Driver *m_driver;
};

//=====================================================================================//
//                               class MultiMoveActivity                               //
//=====================================================================================//
/// Передвижение нескольких персонажей
class MultiMoveActivity : public Activity
{
	struct ActivityInfo
	{
		ParMoveActivity *m_activity;
		pnt_vec_t *m_path;
	};

	typedef std::vector<TrackInfo> TrackInfos_t;
	typedef std::vector<ActivityInfo> ActivityInfos_t;
	typedef std::vector<ParMoveActivity *> Activities_t;

	TrackInfos_t m_infos;
	Activities_t m_activities, m_doneActivities;
	ActivityInfos_t m_actInfos, m_doneActInfos;

public:
	MultiMoveActivity(const std::vector<TrackInfo> &);
    ~MultiMoveActivity();

	enum ObstructorType
	{
		otNothing,
		otMoving,
		otStopped
	};

	typedef std::pair<ObstructorType,ParMoveActivity*> ObsInfo_t;

	bool Run(activity_command cmd);
	ObsInfo_t getObstructorType(ParMoveActivity *act);
	
private:
	void remove(Activities_t::iterator);
};

}
