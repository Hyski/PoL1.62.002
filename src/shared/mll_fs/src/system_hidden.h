#if !defined(__SYSTEM_HIDDEN_H_INCLUDED_2903341426519653__)
#define __SYSTEM_HIDDEN_H_INCLUDED_2903341426519653__

#include "CriticalSection.h"

#include <mll/utils/lcrn_tree.h>
#include <mll/fs/system.h>
#include <mll/fs/cache.h>


namespace mll
{

namespace fs
{


//=====================================================================================//
//                                 class system_hidden                                 //
//=====================================================================================//
class system_hidden: public fs::system
{
private:
	mll::utils::lcrn_tree<directory *> m_tree;
	mutable critical_section m_cs;
	mll::fs::cache m_cache;

	iterator _clean(iterator node);

public:
	system_hidden();
	~system_hidden();
	
	iterator begin() { return m_tree.begin(); }
	iterator end() { return m_tree.end(); }
	const_iterator begin() const { return m_tree.begin(); }
	const_iterator end() const { return m_tree.end(); }

	void _create_path(const fs_path &path);
	iterator find_dir(const fs_path &where) const;

	sub_fs_id mount(std::auto_ptr<sub_fs> what, const fs_path &where);
	std::auto_ptr<sub_fs> unmount(sub_fs_id id);

	mll::fs::cache* cache();

	critical_section *get_cs() const { return &m_cs; }
};

}

}

#endif // !defined(__SYSTEM_HIDDEN_H_INCLUDED_2903341426519653__)