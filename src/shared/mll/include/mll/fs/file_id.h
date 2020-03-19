#if !defined(__FILE_ID_H_INCLUDED_6215049273296640__)
#define __FILE_ID_H_INCLUDED_6215049273296640__


#pragma warning (disable : 4251)


#include "_export_rules.h"
#include "_choose_lib.h"
#include "fs_path.h"

#include <string>
#include <time.h>



namespace mll
{

namespace fs
{


class sub_fs;


//=====================================================================================//
//                                    class file_id                                    //
//=====================================================================================//
class FILE_SYSTEM_EXPORT file_id
{
	const sub_fs *m_sub_fs;
	fs_path m_name;

public:
	file_id(): m_sub_fs(0) {}
	file_id(const sub_fs *sfs, const std::string &name);
	file_id(const file_id &another);
	//~file_id() {}

	file_id& operator=(const file_id& another)
	{
		if(this != &another)
		{
			m_sub_fs = another.m_sub_fs;
			m_name = another.m_name;
		}
		return *this;
	}

public:
	operator const void * const () const { return m_sub_fs; }
	bool operator !() const { return m_sub_fs == 0; }

public:
	/// ��������� ��������� �� sub_fs, � �������� ����������� �������������
	const sub_fs *sub_fs() const { return m_sub_fs; }
	/// ������ ����������� ���� � ������
	std::string path() const;
	/// ����������� ����
	std::string dir() const;
	/// ��� �����
	std::string fname() const;
	/// ����������
	std::string ext() const;
	///	������ ��� � �����������
	std::string name() const;
	///	������ �������� ���� � ������
	std::string disk_path() const;
	/// �������� "������ ������"
	bool is_readonly() const;
	/// ������ ����� � ������
	unsigned file_size() const;
	/// ����� ���������� ��������� �����
	time_t date() const;
	/// ���������� �� ���
	bool is_dir() const;
};

}

}

#endif // !defined(__FILE_ID_H_INCLUDED_6215049273296640__)