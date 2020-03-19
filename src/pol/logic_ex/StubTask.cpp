#include "precomp.h"
#include "StubTask.h"

namespace PoL
{

//=====================================================================================//
//                                StubTask::StubTask()                                 //
//=====================================================================================//
StubTask::StubTask(Id_t id, TaskState ts)
:	Task(id)
{
	setState(ts);
}

}