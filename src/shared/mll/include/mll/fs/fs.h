#if !defined(__FS_H_INCLUDED_8093570542662288__)
#define __FS_H_INCLUDED_8093570542662288__


#include "fs_existender.h"
#include "fs_files.h"
#include "fs_path.h"
#include "file_id.h"
#include "file.h"
#include "system.h"
#include "cache.h"
#include "entry.h"

#include <memory>





namespace mll
{

namespace fs
{




/////////////////////////////////////////////////////////////////////////
//	������������ ��� �������� �������� � �������� ����������
enum SearchParams
{ 
	files = 1<<0,
	dir = 1<<1,
	readonly = 1<<2
};

enum WriteParams
{ 
	append = 1
};
/////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////
//	������� ��������

//	������� ���������, ���������� ���� � ������� ����� ���������� ��������� 
//	��� "�������". ���� ��������� ����� ������ "�� ������ � �������"
struct date_heuristic
{
	bool operator()(const file_id& x, const file_id& y) const
		{ return (x.date() >= y.date()); }
};
/////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////
//	��������� ���� �� ������������

//	��������� �� ������� ����������� �������� �������� ��������
inline bool is_path_valid(const std::string& raw_data);
/////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////
//	������� ���� �� ������

//	�������, ��������� �������� ����������
template <class Pred>
inline std::auto_ptr<i_file> r_open_file(const std::string& file_name, 
										 Pred pred, bool seekable = true);

//	������� � �������������� ������������ ���������
inline std::auto_ptr<i_file> r_open_file(const std::string& file_name, bool seekable = true);

//	������� �� ��������������
inline std::auto_ptr<i_file> r_open_file(const file_id &id, bool seekable = true);
/////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////
//	��������� ���� �� �������������

//	���������, ��������� �������� ����������
template <class Pred>
inline file_id file_exists(const std::string& file_name, Pred pred);

//	��������� � �������������� ������������ ���������
inline file_id file_exists(const std::string& file_name);
/////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////
//	������� ���� �� ������

//	�������, ��������� �������� ����������
template <class Pred>
inline std::auto_ptr<o_file> w_open_file(const std::string& file_name, 
										 Pred pred, bool seekable = true);

//	������� � �������������� ������������ ���������
inline std::auto_ptr<o_file> w_open_file(const std::string& file_name, bool seekable = true);

//	������� �� �������������, �������� - append
inline std::auto_ptr<o_file> w_open_file(const file_id &id,
										 bool seekable = true, bool app = 0);
/////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////
//	������� ����

//	�������, ��������� �������� ����������
template <class Pred>
inline void delete_file(const std::string& file_name, Pred pred);

//	������� � �������������� ������������ ���������
inline void delete_file(const std::string& file_name);

//	������� �� ��������������
inline void delete_file(const file_id &id);
/////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////
//	�������� ������ ��� ������������

//	�������� ������-"����������" ������ �� ����� (path - ���� ������ � ������).
//	�� ���� ����� �������� ���������� file_id �� ���� ������ (����� ��������������
//	���������) � �� ��������� ������� ���������� �����

//	������� ������, ��������� �������� ����������
template <class Pred>
inline fs_files<Pred> get_files(const std::string& file_name, Pred pred, unsigned param);

//	������� � �������������� ������������ ���������
inline fs_files<date_heuristic> get_files(const std::string& file_name, unsigned param);
/////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////
//	������� ���������� ���������� �� ����

inline void make_dir(const std::string& file_name);
/////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////
//	������������/�������������� ���������

//	������������� ����������. ����� �� �������� ����� ������������ fs.
//	������������ ������, ������� ����� �������� � unmount() ��� ���������������
//	��������������.
inline sub_fs_id mount(std::auto_ptr<sub_fs> what, const std::string& file_name);

//	������������� ���������� � �������� ����� �� � ��������
inline std::auto_ptr<sub_fs> unmount(sub_fs_id id);
/////////////////////////////////////////////////////////////////////////






























//=====================================================================================//
//                                     ����������                                      //
//=====================================================================================//



/////////////////////////////////////////////////////////////////////////
//	��������� ���� �� ������������

//	��������� �� ������� ����������� �������� �������� ��������
inline bool is_path_valid(const std::string& raw_data)
{
	std::string data = raw_data;

	//	"�����������"
	std::use_facet<std::ctype<char> >(std::locale()).tolower(&data[0], &data[data.size()]);
	std::replace(data.begin(), data.end(), '\\', '/');

	std::string::size_type n;
	//	�������-�������
	n = data.find("..");
	if (n != data.npos)
		return false;
	//	������ ������ ����� (������ ��� ����� �������� ������� �������� ����)
	n = data.find("//", 2);
	if (n != data.npos)
		return false;
	std::string tmp;
	std::string norm;
	//	����������� ������� � ����
	norm = "qwertyuiopasdfghjklzxcvbnm1234567890`~!@#$%^&()_-+=[]{};',./ ";
	n = data.rfind('/');
	tmp = data.substr(0, n+1);
	if (tmp.find_first_not_of(norm) != tmp.npos)
		return false;
	//	����������� ������� � ����� �����
	norm += "*?";
	n = data.rfind('/');
	tmp = data.substr(n+1);
	if (tmp.find_first_not_of(norm) != tmp.npos)
		return false;

	return true;
}
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//	������� ���� �� ������
//	������� ����, ��������� �������� ����������
template <class Pred>
inline std::auto_ptr<i_file> r_open_file(const std::string& file_name,
										 Pred pred, bool seekable)
{
	if (!is_path_valid(file_name))
		throw incorrect_path("fs: bad filename\n" + file_name);

	fs_path file_path(file_name);
	file_id cached = system::instance()->cache()->single_query_stored(file_path.to_str());
	if (cached)
		return r_open_file(cached, seekable);

	entry guard(system::instance()->get_cs());
	fs_files<Pred> tmp(file_path, files);
	file_container heap = tmp.get_file_container();
	if (!heap.size())
		throw no_file("fs: you try to open non-existent file\n" + file_path.to_str());
	file_id to_cache = *heap.begin();
	system::instance()->cache()->add_single_query(to_cache);
	return r_open_file(to_cache, seekable);
}

inline std::auto_ptr<i_file> r_open_file(const std::string& file_name, bool seekable)
	{ return r_open_file(file_name, date_heuristic(), seekable); }


//	������� ���� �� id
inline std::auto_ptr<i_file> r_open_file(const file_id &id, bool seekable)
{
	return std::auto_ptr<i_file>(new i_file(id, id.sub_fs()->r_open_file(id.path(), seekable)));
}
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//	��������� ���� �� �������������

//	���������, ��������� �������� ����������
template <class Pred>
inline file_id file_exists(const std::string& file_name, Pred pred)
{
	if (!is_path_valid(file_name))
		return file_id();
	fs_path file_path(file_name);
	file_id cached = system::instance()->cache()->single_query_stored(file_path.to_str());
	if (cached)
		return cached;
	fs_files<Pred> tmp(file_path, files);
	file_container heap = tmp.get_file_container();
	if (!heap.size())
		return file_id();
	file_id to_cache = *heap.begin();
	system::instance()->cache()->add_single_query(to_cache);
	return to_cache;
}

//	��������� � �������������� ������������ ���������
inline file_id file_exists(const std::string& file_name)
{
	return file_exists(file_name, date_heuristic());
}
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//	������� ���� �� ������
//	������� ����, ��������� �������� ����������
template <class Pred>
inline std::auto_ptr<o_file> w_open_file(const std::string& file_name,
										 Pred pred, bool seekable)
{
	if (!is_path_valid(file_name))
		throw incorrect_path("fs: bad filename\n" + file_name);
	fs_path file_path(file_name);
//	file_id cached = system::instance()->cache()->single_query_stored(file_path.to_str());
//	if (cached)
//		return w_open_file(cached, seekable);
	fs_files<Pred> tmp(file_path, files);
	file_container heap = tmp.get_file_container();
	if (!heap.size())
	{
		//	� ������ �������� ������ �����
		fs_existender exist(file_path);
		const sub_fs *tmp_sub_fs = exist.get_sub_fs();
		fs_path tmp_sub_path = exist.get_sub_path();
		if (tmp_sub_path.length() > 1)
		{
			fs_path dirs(file_path);
			dirs.pop_back();
			make_dir(dirs.to_str());
		}
		file_id tmp_id(tmp_sub_fs, tmp_sub_path.to_str());
//		system::instance()->cache()->add_single_query(tmp_id);
		return std::auto_ptr<o_file>(new o_file(tmp_id, tmp_sub_fs->w_open_file(tmp_id.path(), false, seekable)));
	}
	else
	{
		//	� ������ �������� �������������
		file_id to_cache = *heap.begin();
//		system::instance()->cache()->add_single_query(to_cache);
		return w_open_file(to_cache, seekable);
	}
	return std::auto_ptr<o_file>(0);
}

inline std::auto_ptr<o_file> w_open_file(const std::string& file_name, bool seekable)
	{ return w_open_file(file_name, date_heuristic(), seekable); }


//	������� ���� �� id
inline std::auto_ptr<o_file> w_open_file(const file_id &id, bool seekable, bool app)
{
	if (app == append)
		return std::auto_ptr<o_file>(new o_file(id, id.sub_fs()->w_open_file(id.path(), true, seekable)));
	else
		return std::auto_ptr<o_file>(new o_file(id, id.sub_fs()->w_open_file(id.path(), false, seekable)));
	return std::auto_ptr<o_file>(0);
}
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//	������� ����
//	������� ����, ��������� �������� ����������
template <class Pred>
inline void delete_file(const std::string& file_name,
						Pred pred)
{
	if (!is_path_valid(file_name))
		throw incorrect_path("fs: bad filename\n" + file_name);
	fs_path file_path(file_name);
	fs_files<Pred> tmp(file_path, files|dir);
	file_container heap = tmp.get_file_container();
	if (!heap.size())
		throw no_file("fs: you try to delete non-existent file\n"+file_path.to_str());

	file_id id(*heap.begin());
	if (id.is_dir())
	{
		fs_files<Pred> tmp2(file_path.to_str() + "/*", files|dir);
		file_container heap2 = tmp2.get_file_container();
		if (heap2.size())
			delete_file(file_path.to_str() + "/" + (*heap2.begin()).name(), pred);
	}
	system::instance()->cache()->drop_file(id);
	delete_file(id);
}

inline void delete_file(const std::string& file_name)
{	delete_file(file_name, date_heuristic());	}


//	������� ���� �� id
inline void delete_file(const file_id &id)
{
	id.sub_fs()->delete_file(id.path());
}
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//	�������� ������-"����������" ������ �� ����� (path - ���� ������ � ������).
//	�� ���� ����� �������� ���������� file_id �� ���� ������ (����� ��������������
//	���������) � �� ��������� ������� ���������� �����
template <class Pred>
inline fs_files<Pred> get_files(const std::string& file_name, Pred pred, unsigned param)
{
	if (!is_path_valid(file_name))
		throw incorrect_path("fs: bad filename\n" + file_name);
	fs_path file_path(file_name);
	return fs_files<Pred>(file_path, param);
}

inline fs_files<date_heuristic> get_files(const std::string& file_name, unsigned param)
{	return get_files(file_name, date_heuristic(), param);	}
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//	������� ��������� �� ���������� ����
inline void make_dir(const std::string& file_name)
{
	if (!is_path_valid(file_name))
		throw incorrect_path("fs: bad filename\n" + file_name);
	fs_path file_path(file_name);
	fs_existender exist(file_path);
	const sub_fs *tmp_sub_fs = exist.get_sub_fs();
	fs_path tmp_sub_path = exist.get_sub_path();
	std::string tmp;
	for (fs_path::const_iterator i = tmp_sub_path.begin(); i != tmp_sub_path.end(); ++i)
	{
		tmp += '/' + *i + '/';
		tmp.erase(tmp.length()-1, tmp.length());
		tmp_sub_fs->make_dir(tmp);
	}
}
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//	������������� ����������. ����� �� �������� ����� ������������ �������
inline sub_fs_id mount(std::auto_ptr<sub_fs> what, const std::string& file_name)
{
	if (!is_path_valid(file_name))
		throw incorrect_path("fs: bad filename\n" + file_name);
	fs_path file_path(file_name);
	return system::instance()->mount(what, file_path);
}

//	������������� ���������� � �������� ����� �� � ��������
inline std::auto_ptr<sub_fs> unmount(sub_fs_id id)
{
	return system::instance()->unmount(id);
}

/////////////////////////////////////////////////////////////////////////



}

}

#endif // !defined(__FS_H_INCLUDED_8093570542662288__)