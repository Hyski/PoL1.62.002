//=====================================================================================//
//                                                                                     //
//                                   Alfa antiterror                                   //
//                                                                                     //
//                         Copyright by MiST land - South 2003                         //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Description:                                                                      //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Author: Pavel A.Duvanov                                                           //
//   Date:   12.07.2003                                                                //
//                                                                                     //
//=====================================================================================//
#ifndef __A_L_D_MGR_H_INCLUDED_3536178515763056__
#define __A_L_D_MGR_H_INCLUDED_3536178515763056__

#include <mll/utils/binary_storage.h>

namespace ALD	//	Alfa Level Data Management
{

class Id;
class Item;
class Folder;
class ItemIterator;
class FolderIterator;

//=====================================================================================//
//                                      class Id                                       //
//=====================================================================================//
class Id
{
public:

	enum
	{
		fFolder	=	1<<0,		//	узел является папкой
	};

private:

	unsigned int m_flags;			//	набор флагов
	std::string m_value;			//	значение идентификатора

public:
	Id();
	Id(const Id& another);
	Id(const std::string& value,unsigned int flags);
public:
	///	операторр копирования
	Id& operator=(const Id& another);
	///	оператор сравнения
	bool operator==(const Id& another) const;
	///	оператор неравенства
	bool operator!=(const Id& another) const;
	///	получить значение идентификатора
	const std::string& value(void) const;
	///	получить флаги идентификатора
	unsigned int flags(void) const;

	///	оператор сдвига в бинарный поток
	friend mll::io::obinstream& operator<<(mll::io::obinstream& stream,const Id& id);
	///	оператор сдвига из бинарного потока
	friend mll::io::ibinstream& operator>>(mll::io::ibinstream& stream,Id& id);

};

inline Id::Id() : m_flags(0)
{
}

inline Id::Id(const Id& another) : m_value(another.m_value),m_flags(another.m_flags)
{
}

inline Id::Id(const std::string& value,unsigned int flags) : m_value(value),m_flags(flags)
{
}
//	операторр копирования
inline Id& Id::operator=(const Id& another)
{
	if(&another != this)
	{
		m_value = another.m_value;
		m_flags = another.m_flags;
	}

	return *this;
}
//	оператор сравнения
inline bool Id::operator==(const Id& another) const
{
	return (m_value == another.m_value) && (m_flags == another.m_flags);
}
//	оператор неравенства
inline bool Id::operator!=(const Id& another) const
{
	return !(*this == another); 
}
//	получить значение идентификатора
inline const std::string& Id::value(void) const
{
	return m_value;
}
//	получить флаги идентификатора
inline unsigned int Id::flags(void) const
{
	return m_flags;
}
//	оператор сдвига в поток
inline mll::io::obinstream& operator<<(mll::io::obinstream& stream,const Id& id)
{
	return stream << id.m_value << id.m_flags;
}
//	оператор сдвига из потока
inline mll::io::ibinstream& operator>>(mll::io::ibinstream& stream,Id& id)
{
	return stream >> id.m_value >> id.m_flags;
}

//=====================================================================================//
//                                    class Manager                                    //
//=====================================================================================//
class Manager : boost::noncopyable
{
public:

	typedef Id id_t;
	typedef Item item_t;
	typedef Folder folder_t;
	typedef ItemIterator item_iterator;
	typedef FolderIterator folder_iterator;
	typedef mll::utils::binary_storage<id_t>::path path_t; 
	typedef mll::utils::binary_storage<id_t>::version version_t; 

private:

	typedef mll::utils::binary_storage<id_t> bs_t;
	bs_t m_bs;

public:
	Manager(const version_t& v);
	Manager(std::istream& stream);
	~Manager();
public:
	///	найти папку по указанному пути
	folder_iterator find(const path_t& path);
	///	получить начальный итератор на папку
	folder_iterator begin(void);
	///	получить конечный итератор на папку
	folder_iterator end(void);
	///	создать вложенную в корень папку, если она не существует, или получить уже существующую
	folder_iterator ifolder(const std::string& id);
	///	удалить папку из дерева
	void erase(folder_iterator it);
	///	сохранить данные в поток
	void save(std::ostream& stream);

public:
	///	получить версию формата
	const version_t& version(void) const;
	///	получить версию формата для изменений
	version_t& version(void);

};

//=====================================================================================//
//                                     class Item                                      //
//=====================================================================================//
///	ВНИМАНИЕ!!! важен порядок расположения данных
class Item : boost::noncopyable
{
private:

	typedef mll::utils::binary_storage<Id>::iterator bs_iterator;

	std::stringstream m_buffer;						//	промежуточный буфер (при вызове деструктора - буфер сбрасывается в данные)
	bs_iterator m_iterator;							//	итератор на узел с данными
	unsigned int m_level;							//	уровень вложенности папки
	std::string m_id;								//	идентификатор папки

	mll::io::obinstream m_obinstream;				//	бинарный поток
	mll::io::ibinstream m_ibinstream;				//	бинарный поток

public:
	Item(bs_iterator value);
	~Item();
public:
	///	получить поток на чтение данных из промежуточного буфера
	mll::io::obinstream& obinstream(void);
	///	получить поток на запись данных в промежуточный буфер
	mll::io::ibinstream& ibinstream(void);
	///	сохранить содержимое промежуточного буфера
	void flush(void);
	///	очистить содержимое промежуточного буфера
	void clear(void);

public:
	///	оператор равенства
	bool operator==(const Item& another) const;
	///	оператор неравенства
	bool operator!=(const Item& another) const;
	///	получить идентификатор папки
	const std::string& id(void) const;
	///	получить уровень вложенности
	unsigned int level(void) const;

private:
	///	установить идентификатор папки
	void id(const std::string& value);
	///	установить уровень вложенности
	void level(unsigned int value);

	friend ItemIterator;
};

//	оператор равенства
inline bool Item::operator==(const Item& another) const
{
	return m_iterator == another.m_iterator;
}
//	оператор неравенства
inline bool Item::operator!=(const Item& another) const
{
	return !(*this == another);
}
//	получить идентификатор папки
inline const std::string& Item::id(void) const
{
	return m_id;
}
//	получить уровень вложенности
inline unsigned int Item::level(void) const
{
	return m_level;
}
//	установить идентификатор папки
inline void Item::id(const std::string& value)
{
	m_id = value;
}
//	установить уровень вложенности
inline void Item::level(unsigned int value)
{
	m_level = value;
}
//	получить поток на чтение данных
inline mll::io::obinstream& Item::obinstream(void)
{
	return m_obinstream;
}
//	получить поток на запись данных
inline mll::io::ibinstream& Item::ibinstream(void)
{
	return m_ibinstream;
}


//=====================================================================================//
//                                 class ItemIterator                                  //
//=====================================================================================//
class ItemIterator : public std::iterator<std::forward_iterator_tag,Item>
{
public:

	typedef mll::utils::binary_storage<Id>::iterator bs_iterator;
	typedef ItemIterator item_iterator;
	typedef Item item_t;

private:

	boost::shared_ptr<item_t> m_item;
	bs_iterator m_iterator;

public:
	ItemIterator();
	explicit ItemIterator(bs_iterator ibsnode);
	ItemIterator(const item_iterator& another);
public:
	///	оператор присваивания
	item_iterator& operator=(const item_iterator& another);
	///	инкриментировать (префикс)
	item_iterator& operator++(void);
	///	инкриментировать (постфикс)
	item_iterator operator++(int);
	///	получить указатель на данные
	item_t* operator->(void);
	///	получить ссылку на данные
	item_t& operator*(void);
	///	равенство
	bool operator==(const item_iterator& another);
	///	неравенство
	bool operator!=(const item_iterator& another);

private:
	///	сформировать данные об итеме
	void createItemData(bs_iterator inode);

};

//	получить указатель на данные
inline ItemIterator::item_t* ItemIterator::operator->(void)
{
	return m_item.get();
}
//	получить ссылку на данные
inline ItemIterator::item_t& ItemIterator::operator*(void)
{
	return *m_item;
}

//=====================================================================================//
//                                    class Folder                                     //
//=====================================================================================//
class Folder : boost::noncopyable
{
private:

	unsigned int m_level;					//	уровень вложенности папки
	std::string m_id;						//	идентификатор папки

public:
	Folder();
	~Folder();
public:
	///	оператор равенства
	bool operator==(const Folder& another) const;
	///	оператор неравенства
	bool operator!=(const Folder& another) const;
	///	получить идентификатор папки
	const std::string& id(void) const;
	///	получить уровень вложенности
	unsigned int level(void) const;

private:
	///	установить идентификатор папки
	void id(const std::string& value);
	///	установить уровень вложенности
	void level(unsigned int value);

	friend FolderIterator;
};

//	оператор равенства
inline bool Folder::operator==(const Folder& another) const
{
	return (m_level == another.m_level) && (m_id == another.m_id);
}
//	оператор неравенства
inline bool Folder::operator!=(const Folder& another) const
{
	return !(*this == another);
}
//	получить идентификатор папки
inline const std::string& Folder::id(void) const
{
	return m_id;
}
//	получить уровень вложенности
inline unsigned int Folder::level(void) const
{
	return m_level;
}
//	установить идентификатор папки
inline void Folder::id(const std::string& value)
{
	m_id = value;
}
//	установить уровень вложенности
inline void Folder::level(unsigned int value)
{
	m_level = value;
}

//=====================================================================================//
//                                class FolderIterator                                 //
//=====================================================================================//
class FolderIterator : public std::iterator<std::forward_iterator_tag,Folder>
{
public:

	typedef mll::utils::binary_storage<Id>::iterator bs_iterator;
	typedef FolderIterator folder_iterator;
	typedef ItemIterator item_iterator;
	typedef Folder folder_t;
	typedef Id id_t;

private:

	boost::shared_ptr<folder_t> m_folder;
	bs_iterator m_iterator;

public:
	FolderIterator();
	FolderIterator(const folder_iterator& another);
public:
	///	оператор присваивания
	folder_iterator& operator=(const folder_iterator& another);
	///	инкриментировать (префикс)
	folder_iterator& operator++(void);
	///	инкриментировать (постфикс)
	folder_iterator operator++(int);
	///	получить указатель на данные
	folder_t* operator->(void);
	///	получить ссылку на данные
	folder_t& operator*(void);
	///	создать вложенную папку, если она не существует, или получить уже существующую
	folder_iterator ifolder(const std::string& id);
	///	получить начальный итератор на вложенную папку
	folder_iterator fbegin(void);
	///	получить конечный итератор на вложенную папку
	folder_iterator fend(void);
	///	создать вложенную итем, если он не существует, или получить уже существующий
	item_iterator iitem(const std::string& id);
	///	получить начальный итератор на вложенный итем
	item_iterator ibegin(void);
	///	получить конечный итератор на вложенный итем
	item_iterator iend(void);
	///	равенство
	bool operator==(const folder_iterator& another);
	///	неравенство
	bool operator!=(const folder_iterator& another);

private:
	///	конструктор из итератора структуры данных
	explicit FolderIterator(bs_iterator ibsnode);
	///	сформировать данные о папке
	void createFolderData(bs_iterator inode);

	friend Manager;
};

//	получить указатель на данные
inline FolderIterator::folder_t* FolderIterator::operator->(void)
{
	return m_folder.get();
}
//	получить ссылку на данные
inline FolderIterator::folder_t& FolderIterator::operator*(void)
{
	return *m_folder;
}

}

#endif //__A_L_D_MGR_H_INCLUDED_3536178515763056__
