#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                             class MovieAct : public Act                             //
//=====================================================================================//
class MovieAct : public Act
{
	std::string m_bik;
	std::string m_levl;
	bool m_started;
	bool m_ended;
	bool m_firstTime;

public:
	MovieAct(const std::string &bik, const std::string &levl);

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() { m_ended = true; }
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_ended; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return true; }
};

}
