#pragma once

#include "Entity.h"
#include "Graphic.h"
#include "EntityRef.h"
#include "BaseActivity.h"
#include <undercover/skin/RealEntityMover.h>

namespace PoL
{

struct TrackInfo;

class MultiMoveActivity;
class ParMoveActivityState;

//=====================================================================================//
//                     class ParMoveActivity : public BaseActivity                     //
//=====================================================================================//
class ParMoveActivity : public BaseActivity, private PoL::MovementController
{
	enum State
	{
		sInit,
		sMoving,
		sStopping,
		sEnded,
		sCount
	};

	typedef bool (ParMoveActivity::*OnRun_t)(activity_command);
	typedef Command (ParMoveActivity::*OnStep_t)();

	static OnRun_t m_onRun[];
	static OnStep_t m_onStep[];

	MultiMoveActivity *m_mmact;

	PoL::BaseEntityRef m_entity;
    const pnt_vec_t &m_path;
	ipnt2_t m_initPos;

	bool m_result;

    State m_state;
	bool m_unlinked;

	size_t m_curHex;
	bool m_paused;
	ParMoveActivity *m_blocker;

public:
	ParMoveActivity(MultiMoveActivity *mmact, const TrackInfo &info);
	~ParMoveActivity();

	virtual bool Run(activity_command cmd);
	virtual Command onStep();
	virtual void onEnd();
	virtual MotionCommand canMoveToNextHex();

	size_t getCurHex() const { return m_curHex; }
	const pnt_vec_t &getPath() const { return m_path; }
	const ipnt2_t &getCurHexPos() const;
	const ipnt2_t &getNextHexPos() const;
	bool hasNextPos() const { return m_curHex == m_path.size(); }

private:
	bool runInit(activity_command);
	bool runMoving(activity_command);
	bool runStopping(activity_command);
	bool runEnded(activity_command);

	Command onStepInit();
	Command onStepMoving();
	Command onStepStopping();
	Command onStepEnded();

	bool needSpeedup() { return false; }

	void link();
	void unlink();

	bool isCircularDependency();
};

}

