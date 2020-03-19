#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                         class Registry : boost::noncopyable                         //
//=====================================================================================//
/// �����, ������ ������������� �������
class Registry : boost::noncopyable
{
protected:
	Registry() {}
	virtual ~Registry() {}
	
public:
	/// ���������� ��������� �� ��������� �������
	static STORAGE_API Registry *instance();
	
	/// ���������������� ������������� �����
	virtual void registerClass(PersistentClass *) = 0;
	/// ���������� ������������� ����� �� �����
	virtual PersistentClass *getClass(const std::string &) const = 0;
};

}