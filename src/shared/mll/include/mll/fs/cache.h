#pragma once

#include "file_id.h"

#include <deque>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                     class cache                                     //
//=====================================================================================//
class FILE_SYSTEM_EXPORT cache
{
private:
	typedef std::deque<file_id> single_query_cache_t;
	single_query_cache_t m_single_query_data;

public:		//	����������� ������
	///	�������� ������ ���������� ������� �� �������� � ����� ������ (r_open, w_open, exists)
	void add_single_query(file_id id);
	///	���� ���������, ����� �������� ��� �� ���� �����
	void drop_file(file_id id);
	///	�������� ��� ���� (��������, ���������� ��������� ���������)
	void clear_all();

public:		//	��������� ������
	///	���������� ������� � ���� id, ����������� ����� �� ������ �� ����
	file_id single_query_stored(const std::string& path);
};

}

}

