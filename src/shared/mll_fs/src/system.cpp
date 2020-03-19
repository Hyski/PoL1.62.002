
#include "precomp.h"

#include "system_hidden.h"

#include <mll/fs/system.h>


using namespace mll;

fs::system *fs::system::m_instance = 0;


//=====================================================================================//
//                               void system::make_fs()                                //
//=====================================================================================//
void fs::system::make_fs()
{
	fs::system_hidden *tmp = new fs::system_hidden;
	assert(!m_instance);
	m_instance = tmp;
}


//=====================================================================================//
//                              void system::destroy_fs()                              //
//=====================================================================================//
void fs::system::destroy_fs()
{
	delete m_instance;
	m_instance = 0;
}


//=====================================================================================//
//                             system *system::instance()                              //
//=====================================================================================//
fs::system *fs::system::instance()
{
	assert(m_instance);
	return m_instance;
}
