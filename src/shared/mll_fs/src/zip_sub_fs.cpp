

#include "precomp.h"

#include "zip_file.h"

#include <mll/fs/zip_sub_fs_iter.h>
#include <mll/fs/zip_sub_fs.h>
#include <mll/fs/system.h>



using namespace mll::fs;
using namespace mll::ml_encrypted_zip;


//=====================================================================================//
//                              zip_sub_fs::zip_sub_fs()                               //
//=====================================================================================//
zip_sub_fs::zip_sub_fs(const std::string &real_path, const std::string &pass)
:	m_real_path(real_path)
{
	m_zip.reset(new zip_file(real_path, pass));
	m_real_path += "/";
}

//=====================================================================================//
//                              zip_sub_fs::~zip_sub_fs()                              //
//=====================================================================================//
zip_sub_fs::~zip_sub_fs()
{
}

//=====================================================================================//
//                  const std::string& zip_sub_fs::disk_path() const                   //
//=====================================================================================//
const std::string& zip_sub_fs::disk_path() const
{
	return m_real_path;
}

//=====================================================================================//
//                         bool zip_sub_fs::system_readonly()                          //
//=====================================================================================//
bool zip_sub_fs::system_readonly() const
{
	return true;
}

//=====================================================================================//
//                           bool zip_sub_fs::file_exists()                            //
//=====================================================================================//
bool zip_sub_fs::file_exists(const fs_path &name) const
{
	std::string _name(name.to_str());
	const zip_file::entries_t& entries = m_zip->get_entries(_name);
	return !entries.empty();
}

//=====================================================================================//
//                std::auto_ptr<sub_fs_iter> zip_sub_fs::new_iterator()                //
//=====================================================================================//
std::auto_ptr<sub_fs_iter> zip_sub_fs::new_iterator(const fs_path &path) const
{
	return std::auto_ptr<sub_fs_iter>(new zip_sub_fs_iter(m_zip.get(), path.to_str(), this));
}

//=====================================================================================//
//                          unsigned zip_sub_fs::file_size()                           //
//=====================================================================================//
unsigned zip_sub_fs::file_size(const fs_path &name) const
{
	std::string _name(name.to_str());
	const zip_file::entries_t& entries = m_zip->get_entries(_name);
	if (!entries.empty())
		return (*entries.begin())->size();
	return 0;
}

//=====================================================================================//
//                           bool zip_sub_fs::is_readonly()                            //
//=====================================================================================//
bool zip_sub_fs::is_readonly(const fs_path &name) const
{
	return true;
}

//=====================================================================================//
//                              time_t zip_sub_fs::date()                              //
//=====================================================================================//
time_t zip_sub_fs::date(const fs_path &name) const
{
	std::string _name(name.to_str());
	const zip_file::entries_t& entries = m_zip->get_entries(_name);
	if (!entries.empty())
		return (*entries.begin())->date();
	return 0;
}

//=====================================================================================//
//                              bool zip_sub_fs::is_dir()                              //
//=====================================================================================//
bool zip_sub_fs::is_dir(const fs_path &name) const
{
	std::string _name(name.to_str());
	const zip_file::entries_t& entries = m_zip->get_entries(_name);
	if (!entries.empty())
		return (*entries.begin())->is_dir();
	return false;
}

//=====================================================================================//
//                std::auto_ptr<std::istream> zip_sub_fs::r_open_file()                //
//=====================================================================================//
std::auto_ptr<std::istream> zip_sub_fs::r_open_file(const fs_path &path,
													bool seekable) const
{
	return m_zip->get_input_stream(path.to_str(), seekable);
}

//=====================================================================================//
//                std::auto_ptr<std::ostream> zip_sub_fs::w_open_file()                //
//=====================================================================================//
std::auto_ptr<std::ostream> zip_sub_fs::w_open_file(const fs_path &path, 
													bool append, bool seekable) const
{
	throw zip_restrictions("fs: can't write into archive");
}

//=====================================================================================//
//                           void zip_sub_fs::delete_file()                            //
//=====================================================================================//
void zip_sub_fs::delete_file(const fs_path &name) const
{
	throw zip_restrictions("fs: can't delete file in archive");
}

//=====================================================================================//
//                             void zip_sub_fs::make_dir()                             //
//=====================================================================================//
void zip_sub_fs::make_dir(const fs_path &path) const
{
	throw zip_restrictions("fs: can't make dirs in archive");
}