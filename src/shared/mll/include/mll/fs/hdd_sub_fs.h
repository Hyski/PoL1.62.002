#if !defined(__HDD_SUB_FS_H_INCLUDED_7224230428579350__)
#define __HDD_SUB_FS_H_INCLUDED_7224230428579350__


#include "_export_rules.h"
#include "_choose_lib.h"

#include <mll/fs/sub_fs.h>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                  class hdd_sub_fs                                   //
//=====================================================================================//
class FILE_SYSTEM_EXPORT hdd_sub_fs: public sub_fs
{
private:
	std::string m_real_path;
	
	std::string translate(const fs_path &path) const;

public:
	hdd_sub_fs(const std::string& real_path);
	~hdd_sub_fs() {}

public:
	const std::string& disk_path() const;
	bool system_readonly() const;
	bool file_exists(const fs_path &name) const;
	std::auto_ptr<sub_fs_iter> new_iterator(const fs_path &) const;
	unsigned file_size(const fs_path &name) const;
	bool is_readonly(const fs_path &name) const;
	time_t date(const fs_path &name) const;
	bool is_dir(const fs_path &name) const;
	std::auto_ptr<std::istream> r_open_file(const fs_path &path, bool seekable) const;
	std::auto_ptr<std::ostream> w_open_file(const fs_path &path, bool append, 
		bool seekable) const;
	void delete_file(const fs_path &name) const;
	void make_dir(const fs_path &path) const;
};

}

}

#endif // !defined(__HDD_SUB_FS_H_INCLUDED_7224230428579350__)