
#include "precomp.h"

#include <mll/fs/fs_guider.h>
#include <mll/fs/directory.h>


using namespace mll::fs;


//=====================================================================================//
//                               fs_guider::fs_guider()                                //
//=====================================================================================//
fs_guider::fs_guider(const fs_path &path)
:	m_path(path)
{
	m_itor = system::instance()->begin();
	m_dir = m_path.begin();
}


//=====================================================================================//
//                               fs_guider::fs_guider()                                //
//=====================================================================================//
fs_guider::fs_guider(const fs_guider &another)
:	m_itor(another.m_itor), m_path(another.m_path), m_dir(another.m_dir)
{
}


//=====================================================================================//
//                              void fs_guider::_search()                              //
//=====================================================================================//
void fs_guider::_search()
{
	for(; m_itor != system::instance()->end() && (*m_itor)->name() != *m_dir; m_itor.brother());
	if(m_itor != system::instance()->end())
		++m_dir;
}


//=====================================================================================//
//                               void fs_guider::next()                                //
//=====================================================================================//
void fs_guider::next()
{
	m_itor.son();
	_search();
}