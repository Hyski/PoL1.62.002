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
#include "Precomp.h"
#include "ALDMgr.h"

namespace ALD
{

namespace Utility
{

//=====================================================================================//
//       mll::utils::binary_storage<Manager::id_t>::iterator getBrotherOrSon();        //
//=====================================================================================//
template<bool>
mll::utils::binary_storage<Manager::id_t>::iterator getBrotherOrSon(mll::utils::binary_storage<Manager::id_t>::iterator it);

//=====================================================================================//
//   mll::utils::binary_storage<Manager::id_t>::iterator getBrotherOrSon<true>(); }    //
//=====================================================================================//
template<>
mll::utils::binary_storage<Manager::id_t>::iterator getBrotherOrSon<true>(mll::utils::binary_storage<Manager::id_t>::iterator it) {	return it.get_brother(); }

//=====================================================================================//
//  mll::utils::binary_storage<Manager::id_t>::iterator getBrotherOrSon<false>(); }    //
//=====================================================================================//
template<>
mll::utils::binary_storage<Manager::id_t>::iterator getBrotherOrSon<false>(mll::utils::binary_storage<Manager::id_t>::iterator it) { return it.get_son(); }

//=====================================================================================//
//                           findBrotherOrSonAtIdFlagPresent                           //
//=====================================================================================//
template<bool condition>
mll::utils::binary_storage<Manager::id_t>::iterator findBrotherOrSonAtIdFlagPresent(mll::utils::binary_storage<Manager::id_t>::iterator it,unsigned int flag,bool present)
{
	typedef mll::utils::binary_storage<Id>::iterator iterator;
	iterator iresult,iend;

	if(it != iend)
	{
		for(iresult = getBrotherOrSon<condition>(it);iresult!=iend;iresult.brother())
		{
			bool is_flag = (iresult->get_id().flags()&flag) != 0;
			if(is_flag == present) break;
		}
	}

	return iresult;
}

//=====================================================================================//
//        mll::utils::binary_storage<Manager::id_t>::iterator getFirstFolder()         //
//=====================================================================================//
mll::utils::binary_storage<Manager::id_t>::iterator getFirstFolder(mll::utils::binary_storage<Manager::id_t>::iterator it)
{
	return findBrotherOrSonAtIdFlagPresent<false>(it,Id::fFolder,true);
}

//=====================================================================================//
//         mll::utils::binary_storage<Manager::id_t>::iterator getFirstItem()          //
//=====================================================================================//
mll::utils::binary_storage<Manager::id_t>::iterator getFirstItem(mll::utils::binary_storage<Manager::id_t>::iterator it)
{
	return findBrotherOrSonAtIdFlagPresent<false>(it,Id::fFolder,false);
}

//=====================================================================================//
//         mll::utils::binary_storage<Manager::id_t>::iterator getNextFolder()         //
//=====================================================================================//
mll::utils::binary_storage<Manager::id_t>::iterator getNextFolder(mll::utils::binary_storage<Manager::id_t>::iterator it)
{
	return findBrotherOrSonAtIdFlagPresent<true>(it,Id::fFolder,true);
}

//=====================================================================================//
//          mll::utils::binary_storage<Manager::id_t>::iterator getNextItem()          //
//=====================================================================================//
mll::utils::binary_storage<Manager::id_t>::iterator getNextItem(mll::utils::binary_storage<Manager::id_t>::iterator it)
{
	return findBrotherOrSonAtIdFlagPresent<true>(it,Id::fFolder,false);
}

//=====================================================================================//
//          mll::utils::binary_storage<Manager::id_t>::iterator findSonById()          //
//=====================================================================================//
mll::utils::binary_storage<Manager::id_t>::iterator findSonById(mll::utils::binary_storage<Manager::id_t>::iterator it,const Manager::id_t& id)
{
	mll::utils::binary_storage<Manager::id_t>::iterator iend;

	for(it.son();it!=iend;it.brother())
		if(it->get_id() == id) break;

	return it;
}

}

//=====================================================================================//
//                                    class Manager                                    //
//=====================================================================================//
Manager::Manager(const version_t& v) : m_bs(v)
{
	m_bs.create_root(id_t("root",id_t::fFolder));
}

Manager::Manager(std::istream& stream) : m_bs(mll::io::ibinstream(stream))
{
}

Manager::~Manager()
{
}
//	��������� ������ � �����
void Manager::save(std::ostream& stream)
{
	m_bs.save(mll::io::obinstream(stream));
}
//	�������� ��������� �������� �� �����
Manager::folder_iterator Manager::begin(void)
{
	return folder_iterator(Utility::getFirstFolder(m_bs.begin()));
}
//	�������� �������� �������� �� �����
Manager::folder_iterator Manager::end(void)
{
	return folder_iterator();
}
//	������� ��������� � ������ ����� ���� ��� �� ���������� ��� �������� ��� ������������
Manager::folder_iterator Manager::ifolder(const std::string& id)
{
	const id_t folder_id = id_t(id,id_t::fFolder);
	bs_t::iterator ifolder = Utility::findSonById(m_bs.begin(),folder_id);

	if(ifolder == m_bs.end()) return folder_iterator(m_bs.begin()->create_node(folder_id));
	else return folder_iterator(ifolder);
}
//	������� ����� �� ������
void Manager::erase(folder_iterator it)
{
	m_bs.erase(it.m_iterator);
}
//	����� ����� �� ���������� ����
Manager::folder_iterator Manager::find(const path_t& path)
{
	return folder_iterator(m_bs.get_node(path_t(m_bs.begin()->get_id())+path));
}
//	�������� ������ �������
const Manager::version_t& Manager::version(void) const
{
	return m_bs.get_version();
}
//	�������� ������ ������� ��� ���������!
Manager::version_t& Manager::version(void)
{
	return m_bs.get_version();
}


//=====================================================================================//
//                                     class Item                                      //
//=====================================================================================//
Item::Item(bs_iterator iterator)
: m_buffer(std::string(reinterpret_cast<const char*>(iterator->get_data().get_data()),iterator->get_data().size()),
		   std::ios_base::in|std::ios_base::out|std::ios_base::binary),
  m_obinstream(m_buffer),
  m_ibinstream(m_buffer),
  m_iterator(iterator)
{
}

Item::~Item()
{
	flush();
}
//	��������� ���������� �������������� ������
void Item::flush(void)
{
	const std::string value = m_buffer.str();
	m_iterator->get_data().set_data(value.c_str(),value.size());
}
//	�������� ���������� �������������� ������
void Item::clear(void)
{
	m_buffer.str("");
}

//=====================================================================================//
//                                 class ItemIterator                                  //
//=====================================================================================//
ItemIterator::ItemIterator()
{
}

ItemIterator::ItemIterator(bs_iterator ibsnode)
: m_iterator(ibsnode)
{
	if(m_iterator != bs_iterator())
		createItemData(m_iterator);
}

ItemIterator::ItemIterator(const item_iterator& another)
: m_iterator(another.m_iterator),
  m_item(another.m_item)
{
}
//	�������� ������������
ItemIterator& ItemIterator::operator=(const item_iterator& another)
{
	if(&another != this)
	{
		m_iterator = another.m_iterator;
		m_item = another.m_item;
	}

	return *this;
}
//	���������������� (�������)
ItemIterator& ItemIterator::operator++(void)
{
	m_iterator = Utility::getNextItem(m_iterator);
	if(m_iterator != bs_iterator()) createItemData(m_iterator);
	else m_item.reset();

	return *this;
}
//	���������������� (��������)
ItemIterator ItemIterator::operator++(int)
{
	item_iterator iitem = *this;
	++(*this);
	return iitem;
}
//	���������
bool ItemIterator::operator==(const item_iterator& another)
{
	if(m_item.get() && another.m_item.get()) return (m_iterator == another.m_iterator) && (*m_item == *another.m_item);
	else return (m_iterator == another.m_iterator) && (m_item == another.m_item);
}
//	�����������
bool ItemIterator::operator!=(const item_iterator& another)
{
	return !(*this == another); 
}
//	������������ ������ �� �����
void ItemIterator::createItemData(bs_iterator inode)
{
	m_item = boost::shared_ptr<Item>(new Item(inode));
	m_item->id(inode->get_id().value());
	m_item->level(inode.level());
}

//=====================================================================================//
//                                    class Folder                                     //
//=====================================================================================//
Folder::Folder()
{
}

Folder::~Folder()
{
}

//=====================================================================================//
//                                class FolderIterator                                 //
//=====================================================================================//
FolderIterator::FolderIterator()
{
}

FolderIterator::FolderIterator(bs_iterator ibsnode)
: m_iterator(ibsnode)
{
	if(m_iterator != bs_iterator())
		createFolderData(m_iterator);
}

FolderIterator::FolderIterator(const folder_iterator& another)
: m_iterator(another.m_iterator),
  m_folder(another.m_folder)
{
}
//	�������� ������������
FolderIterator& FolderIterator::operator=(const folder_iterator& another)
{
	if(&another != this)
	{
		m_iterator = another.m_iterator;
		m_folder = another.m_folder;
	}

	return *this;
}
//	���������������� (�������)
FolderIterator::folder_iterator& FolderIterator::operator++(void)
{
	m_iterator = Utility::getNextFolder(m_iterator);
	if(m_iterator != bs_iterator()) createFolderData(m_iterator);
	else m_folder.reset();

	return *this;
}
//	���������������� (��������)
FolderIterator::folder_iterator FolderIterator::operator++(int)
{
	folder_iterator ifolder = *this;
	++(*this);
	return ifolder;
}
//	���������
bool FolderIterator::operator==(const folder_iterator& another)
{
	if(m_folder.get() && another.m_folder.get()) return (m_iterator == another.m_iterator) && (*m_folder == *another.m_folder);
	else return (m_iterator == another.m_iterator) && (m_folder == another.m_folder);
}
//	�����������
bool FolderIterator::operator!=(const folder_iterator& another)
{
	return !(*this == another); 
}
	///	������� ��������� � ������ ����� ���� ��� �� ���������� ��� �������� ��� ������������
FolderIterator::folder_iterator FolderIterator::ifolder(const std::string& id)
{
	const id_t folder_id = id_t(id,id_t::fFolder);
	bs_iterator ifolder = Utility::findSonById(m_iterator,folder_id);
	bs_iterator iend;

	if(ifolder == iend) return folder_iterator(m_iterator->create_node(folder_id));
	else return folder_iterator(ifolder);
}
//	�������� ��������� �������� �� ��������� �����
FolderIterator::folder_iterator FolderIterator::fbegin(void)
{
	return folder_iterator(Utility::getFirstFolder(m_iterator));
}
//	�������� �������� �������� �� ��������� �����
FolderIterator::folder_iterator FolderIterator::fend(void)
{
	return folder_iterator();
}
//	������� ��������� ����, ���� �� �� ����������, ��� �������� ��� ������������
FolderIterator::item_iterator FolderIterator::iitem(const std::string& id)
{
	const id_t item_id = id_t(id,0);
	bs_iterator iitem = Utility::findSonById(m_iterator,item_id);
	bs_iterator iend;

	if(iitem == iend) return item_iterator(m_iterator->create_node(item_id));
	else return item_iterator(iitem);
}
//	�������� ��������� �������� �� ��������� ����
FolderIterator::item_iterator FolderIterator::ibegin(void)
{
	return item_iterator(Utility::getFirstItem(m_iterator));
}
//	�������� �������� �������� �� ��������� ����
FolderIterator::item_iterator FolderIterator::iend(void)
{
	return item_iterator();
}
//	������������ ������ � �����
void FolderIterator::createFolderData(bs_iterator inode)
{
	m_folder = boost::shared_ptr<Folder>(new Folder());
	m_folder->id(inode->get_id().value());
	m_folder->level(inode.level());
}

}