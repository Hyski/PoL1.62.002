#include "precomp.h"

#include "filemgmt.h"
#include "RealFile.h"
#include "Ptr2.h"
#include "Work2.h"
#include "Worker2.h"

namespace Muffle
{

namespace FileMgmtDetails
{

//=====================================================================================//
//                      class FileMgmtWork : public Muffle2::Work                      //
//=====================================================================================//
class FileMgmtWork : public Muffle2::Work
{
	WHFileMgmt m_mgmt;

public:
	FileMgmtWork(FileMgmt *mgmt) : m_mgmt(mgmt) {}

	/// Возвращает true, если объект уже не актуален
	virtual bool expired() const { return m_mgmt.expired(); }

	/// Выполнить действие
	virtual void execute()
	{
		if(HFileMgmt lockedMgmt = m_mgmt.acquire())
		{
			lockedMgmt->onTimer();
		}
	}
};

}

using namespace FileMgmtDetails;
static WHFileMgmt g_inst;

//=====================================================================================//
//                                FileMgmt::FileMgmt()                                 //
//=====================================================================================//
FileMgmt::FileMgmt(ci_VFileSystem *vfs)
:	m_vfs(vfs),
	m_totalSize(0)
{
	g_inst = this;

	m_fileUnloader = Muffle2::Ptr<FileMgmtWork>::create(this);
	Muffle2::Worker::inst()->addWork(m_fileUnloader,10000);
}

//=====================================================================================//
//                                FileMgmt::~FileMgmt()                                //
//=====================================================================================//
FileMgmt::~FileMgmt()
{
	g_inst.reset();

	for(FileMap_t::iterator i = m_fileMap.begin(); i != m_fileMap.end();)
	{
		m_totalSize -= i->second->getLength();
		i->second->release();
		m_fileMap.erase(i++);
	}
}

//=====================================================================================//
//                           bool FileMgmt::canCreateFile()                            //
//=====================================================================================//
bool FileMgmt::canCreateFile(const std::string &bad_name)
{
	Muffle2::Lock guard = lock();

	std::string tname(bad_name);
	std::transform(tname.begin(),tname.end(),tname.begin(),tolower);
	std::replace(tname.begin(),tname.end(),'\\','/');

	FileMap_t::iterator i = m_fileMap.find(tname);

	if(i == m_fileMap.end())
	{
		return RealFile::canCreateFile(m_vfs,tname);
	}
	else
	{
		return true;
	}
}

//=====================================================================================//
//                            Stream FileMgmt::createFile()                            //
//=====================================================================================//
Stream FileMgmt::createFile(const std::string &bad_name)
{
	Muffle2::Lock guard = lock();

	std::string tname(bad_name);
	std::transform(tname.begin(),tname.end(),tname.begin(),tolower);
	std::replace(tname.begin(),tname.end(),'\\','/');

	FileMap_t::iterator i = m_fileMap.find(tname);

	if(i == m_fileMap.end())
	{
		RealFile *file = new RealFile(m_vfs,tname);
		file->addRef();
		m_fileMap.insert(std::make_pair(tname,file));
		m_totalSize += file->getLength();
		return Stream(file);
	}
	else
	{
		return Stream(i->second);
	}
}

//=====================================================================================//
//                              void FileMgmt::onTimer()                               //
//=====================================================================================//
void FileMgmt::onTimer()
{
	Muffle2::Lock guard = lock();

	unsigned int time = timeGetTime();

	for(FileMap_t::iterator i = m_fileMap.begin(); i != m_fileMap.end();)
	{
		if(i->second->getRefCounter() == 1 &&
			(time - i->second->getLastUseTime()) > 10000)
		{
			m_totalSize -= i->second->getLength();
			i->second->release();
			m_fileMap.erase(i++);
		}
		else
		{
			++i;
		}
	}
}

//=====================================================================================//
//                          unsigned FileMgmt::getTotalSize()                          //
//=====================================================================================//
unsigned FileMgmt::getTotalSize()
{
	return m_totalSize;
}

//=====================================================================================//
//                          unsigned FileMgmt::getFileCount()                          //
//=====================================================================================//
unsigned FileMgmt::getFileCount()
{
	return m_fileMap.size();
}

//=====================================================================================//
//                          void FileMgmt::outputDebugInfo()                           //
//=====================================================================================//
void FileMgmt::outputDebugInfo(SndServices *svc)
{
	Muffle2::Lock guard = lock();

	unsigned int y = 0;
	for(FileMap_t::iterator i = m_fileMap.begin(); i != m_fileMap.end(); ++i)
	{
		svc->dbg_printf(700,500+y,i->first.c_str());
		y += 20;
	}
}

//=====================================================================================//
//                             HFileMgmt FileMgmt::inst()                              //
//=====================================================================================//
HFileMgmt FileMgmt::inst()
{
	return g_inst.acquire();
}

}	//	namespace Muffle