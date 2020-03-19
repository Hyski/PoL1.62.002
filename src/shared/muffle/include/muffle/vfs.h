#if !defined(__VIRTUAL_FILE_SYSTEM_INCLUDED__)
#define __VIRTUAL_FILE_SYSTEM_INCLUDED__

namespace Muffle
{

//=====================================================================================//
//                                   class ci_VDirIt                                   //
//=====================================================================================//
class ci_VDirIt
{
public:
	virtual const std::string& getFullName() const = 0;
	virtual bool isDone() const = 0; 
	virtual void release() = 0;
	virtual void next() = 0;
};

//=====================================================================================//
//                                   class ci_VFile                                    //
//=====================================================================================//
class ci_VFile
{
public:
	///	�������� ������ �����
	virtual const void* data(void) const = 0;
	///	�������� ������ �����
	virtual int size(void) const = 0;
	///	������� ����
	virtual void release(void) = 0;
};

//=====================================================================================//
//                                class ci_VFileSystem                                 //
//=====================================================================================//
class ci_VFileSystem
{
public:
	/// ������� ����
	virtual ci_VFile* openFile(const std::string& fname) const = 0;
	///	��������� �� ������������� �����
	virtual bool isExist(const std::string& fname) const = 0;
	/// ���������� ������ � FileInfo �� ������ ����������� � Dir
	virtual ci_VDirIt* getDir(const std::string& path) = 0; 

};

}	//	namespace Muffle

#endif