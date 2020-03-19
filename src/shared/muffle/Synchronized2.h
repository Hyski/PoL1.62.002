#pragma once

#include "Lock2.h"
#include "WeakToken2.h"

namespace Muffle2
{

//=====================================================================================//
//                                 class Synchronized                                  //
//=====================================================================================//
class Synchronized
{
	WeakToken *m_weakToken;

	Synchronized(const Synchronized &);
	Synchronized &operator=(const Synchronized &);

public:
	Synchronized();
	virtual ~Synchronized();

	/// Получить полномочия на изменение объекта в многопоточной среде
	Lock lock() const { return getWeakToken()->lock(); }

	/// Возвращает токен для контроля доступа к объекту
	WeakToken *getWeakToken() const { return m_weakToken; }

	/// Возвращает количество объектов Synchronized
	static long getInstanceCount();
};

}
