
#include "precomp.h"

#include <mll/fs/hdd_sub_fs_iter.h>


using namespace mll::fs;


//=====================================================================================//
//                         hdd_sub_fs_iter::hdd_sub_fs_iter()                          //
//=====================================================================================//
hdd_sub_fs_iter::hdd_sub_fs_iter(const std::string& disk_path,
								 const std::string& rel_path,
								 const sub_fs *sfs)
:	m_search_path(disk_path + rel_path),
	m_rel_path(rel_path), 
	m_sub_fs(sfs)
{
	memset(&m_buf, 0, sizeof(_finddata_t));
	m_hFind = _findfirst(m_search_path.c_str(), &m_buf);
	if (m_hFind != -1L)
	{
		std::string forbidden(m_buf.name);
		if ((forbidden == ".") || (forbidden == ".."))
			++(*this);
	}
}


//=====================================================================================//
//                         hdd_sub_fs_iter::~hdd_sub_fs_iter()                         //
//=====================================================================================//
hdd_sub_fs_iter::~hdd_sub_fs_iter()
{
	if (m_hFind != -1L)
		_findclose(m_hFind);
}


//=====================================================================================//
//                        bool hdd_sub_fs_iter::is_done() const                        //
//=====================================================================================//
bool hdd_sub_fs_iter::is_done()
{
	if (m_hFind == -1L)
		return true;
	else
		return false;
}


//=====================================================================================//
//                          file_id hdd_sub_fs_iter::get_id()                          //
//=====================================================================================//
file_id hdd_sub_fs_iter::get_id()
{
	fs_path temp(m_rel_path);
	if (m_rel_path.length())
		temp.pop_back();
	return file_id(m_sub_fs, temp.to_str() + "/" + m_buf.name);
}


//=====================================================================================//
//                     sub_fs_iter &hdd_sub_fs_iter::operator++()                      //
//=====================================================================================//
sub_fs_iter &hdd_sub_fs_iter::operator++()
{
	std::string forbidden;
	do
	{
		if (_findnext(m_hFind, &m_buf) == -1L)
		{
			_findclose(m_hFind);
			m_hFind = -1L;
			return *this;
		}
		else
			forbidden = m_buf.name;
	}
	while ((forbidden == ".") || (forbidden == ".."));
	return *this;
}