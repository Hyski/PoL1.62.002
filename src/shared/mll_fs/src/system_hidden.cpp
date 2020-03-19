
#include "precomp.h"

#include "system_hidden.h"

#include <mll/fs/directory.h>
#include <mll/fs/fs_guider.h>
#include <mll/fs/entry.h>

using namespace mll::fs;


//=====================================================================================//
//                           system_hidden::system_hidden()                            //
//=====================================================================================//
system_hidden::system_hidden()
{
	m_tree.create_root(new directory("root"));
}

//=====================================================================================//
//                           system_hidden::~system_hidden()                           //
//=====================================================================================//
system_hidden::~system_hidden()
{
	for (iterator i = begin(); i != end(); ++i)
		delete *i;
	m_tree.clear();
}

//=====================================================================================//
//                          void system_hidden::_create_path()                         //
//=====================================================================================//
void system_hidden::_create_path(const fs_path &path)
{
	entry guard(get_cs());

	bool dir_founded = false;
	dir_tree::iterator itor = m_tree.begin();
	// шагаем по path
	for (fs_path::const_iterator i = path.begin(); i != path.end(); ++i)
	{
		// если находим директорию с таким именем, итерируем вперед по path
		for (dir_tree::iterator j = itor.son_begin(); j != itor.son_end(); j.brother())
		{
			directory *tmp_dir = *j;
			if (tmp_dir->name() == *i)
			{
				itor = j;
				dir_founded = true;
			}
		}
		// а если не находим, то создаем
		if (dir_founded == false)
		{
			itor = m_tree.insert_son(itor, new directory(*i));
		}
	}
}

//=====================================================================================//
//                      iterator system_hidden::find_dir() const                       //
//=====================================================================================//
system_hidden::iterator system_hidden::find_dir(const fs_path &where) const
{
	entry guard(get_cs());

	iterator tmp;
	fs_guider guid(where);
	for (; !guid.is_done(); guid.next())
		tmp = guid.iter();
    return tmp;
}

//=====================================================================================//
//                          sub_fs_id system_hidden::mount()                           //
//=====================================================================================//
sub_fs_id system_hidden::mount(std::auto_ptr<sub_fs> what, const fs_path &where)
{
	entry guard(get_cs());

	m_cache.clear_all();
	_create_path(where);
	iterator tmp_tree_iter = find_dir(where);
	directory *tmp_dir = *tmp_tree_iter;
	directory::iterator tmp_dir_iter = tmp_dir->mount(what.release());
	return sub_fs_id(tmp_tree_iter, tmp_dir_iter);
}

//=====================================================================================//
//                   std::auto_ptr<sub_fs> system_hidden::unmount()                    //
//=====================================================================================//
std::auto_ptr<sub_fs> system_hidden::unmount(sub_fs_id id)
{
	entry guard(get_cs());

	m_cache.clear_all();
	directory *tmp_dir = *id.get_tree_iter();
	directory::iterator tmp_dir_iter = id.get_dir_iter();
	std::auto_ptr<sub_fs> tmp_sub_fs = tmp_dir->unmount(tmp_dir_iter);
	_clean(begin());
	return tmp_sub_fs;
}

//=====================================================================================//
//                       mll::fs::cache* system_hidden::cache()                        //
//=====================================================================================//
mll::fs::cache* system_hidden::cache()
{
	return &m_cache;
}

//=====================================================================================//
//                          iterator system_hidden::_clean()                           //
//=====================================================================================//
system::iterator system_hidden::_clean(iterator node)
{
	entry guard(get_cs());

	//	очистка мусера
	std::string tmp = (*node)->name();

	if (node.has_son())
	{
		iterator node_son;
		for (node_son = node.son_begin(); node_son != node.son_end();)
		{
			node_son = _clean(node_son);
		}
	}
	if(!node.has_son())
	{
		if (!((*node)->size()) && (tmp != "root"))
		{
			delete *node;
			iterator new_node = node.get_brother();
			m_tree.erase(node);
			return new_node;
		}
	}
	return node.get_brother();
}

