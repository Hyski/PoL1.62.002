#pragma once

#include "Condition.h"

namespace PoL
{

//=====================================================================================//
//                          class Signal : public Persistent                           //
//=====================================================================================//
class Signal : public Condition
{
public:
	Signal(Id_t id) : Condition(false,id) {}

protected:
	/// ��������� ���������� �������
	void notify()
	{
		setState(true);
		setState(false);
	};
};

}