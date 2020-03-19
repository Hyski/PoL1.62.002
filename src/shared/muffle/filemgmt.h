#if !defined(__FILEMGMT_H_INCLUDED_7734585223535450__)
#define __FILEMGMT_H_INCLUDED_7734585223535450__

#include "Ptr2.h"
#include "Work2.h"
#include "SafeAlloc2.h"
#include "Synchronized2.h"
#include <hash_map>

namespace Muffle
{

class RealFile;
class FileMgmt;

typedef Muffle2::Ptr<FileMgmt> HFileMgmt;
typedef Muffle2::WeakPtr<FileMgmt> WHFileMgmt;

//=====================================================================================//
//                                   class FileMgmt                                    //
//=====================================================================================//
class FileMgmt : public Muffle2::Synchronized
{
	typedef std::hash_map<std::string,
						  RealFile *,
						  std::hash<std::string>,
						  std::equal_to<std::string>,
						  Muffle2::SafeAlloc< std::pair< const std::string, RealFile*> > > FileMap_t;

	FileMap_t m_fileMap;
	ci_VFileSystem *m_vfs;

	unsigned int m_totalSize;
	Muffle2::HWork m_fileUnloader;

public:
	FileMgmt(ci_VFileSystem *);
	~FileMgmt();

	bool canCreateFile(const std::string &);
	Stream createFile(const std::string &);

	unsigned int getTotalSize();
	unsigned int getFileCount();

	void onTimer();

	void outputDebugInfo(SndServices*);

	/// Возвращает экземпляр класса FileMgmt
	static HFileMgmt inst();
};

}	//	namespasce Muffle

#endif // !defined(__FILEMGMT_H_INCLUDED_7734585223535450__)