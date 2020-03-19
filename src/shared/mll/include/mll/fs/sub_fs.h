#if !defined(__SUB_FS_H_INCLUDED_8016431795074442__)
#define __SUB_FS_H_INCLUDED_8016431795074442__


#include "fs_path.h"

#include <ostream>
#include <istream>
#include <memory>
#include <time.h>


namespace mll
{

namespace fs
{


class sub_fs_iter;


//=====================================================================================//
//                                    class sub_fs                                     //
//=====================================================================================//
class sub_fs
{
public:
	virtual ~sub_fs() = 0 {}

	///	Дисковый путь до объекта подсистемы (каталога, zip-файла etc), со слешем на конце
	virtual const std::string& disk_path() const = 0;
	/// Возможность изменять файлы внутри подсистемы
	virtual bool system_readonly() const = 0;
	/// Существует ли файл с указанным именем
	virtual bool file_exists(const fs_path &name) const = 0;
	/// Создать итератор внутри подсистемы по введенному пути
	virtual std::auto_ptr<sub_fs_iter> new_iterator(const fs_path &) const = 0;
	/// Размер файла с указанным именем в байтах
	virtual unsigned file_size(const fs_path &name) const = 0;
	/// Возможность писать в файл с указанным именем
	virtual bool is_readonly(const fs_path &name) const = 0;
	/// Дата последнего изменения файла с указанным именем
	virtual time_t date(const fs_path &name) const = 0;
	/// Является ли файл с указанным именем директорией
	virtual bool is_dir(const fs_path &name) const = 0;
	/// Вернуть указатель на поток istream для файла с указанным именем
	virtual std::auto_ptr<std::istream> r_open_file(const fs_path &path,
		bool seekable = true) const = 0;
	/// Вернуть указатель на поток ostream для файла с указанным именем
	/** Если указан параметр append, поток должен указывать 
		на конец файла.*/
	virtual std::auto_ptr<std::ostream> w_open_file(const fs_path &path,
		bool append, bool seekable = true) const = 0;
	/// Удалить файл с указанным именем
	virtual void delete_file(const fs_path &name) const = 0;
	///	Создать директорию
	virtual void make_dir(const fs_path &path) const = 0;
};

}

}

#endif // !defined(__SUB_FS_H_INCLUDED_8016431795074442__)