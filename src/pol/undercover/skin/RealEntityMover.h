#pragma once

#include "BSpline.h"
#include "../character.h"
#include <logic2/logictypes.h>
#include <boost/weak_ptr.hpp>

class RealEntity;

namespace PoL
{

//=====================================================================================//
//                              class MovementController                               //
//=====================================================================================//
class MovementController
{
public:
	typedef boost::shared_ptr<int> LifeToken_t;
	typedef boost::weak_ptr<int> LifeTokenCtrl_t;

protected:
	MovementController() : m_lifeToken(new int) {}
	virtual ~MovementController() {}

private:
	LifeToken_t m_lifeToken;

public:
	enum Command
	{
		cmdStop,
		cmdContinue
	};

	enum MotionCommand
	{
		mcNothing,
		mcPause,
		mcUnpause,
		mcStop
	};

	virtual bool needSpeedup() = 0;

	virtual MotionCommand canMoveToNextHex() { return mcNothing; }
	virtual Command onStep() = 0;
	virtual void onEnd() = 0;

	LifeTokenCtrl_t getLifeTokenCtrl() { return m_lifeToken; }
};

//=====================================================================================//
//                                class RealEntityMover                                //
//=====================================================================================//
class RealEntityMover
{
	typedef BSpline3 Spline_t;
	typedef std::vector<float> HexCentres_t;
	typedef std::vector<float> HexStarts_t;

	static const size_t WrongIndex = 0xFFFFFFFF;

	size_t m_lastHex;
	size_t m_targetHex;

	RealEntity *m_ent;
	pnt_vec_t m_path;
	Spline_t m_spline;
	MovementController *m_ctrl;
	float m_speed;

	float m_startTime;
	float m_lastUpdate;
	float m_initAngle;
	point3 m_initPos;

	bool m_needSpeedUp;
	bool m_splineReady;
	HexStarts_t m_hexStarts;
	HexCentres_t m_hexCentres;

	bool m_paused;

	MovementController::LifeTokenCtrl_t m_lifeCtrl;

public:
	/// Конструктор
	RealEntityMover(ANIMATION, RealEntity *ent, const pnt_vec_t &path, MovementController *);
	~RealEntityMover();

	/// Обновить сущность
	bool update(float tau);

	/// Изменить путь
	void repath(const pnt_vec_t &path);

private:
	bool isEnded() const;
	bool canMoveFast() const;
	bool moveFast();
	bool moveNormal(float tau);

	void setLoc(size_t);
	void placeOnSpline(float dist);
	float calcAngle(float dist);
	float angleFromVector(const point3 &vec);

	MovementController::Command doVisitHex();

	const Spline_t &getSpline();
	void prepareSpline();
	float getHexCenter(size_t);
	float getHexStart(size_t);

	size_t getHexFromDist(float dist);
};

}
