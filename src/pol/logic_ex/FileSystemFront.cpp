#include "precomp.h"
#include "FileSystemFront.h"

namespace PoL
{

//=====================================================================================//
//                    std::string FileSystemFront::getFileContent()                    //
//=====================================================================================//
std::string FileSystemFront::getFileContent(const std::string &name)
{
	std::string result;
	VFile *file = DataMgr::Load(name.c_str());
	result.resize(file->Size());
	std::copy_n(reinterpret_cast<const char *>(file->Data()), file->Size(), result.begin());
	DataMgr::Release(name.c_str());
	return result;
}

//=====================================================================================//
//                std::vector<std::string> FileSystemFront::getFiles()                 //
//=====================================================================================//
std::vector<std::string> FileSystemFront::getFiles(const std::string &dir, const std::string &ext)
{
	typedef std::vector<DataMgr::FileInfo> Files_t;

	Files_t files;
	std::string realdir = dir;
	std::string realext = ext;

	std::transform(realdir.begin(),realdir.end(),realdir.begin(),std::tolower);
	if(realdir[realdir.size()-1] != '/') realdir.push_back('/');

	std::transform(realext.begin(),realext.end(),realext.begin(),std::tolower);
	if(realext[0] != '.') realext.insert(realext.begin(),'.');

	DataMgr::Scan(realdir.c_str(),files);

	std::vector<std::string> result;

	for(Files_t::iterator i = files.begin(); i != files.end(); ++i)
	{
		std::string cmpext = i->m_Extension;
		std::transform(cmpext.begin(),cmpext.end(),cmpext.begin(),std::tolower);
		if(cmpext == realext)
		{
			result.push_back(dir+i->m_FullName);
		}
	}

	return result;
}

}