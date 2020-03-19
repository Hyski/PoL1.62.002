#pragma once

#include "StoreBox2.h"

namespace PoL
{

//=====================================================================================//
//                                    class Bureau2                                    //
//=====================================================================================//
class Bureau2
{
	typedef std::map<std::string,HStoreBox2> Boxes_t;
	Boxes_t m_boxes;

public:
	Bureau2();
	~Bureau2();

	/// �������� ���� � ����
	void addBox(HStoreBox2);
	/// ���������� ���� � ������ ��������� ������ fullId
	HStoreBox2 getBox(const std::string &fullId);
	/// ���������� ���� �� ����� ���� � ����
	bool hasBox(const std::string &fullId) const;
	/// ���������� ���� � ��������� ������ id
	HStoreBox2 getBoxEx(const std::string &id);

	/// ��������� ��������� ������� � �����
	void store(mll::io::obinstream &) const;
	/// ������������ ��������� ������� �� ������
	void restore(mll::io::ibinstream &);

	/// ���������� ��������� Bureau2
	static Bureau2 &inst();
};

}
