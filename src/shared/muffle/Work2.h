#pragma once

#include "Ptr2.h"
#include "Synchronized2.h"

namespace Muffle2
{

//=====================================================================================//
//                          class Work : public Synchronized                           //
//=====================================================================================//
class Work : public Synchronized
{
public:
	Work();
	~Work();

	/// Возвращает true, если объект уже не актуален
	virtual bool expired() const = 0;
	/// Выполнить действие
	virtual void execute() = 0;

	/// Возвращает количество объектов Work
	static long getInstanceCount();
};

typedef Ptr<Work> HWork;
typedef WeakPtr<Work> WHWork;

}
