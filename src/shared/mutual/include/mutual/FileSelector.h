#if !defined(__FILE_SELECTOR_H_INCLUDED_9253329046698881__)
#define __FILE_SELECTOR_H_INCLUDED_9253329046698881__

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#	pragma once
#endif

#include <mll/fs/fs.h>
#include <boost/utility.hpp>
#include <mll/debug/exception.h>

MLL_MAKE_EXCEPTION(no_file_alternative_found,mll::debug::exception);

//=====================================================================================//
//                                 class FileTypeList                                  //
//=====================================================================================//
class MlFileTypeList
{
	friend class MlFileSelector;
	std::list<std::string> m_exts;

public:
	MlFileTypeList() {}
	friend MlFileTypeList &operator,(MlFileTypeList &, const std::string &);
};

//=====================================================================================//
//                          inline FileTypeAdder &operator,()                          //
//=====================================================================================//
inline MlFileTypeList &operator,(MlFileTypeList &a,const std::string &e)
{
	a.m_exts.push_back(e);
	return a;
}

//=====================================================================================//
//                                   class FileType                                    //
//=====================================================================================//
class MlFileSelector
{
	std::list<std::string> m_exts;

public:
	MlFileSelector(MlFileTypeList &fa) { m_exts.swap(fa.m_exts); }

	mll::fs::file_id getFile(const std::string &) const;
	mll::fs::file_id hasFile(const std::string &) const;
};

//=====================================================================================//
//               inline mll::fs::file_id MlFileSelector::hasFile() const               //
//=====================================================================================//
inline mll::fs::file_id MlFileSelector::hasFile(const std::string &name) const
{
	using namespace mll::fs;
	fs_files<> tfiles = get_files(name+".*",files);
	file_container fls = tfiles.get_file_container();

	if(fls.begin() == fls.end()) return file_id();

	file_id result = *fls.begin();
	for(file_container::iterator i = boost::next(fls.begin()); i != fls.end(); ++i)
	{
		if(i->date() > result.date())
		{
			result = *i;
		}
	}

	return result;
}

//=====================================================================================//
//               inline mll::fs::file_id MlFileSelector::getFile() const               //
//=====================================================================================//
inline mll::fs::file_id MlFileSelector::getFile(const std::string &name) const
{
	using mll::fs::file_id;
	file_id result = hasFile(name);

	if(!result)
	{
		throw no_file_alternative_found("MlFileSelector::getFile: no file alternatives found [" + name + "]");
	}

	return result;
}

#endif // !defined(__FILE_SELECTOR_H_INCLUDED_9253329046698881__)