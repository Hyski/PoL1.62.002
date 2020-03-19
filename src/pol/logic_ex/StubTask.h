#pragma once

#include "Task.h"

namespace PoL
{

//=====================================================================================//
//                            class StubTask : public Task                             //
//=====================================================================================//
class StubTask : public Task
{
public:
	StubTask(Id_t id, TaskState ts);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stStubTask; }

	virtual void store(mll::io::obinstream &out) const { Task::store(out); }
	virtual void restore(mll::io::ibinstream &in) { Task::restore(in); }
	virtual void reset() { Task::reset(); }
};

}