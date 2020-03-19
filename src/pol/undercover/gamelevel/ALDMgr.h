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
		fFolder	=	1<<0,		//	���� �������� ������
	};

private:

	unsigned int m_flags;			//	����� ������
	std::string m_value;			//	�������� ��������������

public:
	Id();
	Id(const Id& another);
	Id(const std::string& value,unsigned int flags);
public:
	///	��������� �����������
	Id& operator=(const Id& another);
	///	�������� ���������
	bool operator==(const Id& another) const;
	///	�������� �����������
	bool operator!=(const Id& another) const;
	///	�������� �������� ��������������
	const std::string& value(void) const;
	///	�������� ����� ��������������
	unsigned int flags(void) const;

	///	�������� ������ � �������� �����
	friend mll::io::obinstream& operator<<(mll::io::obinstream& stream,const Id& id);
	///	�������� ������ �� ��������� ������
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
//	��������� �����������
inline Id& Id::operator=(const Id& another)
{
	if(&another != this)
	{
		m_value = another.m_value;
		m_flags = another.m_flags;
	}

	return *this;
}
//	�������� ���������
inline bool Id::operator==(const Id& another) const
{
	return (m_value == another.m_value) && (m_flags == another.m_flags);
}
//	�������� �����������
inline bool Id::operator!=(const Id& another) const
{
	return !(*this == another); 
}
//	�������� �������� ��������������
inline const std::string& Id::value(void) const
{
	return m_value;
}
//	�������� ����� ��������������
inline unsigned int Id::flags(void) const
{
	return m_flags;
}
//	�������� ������ � �����
inline mll::io::obinstream& operator<<(mll::io::obinstream& stream,const Id& id)
{
	return stream << id.m_value << id.m_flags;
}
//	�������� ������ �� ������
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
	///	����� ����� �� ���������� ����
	folder_iterator find(const path_t& path);
	///	�������� ��������� �������� �� �����
	folder_iterator begin(void);
	///	�������� �������� �������� �� �����
	folder_iterator end(void);
	///	������� ��������� � ������ �����, ���� ��� �� ����������, ��� �������� ��� ������������
	folder_iterator ifolder(const std::string& id);
	///	������� ����� �� ������
	void erase(folder_iterator it);
	///	��������� ������ � �����
	void save(std::ostream& stream);

public:
	///	�������� ������ �������
	const version_t& version(void) const;
	///	�������� ������ ������� ��� ���������
	version_t& version(void);

};

//=====================================================================================//
//                                     class Item                                      //
//=====================================================================================//
///	��������!!! ����� ������� ������������ ������
class Item : boost::noncopyable
{
private:

	typedef mll::utils::binary_storage<Id>::iterator bs_iterator;

	std::stringstream m_buffer;						//	������������� ����� (��� ������ ����������� - ����� ������������ � ������)
	bs_iterator m_iterator;							//	�������� �� ���� � �������
	unsigned int m_level;							//	������� ����������� �����
	std::string m_id;								//	������������� �����

	mll::io::obinstream m_obinstream;				//	�������� �����
	mll::io::ibinstream m_ibinstream;				//	�������� �����

public:
	Item(bs_iterator value);
	~Item();
public:
	///	�������� ����� �� ������ ������ �� �������������� ������
	mll::io::obinstream& obinstream(void);
	///	�������� ����� �� ������ ������ � ������������� �����
	mll::io::ibinstream& ibinstream(void);
	///	��������� ���������� �������������� ������
	void flush(void);
	///	�������� ���������� �������������� ������
	void clear(void);

public:
	///	�������� ���������
	bool operator==(const Item& another) const;
	///	�������� �����������
	bool operator!=(const Item& another) const;
	///	�������� ������������� �����
	const std::string& id(void) const;
	///	�������� ������� �����������
	unsigned int level(void) const;

private:
	///	���������� ������������� �����
	void id(const std::string& value);
	///	���������� ������� �����������
	void level(unsigned int value);

	friend ItemIterator;
};

//	�������� ���������
inline bool Item::operator==(const Item& another) const
{
	return m_iterator == another.m_iterator;
}
//	�������� �����������
inline bool Item::operator!=(const Item& another) const
{
	return !(*this == another);
}
//	�������� ������������� �����
inline const std::string& Item::id(void) const
{
	return m_id;
}
//	�������� ������� �����������
inline unsigned int Item::level(void) const
{
	return m_level;
}
//	���������� ������������� �����
inline void Item::id(const std::string& value)
{
	m_id = value;
}
//	���������� ������� �����������
inline void Item::level(unsigned int value)
{
	m_level = value;
}
//	�������� ����� �� ������ ������
inline mll::io::obinstream& Item::obinstream(void)
{
	return m_obinstream;
}
//	�������� ����� �� ������ ������
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
	///	�������� ������������
	item_iterator& operator=(const item_iterator& another);
	///	���������������� (�������)
	item_iterator& operator++(void);
	///	���������������� (��������)
	item_iterator operator++(int);
	///	�������� ��������� �� ������
	item_t* operator->(void);
	///	�������� ������ �� ������
	item_t& operator*(void);
	///	���������
	bool operator==(const item_iterator& another);
	///	�����������
	bool operator!=(const item_iterator& another);

private:
	///	������������ ������ �� �����
	void createItemData(bs_iterator inode);

};

//	�������� ��������� �� ������
inline ItemIterator::item_t* ItemIterator::operator->(void)
{
	return m_item.get();
}
//	�������� ������ �� ������
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

	unsigned int m_level;					//	������� ����������� �����
	std::string m_id;						//	������������� �����

public:
	Folder();
	~Folder();
public:
	///	�������� ���������
	bool operator==(const Folder& another) const;
	///	�������� �����������
	bool operator!=(const Folder& another) const;
	///	�������� ������������� �����
	const std::string& id(void) const;
	///	�������� ������� �����������
	unsigned int level(void) const;

private:
	///	���������� ������������� �����
	void id(const std::string& value);
	///	���������� ������� �����������
	void level(unsigned int value);

	friend FolderIterator;
};

//	�������� ���������
inline bool Folder::operator==(const Folder& another) const
{
	return (m_level == another.m_level) && (m_id == another.m_id);
}
//	�������� �����������
inline bool Folder::operator!=(const Folder& another) const
{
	return !(*this == another);
}
//	�������� ������������� �����
inline const std::string& Folder::id(void) const
{
	return m_id;
}
//	�������� ������� �����������
inline unsigned int Folder::level(void) const
{
	return m_level;
}
//	���������� ������������� �����
inline void Folder::id(const std::string& value)
{
	m_id = value;
}
//	���������� ������� �����������
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
	///	�������� ������������
	folder_iterator& operator=(const folder_iterator& another);
	///	���������������� (�������)
	folder_iterator& operator++(void);
	///	���������������� (��������)
	folder_iterator operator++(int);
	///	�������� ��������� �� ������
	folder_t* operator->(void);
	///	�������� ������ �� ������
	folder_t& operator*(void);
	///	������� ��������� �����, ���� ��� �� ����������, ��� �������� ��� ������������
	folder_iterator ifolder(const std::string& id);
	///	�������� ��������� �������� �� ��������� �����
	folder_iterator fbegin(void);
	///	�������� �������� �������� �� ��������� �����
	folder_iterator fend(void);
	///	������� ��������� ����, ���� �� �� ����������, ��� �������� ��� ������������
	item_iterator iitem(const std::string& id);
	///	�������� ��������� �������� �� ��������� ����
	item_iterator ibegin(void);
	///	�������� �������� �������� �� ��������� ����
	item_iterator iend(void);
	///	���������
	bool operator==(const folder_iterator& another);
	///	�����������
	bool operator!=(const folder_iterator& another);

private:
	///	����������� �� ��������� ��������� ������
	explicit FolderIterator(bs_iterator ibsnode);
	///	������������ ������ � �����
	void createFolderData(bs_iterator inode);

	friend Manager;
};

//	�������� ��������� �� ������
inline FolderIterator::folder_t* FolderIterator::operator->(void)
{
	return m_folder.get();
}
//	�������� ������ �� ������
inline FolderIterator::folder_t& FolderIterator::operator*(void)
{
	return *m_folder;
}

}

#endif //__A_L_D_MGR_H_INCLUDED_3536178515763056__
