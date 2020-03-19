#pragma warning (disable: 4786)


#include "precomp.h"

#include "zip_file.h"

#include <mll/fs/zip_sub_fs_iter.h>


using namespace mll::fs;
using namespace mll::ml_encrypted_zip;



//=====================================================================================//
//                         zip_sub_fs_iter::zip_sub_fs_iter()                          //
//=====================================================================================//
zip_sub_fs_iter::zip_sub_fs_iter(zip_file *file,
								 const std::string &rel_path, const mll::fs::sub_fs *sfs)
:	m_sfs(sfs), m_zip(file)
{
	const zip_file::entries_t& entries = m_zip->get_entries(rel_path);
	for (zip_file::entries_t::const_iterator i = entries.begin(); i != entries.end(); ++i)
		m_files.push_back(file_id(m_sfs, (*i)->filepath()));
	m_it = m_files.begin();
}


//=====================================================================================//
//                           bool zip_sub_fs_iter::is_done()                           //
//=====================================================================================//
bool zip_sub_fs_iter::is_done()
{
	if (m_it != m_files.end())
		return false;
	return true;
}


//=====================================================================================//
//                     sub_fs_iter &zip_sub_fs_iter::operator++()                      //
//=====================================================================================//
sub_fs_iter &zip_sub_fs_iter::operator++()
{
	++m_it;
	return *this;
}


//=====================================================================================//
//                          file_id zip_sub_fs_iter::get_id()                          //
//=====================================================================================//
file_id zip_sub_fs_iter::get_id()
{
	return file_id(*m_it);
}