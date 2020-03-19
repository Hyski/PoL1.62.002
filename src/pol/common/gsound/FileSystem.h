#if !defined(__VIRTUAL_FILE_SYSTEM_WRAPPER_INCLUDED__)
#define __VIRTUAL_FILE_SYSTEM_WRAPPER_INCLUDED__

#include <Muffle/VFS.h>
#include "../DataMgr/DataMgr.h"
#include <string>
#include <vector>
#include <map>

class Dir;

//=====================================================================================//
//                                   class cc_VDirIt                                   //
//=====================================================================================//
class cc_VDirIt : public Muffle::ci_VDirIt
{
	typedef std::vector<DataMgr::FileInfo> files_t;
	files_t m_Files;
	int m_Current;
	std::string m_Mask;
	std::string m_Path;

	void findFirst();

public:
	cc_VDirIt(const std::string &name);
	~cc_VDirIt();

	bool isDone() const; 
	void next();
	ci_VDirIt *clone() const;

	bool isArchive() const;
	bool isHidden() const;
	bool isReadOnly() const;
	bool isDir() const;
	bool isSystem() const;

	const std::string &getFullName() const;
	const std::string &getName() const;
	const std::string &getExtension() const;

	void release();
};

//=====================================================================================//
//                                   class cc_VFile                                    //
//=====================================================================================//
class cc_VFile : public Muffle::ci_VFile
{
	VFile *m_File;

public:
	cc_VFile(const std::string &name);
	~cc_VFile();

	int    size() const;

	const void *data() const;

	void   release();
};

//=====================================================================================//
//                                class cc_VFileSystem                                 //
//=====================================================================================//
class cc_VFileSystem : public Muffle::ci_VFileSystem
{
	static cc_VFileSystem *m_Instance;

	typedef std::map<std::string,unsigned> RefCounters_t;
	mutable RefCounters_t m_refCounters;

public:
	Muffle::ci_VFile* openFile(const std::string &) const;
	Muffle::ci_VDirIt *getDir(const std::string &);
	bool isExist(const std::string &) const;

	bool releaseFile(const char *);

	static void initInstance();
	static void shutInstance();
	static Muffle::ci_VFileSystem *instance();
};

#endif