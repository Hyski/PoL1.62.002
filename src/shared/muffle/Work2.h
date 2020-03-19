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

	/// ���������� true, ���� ������ ��� �� ��������
	virtual bool expired() const = 0;
	/// ��������� ��������
	virtual void execute() = 0;

	/// ���������� ���������� �������� Work
	static long getInstanceCount();
};

typedef Ptr<Work> HWork;
typedef WeakPtr<Work> WHWork;

}
