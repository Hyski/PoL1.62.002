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
	///	получить данные файла
	virtual const void* data(void) const = 0;
	///	получить размер файла
	virtual int size(void) const = 0;
	///	закрыть файл
	virtual void release(void) = 0;
};

//=====================================================================================//
//                                class ci_VFileSystem                                 //
//=====================================================================================//
class ci_VFileSystem
{
public:
	/// открыть файл
	virtual ci_VFile* openFile(const std::string& fname) const = 0;
	///	проверить на существование файла
	virtual bool isExist(const std::string& fname) const = 0;
	/// возвращает вектор с FileInfo из файлов находящихся в Dir
	virtual ci_VDirIt* getDir(const std::string& path) = 0; 

};

}	//	namespace Muffle

#endif