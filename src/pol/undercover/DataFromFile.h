#pragma once

//=====================================================================================//
//                        inline std::string getDataFromFile()                         //
//=====================================================================================//
inline std::string getDataFromFile(const std::string &name)
{
	VFile *file = DataMgr::Load(name.c_str());
	std::string result;
	result.resize(file->Size());
	std::copy_n(file->Data(),file->Size(),result.begin());
	DataMgr::Release(name.c_str());
	return result;
}