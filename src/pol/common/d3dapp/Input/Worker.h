#pragma once

namespace PoL
{

//=====================================================================================//
//                                    class Worker                                     //
//=====================================================================================//
class Worker
{
protected:
	virtual ~Worker() {}

public:
	virtual void work() = 0;
};

}
