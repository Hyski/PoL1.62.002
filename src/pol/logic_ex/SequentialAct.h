#pragma once

#include "ComplexAct.h"

namespace PoL
{

//=====================================================================================//
//                       class SequentialAct : public ComplexAct                       //
//=====================================================================================//
class SequentialAct : public ComplexAct
{
	bool m_started;
	bool m_endable;
	Acts_t::iterator m_currentAct;

public:
	template<typename It> SequentialAct(It begin, It end);

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip();
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_currentAct == actsEnd(); }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return m_endable; }

private:
	/// Рассчитать возможность окончания действия
	bool calcEndable() const;
	/// Валидно ли действие
	bool isValid() const;
};

//=====================================================================================//
//                        inline SequentialAct::SequentialAct()                        //
//=====================================================================================//
template<typename It>
inline SequentialAct::SequentialAct(It begin, It end)
:	ComplexAct(begin,end),
	m_started(false),
	m_endable(calcEndable()),
	m_currentAct(actsBegin())
{
}

}