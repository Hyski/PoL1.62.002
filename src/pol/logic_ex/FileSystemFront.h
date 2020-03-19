#pragma once

namespace PoL
{

//=====================================================================================//
//                                class FileSystemFront                                //
//=====================================================================================//
class FileSystemFront
{
public:
	/// Получить содержимое файла
	static std::string getFileContent(const std::string &);
	static std::vector<std::string> getFiles(const std::string &dir, const std::string &ext);
};

}