#include "precomp.h"
#include "RealFile.h"
//#include <xdebug>
#include <io.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

namespace Muffle
{

namespace
{
//=====================================================================================//
//                                    bool exists()                                    //
//=====================================================================================//
	bool exists(const char *name)
	{
		_finddata_t fd;
		long hfind = _findfirst(name,&fd);
		if(hfind != -1)
		{
			_findclose(hfind);
			return true;
		}
		else return false;
	}
}

//=====================================================================================//
//                                RealFile::RealFile()                                 //
//=====================================================================================//
RealFile::RealFile(ci_VFileSystem *vfs, const std::string &name)
:	m_name(name),
	m_refCtr(0),
	m_file(0)
{
	if(!exists(name.c_str()))
	{
		m_file = vfs->openFile(name.c_str());
		if(m_file == 0) throw mll::debug::exception("File not found: " + name);
		if(m_file->size()==0)
		{
			m_file->release();
			throw mll::debug::exception("File not found: " + name);
		}
		m_length = m_file->size();
	}
	else
	{
		struct _stat st;
		_stat(name.c_str(),&st);
		m_length = st.st_size;
	}
}

//=====================================================================================//
//                                RealFile::~RealFile()                                //
//=====================================================================================//
RealFile::~RealFile()
{
	assert( is_lowercase(m_name) );
	if(m_file)
	{
		m_file->release();
		m_file = 0;
	}
}

//=====================================================================================//
//                           bool RealFile::canCreateFile()                            //
//=====================================================================================//
bool RealFile::canCreateFile(ci_VFileSystem *vfs, const std::string &name)
{
	return vfs->isExist(name);
}

//=====================================================================================//
//                               void RealFile::addRef()                               //
//=====================================================================================//
void RealFile::addRef()
{
	InterlockedIncrement(&m_refCtr);
}

//=====================================================================================//
//                              void RealFile::release()                               //
//=====================================================================================//
void RealFile::release()
{
	InterlockedDecrement(&m_refCtr);
	if(!m_refCtr)
	{
		delete this;
	}
	else if(m_refCtr == 1)
	{
		m_lastUseTime = timeGetTime();
	}
}
//=====================================================================================//
//                              char *RealFile::getData()                              //
//=====================================================================================//
const char *RealFile::getData()
{
	return (const char *)m_file->data();
}

//=====================================================================================//
//                           unsigned RealFile::getLength()                            //
//=====================================================================================//
unsigned RealFile::getLength()
{
	return m_length;
}

//=====================================================================================//
//                  std::auto_ptr<std::istream> RealFile::getStream()                  //
//=====================================================================================//
std::auto_ptr<std::istream> RealFile::getStream()
{
	if(m_file)
	{
		return std::auto_ptr<std::istream>(new std::istrstream(getData(),getLength()));
	}
	else
	{
		return std::auto_ptr<std::istream>(new std::ifstream(name().c_str(),std::ios::binary));
	}
}

}	//	namespace Muffle