
#include "precomp.h"

#include <mll/fs/sub_fs.h>

namespace mll
{

using namespace fs;

//=====================================================================================//
//                                 file_id::file_id()                                  //
//=====================================================================================//
file_id::file_id(const fs::sub_fs *sfs, const std::string &name)
:	m_sub_fs(sfs), m_name(name)
{
}


//=====================================================================================//
//                                 file_id::file_id()                                  //
//=====================================================================================//
file_id::file_id(const file_id &another)
:	m_sub_fs(another.m_sub_fs), m_name(another.m_name)
{
}


//=====================================================================================//
//                          std::string file_id::path() const                          //
//=====================================================================================//
std::string file_id::path() const
{
	return m_name.to_str();
}
 

//=====================================================================================//
//                          std::string file_id::dir() const                           //
//=====================================================================================//
std::string file_id::dir() const
{
	std::string tmp = m_name.to_str();
	unsigned n = tmp.rfind('/');
	return tmp.substr(0, n+1);
}


//=====================================================================================//
//                         std::string file_id::fname() const                          //
//=====================================================================================//
std::string file_id::fname() const
{
	std::string tmp = m_name.to_str();
	int n = tmp.rfind('/');
	const std::string full_name = tmp.substr(n+1);
	n = full_name.find('.');
	if (n != -1)
		return full_name.substr(0, n);
	else
		return full_name;
	return "";
}


//=====================================================================================//
//                          std::string file_id::ext() const                           //
//=====================================================================================//
std::string file_id::ext() const
{
	std::string tmp = m_name.to_str();
	int n = tmp.rfind('/');
	const std::string full_name = tmp.substr(n+1);
	n = full_name.find('.');
	if (n != -1)
		return full_name.substr(n+1);
	else
		return "";
}


//=====================================================================================//
//                          std::string file_id::name() const                          //
//=====================================================================================//
std::string file_id::name() const
{
	std::string tmp = m_name.to_str();
	unsigned n = tmp.rfind('/');
	return tmp.substr(n+1);
}


//=====================================================================================//
//                       std::string file_id::disk_path() const                        //
//=====================================================================================//
std::string file_id::disk_path() const
{
	std::string tmp;
	if (m_sub_fs)
		tmp += m_sub_fs->disk_path();
	//	слеш добавляет подсистема
//	tmp += "/";
	tmp += m_name.to_str();
	return tmp;
}


//=====================================================================================//
//                          bool file_id::is_readonly() const                          //
//=====================================================================================//
bool file_id::is_readonly() const
{
	if (m_sub_fs)
		return m_sub_fs->is_readonly(path());
	else
		return true;
}


//=====================================================================================//
//                         unsigned file_id::file_size() const                         //
//=====================================================================================//
unsigned file_id::file_size() const
{
	if (m_sub_fs)
		return m_sub_fs->file_size(path());
	else
		return 0;
}


//=====================================================================================//
//                            time_t file_id::date() const                             //
//=====================================================================================//
time_t file_id::date() const
{
	if (m_sub_fs)
		return m_sub_fs->date(path());
	else
		return 0;
}


//=====================================================================================//
//                            bool file_id::is_dir() const                             //
//=====================================================================================//
bool file_id::is_dir() const
{
	if (m_sub_fs)
		return m_sub_fs->is_dir(path());
	else
		return true;
}

}