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

	/// �������� ���������� �� ��������� ������� � ������������� �����
	Lock lock() const { return getWeakToken()->lock(); }

	/// ���������� ����� ��� �������� ������� � �������
	WeakToken *getWeakToken() const { return m_weakToken; }

	/// ���������� ���������� �������� Synchronized
	static long getInstanceCount();
};

}
