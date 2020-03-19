#pragma once

#include "ComplexAct.h"

namespace PoL
{

//=====================================================================================//
//                        class ParallelAct : public ComplexAct                        //
//=====================================================================================//
class ParallelAct : public ComplexAct
{
	bool m_started;
	Acts_t::iterator m_lastAct;
	size_t m_endedCount;
	size_t m_endableCount;

public:
	template<typename It> ParallelAct(It,It);

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip();
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_endableCount && m_endedCount == m_endableCount; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return m_endableCount != 0; }

private:
	/// Возвращает количество завершаемых актов
	size_t calcEndable() const;
};

//=====================================================================================//
//                          inline ParallelAct::ParallelAct()                          //
//=====================================================================================//
template<typename It>
inline ParallelAct::ParallelAct(It begin,It end)
:	ComplexAct(begin,end),
	m_lastAct(actsEnd()),
	m_endedCount(0),
	m_started(false),
	m_endableCount(calcEndable())
{
}

}