/***********************************************************************

Virtuality

Copyright by MiST land 2000

--------------------------------------------------------------------
Description:
--------------------------------------------------------------------
Author: Pavel A.Duvanov (Naughty)
Date:   15.04.2000

************************************************************************/
#include "Precomp.h"
#include "..\Utils\Dir.h"
#include "ZipInfo\ZipInfo.h"
#include "DataMgr.h"

#include <sys/stat.h>
#undef EnterCriticalSection


#include <fstream>
//---------- ��� ���� ------------
//#define _ENABLE_LOG
#ifdef _ENABLE_LOG
#include <time.h>
CLog datamgr;
#define dm datamgr["dm.log"]
#else
#define dm /##/
#endif
//--------------------------------

#define _ENABLE_CRITICAL_SECTION

namespace DataMgr
{
	struct PackInfo
	{
		int m_priority;				// ��������� ���� (��� ������ �����, ��� ������������ ���)
		std::string m_Path;			// ���������� ���� �� ����
		long m_CreationTime;		// ����� ��������
		PackInfo(std::string path, long ct, int pri)
			:	m_Path(path),
			m_CreationTime(ct),
			m_priority(pri)
		{
		}
	};
	const char* m_PackExtension = ".pak";
	//	----------------------------------------------------------------------------
	std::vector< std::string > m_DataNames;  //Grom
	std::vector< VFile* > m_DataRefs;  //Grom

	//	std::map<std::string,VFile*> m_DataMap;			//	������ ����������� ������
	std::set<PackInfo> m_Packs;						//	������ �������� �����
	//	������ �������� - ������ ��� ����� � ������ ��������, 
	//	������ �������� - �������������� ��� ����� � zip �����
	std::map<std::string,std::string> m_PackFiles;	//	������ ����������� ��� �����

	/////////////////////////////////////////////////////////////////////////
	//	#ifndef _ENABLE_CRITICAL_SECTION
	//	#define CriticalSection	/##/
	//	#else

	//=====================================================================================//
	//                              namespace CriticalSection                              //
	//=====================================================================================//
	namespace CriticalSection
	{
		//	����������� ������ ��������� ��� �������������� ���������� � �������� �������� ���������
		CRITICAL_SECTION m_CriticalSection;

		void Initialize(void)	
		{ 
			InitializeCriticalSection(&m_CriticalSection); 
			dm("...initialized critical section;\n");
		}
		void Delete(void)		
		{ 
			DeleteCriticalSection(&m_CriticalSection); 
			dm("...deleted critical section;\n");
		}
		void Enter(void)
		{
			EnterCriticalSection(&m_CriticalSection);
		}
		void Leave(void)
		{
			LeaveCriticalSection(&m_CriticalSection);
		}

	}

	//	#endif	//_ENABLE_CRITICAL_SECTION
	/////////////////////////////////////////////////////////////////////////
}

namespace DataMgr
{
	VFile *LoadFromPack(const char *pFileName);
	int PackDir(const char* dir_name,std::vector<DataMgr::FileInfo>& files);
	void SetFileInfo(DataMgr::FileInfo& fi,const char* file_name);
	long GetCreationTime(const char* path);

	//	�������� "������" ��� PackInfo
	inline bool operator < (const PackInfo& x, const PackInfo& y)
	{
		if(x.m_priority != y.m_priority) return x.m_priority < y.m_priority;
		if(x.m_CreationTime != y.m_CreationTime) return x.m_CreationTime > y.m_CreationTime;
		return x.m_Path < y.m_Path;
	}
}


//****************** ������������� DataMgr **********************************//
void DataMgr::ReadPacks2(const char* path, int priority)
{
	std::vector<std::string> files;
	char buff[MAX_PATH];
	Dir content;

	dm("Read packs from dir: [%s]\n",path);
	sprintf(buff,"%s/*%s",path,m_PackExtension);
	dm("...generate mask for search: [%s]\n\n",buff);
	content.Scan(buff);

	for(unsigned int i=0;i<content.Files().size();i++)
	{
		sprintf(buff,"%s/%s",path,content.Files()[i].full_name.c_str());
		dm("...look for pack [%s];\n",buff);
		m_Packs.insert(PackInfo(buff,GetCreationTime(buff),priority));
#ifdef _ENABLE_LOG
		long ct = GetCreationTime(buff);
		struct tm* t = localtime(&ct);
		dm("   creation time: %0.2d/%0.2d/%0.4d %0.2d:%0.2d:%0.2d\n",t->tm_mday,t->tm_mon,1900+t->tm_year,t->tm_hour,t->tm_min,t->tm_sec);
#endif// _ENABLE_LOG
	}

	for(std::set<PackInfo>::reverse_iterator i = m_Packs.rbegin(); i != m_Packs.rend(); ++i)
	{
		ZipInfo::ListZip(i->m_Path.c_str(),files);

		for(unsigned int j=0;j<files.size();j++)
		{
			strcpy(buff,files[j].c_str());
			m_PackFiles[_strlwr(buff)] = files[j];
			dm("        [%s]\n",_strlwr(buff));
		}
	}

#ifdef _ENABLE_LOG
	dm("\nPacks: [%d]\n",m_Packs.size());
	for(std::set<PackInfo>::iterator ip=m_Packs.begin();ip!=m_Packs.end();ip++)
	{
		struct tm* t = localtime(&ip->m_CreationTime);
		dm("...%s",ip->m_Path.c_str());
		dm(" [%s] ",ip->m_IsMod?"is_mod":"is_original");
		dm(" creation time: %0.2d/%0.2d/%0.4d %0.2d:%0.2d:%0.2d\n",t->tm_mday,t->tm_mon,1900+t->tm_year,t->tm_hour,t->tm_min,t->tm_sec);

	}
	dm("\nFiles:\n");
	for(std::map<std::string,std::string>::iterator it=m_PackFiles.begin();it!=m_PackFiles.end();it++)
		dm("...%s\n",it->second.c_str());
	dm("\n");
#endif// _ENABLE_LOG
}

//=====================================================================================//
//                             void DataMgr::Initialize()                              //
//=====================================================================================//
void DataMgr::Initialize(void)
{
	//	�������������� ����������� ������
	InitializeCriticalSection(&CriticalSection::m_CriticalSection);
	//	CriticalSection::Initialize();
}

//=====================================================================================//
//                            void DataMgr::Uninitialize()                             //
//=====================================================================================//
void DataMgr::Uninitialize(void)
{
	for(int i=0; i != m_DataRefs.size(); i++)
	{
		delete m_DataRefs[i];
	}

	m_DataRefs.clear();
	m_DataNames.clear();

	//	����������������� ����������� ������
	DeleteCriticalSection(&CriticalSection::m_CriticalSection);
	//	CriticalSection::Delete();
}

//=====================================================================================//
//                               VFile *DataMgr::Load()                                //
//=====================================================================================//
VFile *DataMgr::Load(const char *pDataName/*, int iParam*/)
{
	EnterCriticalSection(&CriticalSection::m_CriticalSection);

	std::map<std::string,VFile*>::iterator i;
	VFile *pVFile = 0;

	dm("loading [%s]: {\n",pDataName);

	std::vector<std::string>::iterator si;
	if(  (si=std::find(m_DataNames.begin(), m_DataNames.end(), pDataName)) == m_DataNames.end())
	{
		VFile *nf = new VFile(pDataName);
		m_DataNames.push_back(pDataName);
		m_DataRefs.push_back(nf);
		pVFile = nf;
		if(  (*m_DataRefs.rbegin())->Size() == 0)
		{
			delete (*m_DataRefs.rbegin());

			//	��������� �� ����
			if(!(pVFile = LoadFromPack(pDataName)))
			{
				VFile *nf=new VFile();
				*m_DataRefs.rbegin() = nf;
				pVFile = nf;
			}
			else
			{
				*m_DataRefs.rbegin() = pVFile;
			}
		}
	}
	else
	{

		pVFile = m_DataRefs[si-m_DataNames.begin()]; 
	}

	dm("}\n");
	LeaveCriticalSection(&CriticalSection::m_CriticalSection);
	return pVFile;
}

//=====================================================================================//
//                              bool DataMgr::IsExists()                               //
//=====================================================================================//
bool DataMgr::IsExists(const char *pDataName)
{
	bool result = false;

	EnterCriticalSection(&CriticalSection::m_CriticalSection);
	_finddata_t fd;
	intptr_t sh = _findfirst(pDataName,&fd);
	if(sh != -1)
	{
		_findclose(sh);
		result = true;
	}
	else
	{
		char szFileName[MAX_PATH]; 
		std::map<std::string,std::string>::iterator im;

		strcpy(szFileName,pDataName);
		im = m_PackFiles.find(strlwr(szFileName));
		result = (im != m_PackFiles.end());
	}
	LeaveCriticalSection(&CriticalSection::m_CriticalSection);

	return result;
}

void DataMgr::Release(const char* pDataName)
{
	EnterCriticalSection(&CriticalSection::m_CriticalSection);
	//	CriticalSection::Enter();

	dm(":try release file: <%s>\n",pDataName?pDataName:"!!! null file name !!!");
	if(pDataName)
	{
		std::vector<std::string>::iterator si;
		if((si=std::find(m_DataNames.begin(), m_DataNames.end(), pDataName))!=m_DataNames.end())
		{
			int pos=si-m_DataNames.begin();
			delete m_DataRefs[pos];
			m_DataRefs.erase(m_DataRefs.begin()+pos);
			m_DataNames.erase(si);
		}
	}

	LeaveCriticalSection(&CriticalSection::m_CriticalSection);
	//	CriticalSection::Leave();
}

VFile *DataMgr::LoadFromPack(const char *pFileName)
{
	char szFileName[MAX_PATH]; 
	std::map<std::string,std::string>::iterator im;
	VFile *pVFile = 0;
	unsigned char *pBuff;
	unsigned int size;

	strcpy(szFileName,pFileName);
	im = m_PackFiles.find(strlwr(szFileName));
	if(im != m_PackFiles.end())
	{
		for(std::set<PackInfo>::iterator ip=m_Packs.begin();ip!=m_Packs.end();ip++)
		{
			if(ZipInfo::LoadFile(ip->m_Path.c_str(),im->second.c_str(),&pBuff,&size))
			{
				//				dm("in pack <%s> ",szFileName);
				std::string name(im->second);
				std::transform(name.begin(),name.end(),name.begin(),tolower);
				pVFile = new VFile(pBuff,size,name.c_str());
				if(pBuff)
					delete[] pBuff;
				break;
			}
		}
	}

	return pVFile;
}

int DataMgr::PackDir(const char* dir_name,std::vector<DataMgr::FileInfo>& files)
{
	std::map<std::string,std::string>::iterator i;
	DataMgr::FileInfo fi;
	char buff[MAX_PATH];
	int len;

	files.clear();
	if(dir_name && (len = strlen(dir_name)))
	{
		strcpy(buff,dir_name);
		_strlwr(buff);
		i = m_PackFiles.find(buff);
		if(i != m_PackFiles.end())
		{
			for(i++;i!=m_PackFiles.end();i++)
			{
				if(!_strnicmp(buff,i->first.c_str(),len))
				{
					char* p = strchr(i->first.c_str()+len,'/');
					if(!p || (p && strlen(p)==1))
					{
						//						dm("    ...find: \"%s\"\n",i->second.c_str());

						SetFileInfo(fi,i->second.c_str());
						files.push_back(fi);
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return files.size();
}

void DataMgr::SetFileInfo(DataMgr::FileInfo& fi,const char* file_name)
{
	char buff[MAX_PATH];
	char* ptr;
	int len = strlen(file_name);

	fi.m_Attr = 0;
	fi.m_Location = DataMgr::FileInfo::IN_PACK;
	fi.m_FullName.clear();
	fi.m_Name.clear();
	fi.m_Extension.clear();

	strcpy(buff,file_name);
	if(buff[len-1] == '/')
	{
		buff[len-1] = 0;
		fi.m_Attr = DataMgr::FileInfo::SUBDIR;
	}
	ptr = strrchr(buff,'/');
	if(ptr)
		fi.m_FullName = ptr+1;
	else
		fi.m_FullName = buff;
	ptr = strchr(buff,'.');
	if(ptr)
	{
		fi.m_Extension = ptr;
		if(fi.m_FullName.size())
		{
			len = fi.m_FullName.size()-fi.m_Extension.size();
			strncpy(buff,fi.m_FullName.c_str(),len);
			buff[len] = 0;
			fi.m_Name = buff;
		}
	}
	else
	{
		fi.m_Name = fi.m_FullName;
	}

	dm("splitpath: [%s] [%s] [%s]\n",fi.m_FullName.c_str(),fi.m_Name.c_str(),fi.m_Extension.c_str());
}

//	�������� ���-�� ������ �������� � ������
int DataMgr::GetFileNum(void)	//	���-�� �������� ������
{
	/*if(rand()<50)
	{
	static int n=0;
	std::ostringstream name;
	name<<"data_mgr"<<n++<<".log";
	std::ofstream f(name.str().c_str());
	for(int i=0;i<m_DataNames.size();i++)
	{
	f<<"#"<<i<<" at "<<m_DataRefs[i]<<" => "<<m_DataNames[i]<<"\n";
	}
	}
	*/

	return m_DataNames.size();
}

int DataMgr::GetDataSize(void)	//	���-�� ���� ���������� ��� � ������
{
	return 0;
	/*dm("gd-enter\n");
	int size = 0;

	CriticalSection::Enter();

	for(std::map<std::string,VFile*>::iterator i = m_DataMap.begin();i!=m_DataMap.end();i++)
	{
	dm("%p ",i->second);
	size += i->second->Size();
	}

	CriticalSection::Leave();

	dm("gd-leave\n");  
	return size;		*/
}
//	������������ �������� � ���� � �� �����
int DataMgr::Scan(const char* dir_name,std::vector<DataMgr::FileInfo>& files,METHOD method)
{
	std::vector<::FileInfo> on_disk_files;
	std::map<std::string,DataMgr::FileInfo> result;
	DataMgr::FileInfo fi;
	char buff[MAX_PATH];
	Dir on_disk;

	files.clear();
	if(method != INPACK)
	{//	��������� ����� �� �����
		sprintf(buff,"%s*.*",dir_name);
		on_disk.Scan(buff);
		on_disk_files = on_disk.Files();
#ifdef _DEBUG_SHELL
		dm("\nScan on disk result - [%d] files:\n\n",on_disk_files.size());
		for(unsigned int l=0;l<on_disk_files.size();l++)
			dm("[%s] [%s] [%s]\n",on_disk_files[l].full_name.c_str(),on_disk_files[l].name.c_str(),on_disk_files[l].extension.c_str());
		dm("\n");
#endif
	}
	if(method != ONDISK)
	{//	��������� ����� � ����
		PackDir(dir_name,files);
	}
	for(unsigned int i=0;i<on_disk_files.size();i++)
	{
		fi.m_FullName = on_disk_files[i].full_name;
		fi.m_Name = on_disk_files[i].name;
		fi.m_Extension = on_disk_files[i].extension;
		fi.m_Location = DataMgr::FileInfo::ON_DISK;
		fi.m_Attr = on_disk_files[i].attr;
		strcpy(buff,fi.m_FullName.c_str());
		_strlwr(buff);
		result[buff] = fi;
	}
	on_disk_files.clear();
	for(unsigned int i=0;i<files.size();i++)
	{
		strcpy(buff,files[i].m_FullName.c_str());
		_strlwr(buff);
		result[buff] = files[i];
	}
	files.clear();
	for(std::map<std::string,DataMgr::FileInfo>::iterator it=result.begin();it!=result.end();it++)
	{
		files.push_back(it->second);
	}

#ifdef _DEBUG_SHELL
	dm("\nScan result - [%d] files:\n\n",files.size());
	for(unsigned int k=0;k<files.size();k++)
		dm("%s [%s]\n",files[k].m_FullName.c_str(),files[k].m_Name.c_str());
#endif

	return files.size();
}

long DataMgr::GetCreationTime(const char* path)
{
	struct _stat st;

	_stat(path,&st);

	return st.st_ctime;
}
