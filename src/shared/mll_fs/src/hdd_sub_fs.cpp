
#include "precomp.h"

#include <mll/fs/hdd_sub_fs_iter.h>
#include <mll/fs/hdd_sub_fs.h>
#include <mll/fs/system.h>

#include <fstream>
#include <direct.h>
#include <io.h>


using namespace mll::fs;


//=====================================================================================//
//                              hdd_sub_fs::hdd_sub_fs()                               //
//=====================================================================================//
hdd_sub_fs::hdd_sub_fs(const std::string& real_path)
:	m_real_path(real_path)
{
	std::use_facet<std::ctype<char> >(std::locale()).tolower(&m_real_path[0], &m_real_path[m_real_path.size()]);
	std::replace(m_real_path.begin(), m_real_path.end(), '\\', '/');
	//	если сзади нет слеша - добавим
	if (m_real_path[m_real_path.length() - 1] != '/')
		m_real_path.append(std::string("/"));
}


//=====================================================================================//
//                         std::string hdd_sub_fs::translate()                         //
//=====================================================================================//
std::string hdd_sub_fs::translate(const fs_path &path) const
{
	return m_real_path + path.to_str();
}


//=====================================================================================//
//                  const std::string& hdd_sub_fs::disk_path() const                   //
//=====================================================================================//
const std::string& hdd_sub_fs::disk_path() const
{
	return m_real_path;
}


//=====================================================================================//
//                      bool hdd_sub_fs::system_readonly() const                       //
//=====================================================================================//
bool hdd_sub_fs::system_readonly() const
{
	return false;
}


//=====================================================================================//
//                        bool hdd_sub_fs::file_exists() const                         //
//=====================================================================================//
bool hdd_sub_fs::file_exists(const fs_path &path) const
{
	bool flag;
	_finddata_t buf;
	long hFind = _findfirst(translate(path).c_str(), &buf);
	if (hFind == -1L)
		flag = false;
	else
		flag = true;
	_findclose(hFind);
	return flag;
}


//=====================================================================================//
//                std::auto_ptr<sub_fs_iter> hdd_sub_fs::new_iterator()                //
//=====================================================================================//
std::auto_ptr<sub_fs_iter> hdd_sub_fs::new_iterator(const fs_path &path) const
{
	return std::auto_ptr<sub_fs_iter>(new hdd_sub_fs_iter(m_real_path, path.to_str(), this));
}


//=====================================================================================//
//                         iunsigned hdd_sub_fs::file_size()                           //
//=====================================================================================//
unsigned hdd_sub_fs::file_size(const fs_path &path) const
{
	unsigned result;
	_finddata_t buf;
	long hFind = _findfirst(translate(path).c_str(), &buf);
	if (hFind != -1L)
	{
		result = buf.size;
	}
	_findclose(hFind);
	return result;
}


//=====================================================================================//
//                           bool hdd_sub_fs::is_readonly()                            //
//=====================================================================================//
bool hdd_sub_fs::is_readonly(const fs_path &path) const
{
	bool result;
	_finddata_t buf;
	long hFind = _findfirst(translate(path).c_str(), &buf);
	if (hFind != -1L)
	{
		if (buf.attrib & _A_RDONLY)
			result = true;
		else
			result = false;
	}
	_findclose(hFind);
	return result;
}


//=====================================================================================//
//                              time_t hdd_sub_fs::date()                              //
//=====================================================================================//
time_t hdd_sub_fs::date(const fs_path &path) const
{
	time_t result;
	_finddata_t buf;
	long hFind = _findfirst(translate(path).c_str(), &buf);
	if (hFind != -1L)
	{
		result = buf.time_write;
	}
	_findclose(hFind);
	return result;
}


//=====================================================================================//
//                           bool hdd_sub_fs::is_dir() const                           //
//=====================================================================================//
bool hdd_sub_fs::is_dir(const fs_path &path) const
{
	bool result;
	_finddata_t buf;
	long hFind = _findfirst(translate(path).c_str(), &buf);
	if (hFind != -1L)
	{
		if (buf.attrib & _A_SUBDIR)
			result = true;
		else
			result = false;
	}
	_findclose(hFind);
	return result;
}


//=====================================================================================//
//                std::auto_ptr<std::istream> hdd_sub_fs::r_open_file()                //
//=====================================================================================//
std::auto_ptr<std::istream> hdd_sub_fs::r_open_file(const fs_path &path, bool seekable) const
{
	const std::string name = translate(path);
	std::auto_ptr<std::istream> tmp(new std::ifstream(name.c_str(), std::ios::binary));
	if (!tmp->good())
	{
		throw stream_fail("fs: can't create input file stream");
	}
	return tmp;
}


//=====================================================================================//
//                std::auto_ptr<std::ostream> hdd_sub_fs::w_open_file()                //
//=====================================================================================//
std::auto_ptr<std::ostream> hdd_sub_fs::w_open_file(const fs_path &path, 
													bool append, bool seekable) const
{
	std::auto_ptr<std::ostream> tmp;
	if (append)
		tmp.reset(new std::ofstream(translate(path).c_str(), std::ios::binary|std::ios::app));
	else
		tmp.reset(new std::ofstream(translate(path).c_str(), std::ios::binary));
	if (!tmp->good())
	{
		throw stream_fail("fs: can't create output file stream");
	}
	return tmp;
}


//=====================================================================================//
//                           void hdd_sub_fs::delete_file()                            //
//=====================================================================================//
void hdd_sub_fs::delete_file(const fs_path &path) const
{
	if (is_dir(path))
		_rmdir(translate(path).c_str());
	else
		remove(translate(path).c_str());
}


//=====================================================================================//
//                             void hdd_sub_fs::make_dir()                             //
//=====================================================================================//
void hdd_sub_fs::make_dir(const fs_path &path) const
{
	_mkdir(translate(path).c_str());
}