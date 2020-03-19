#if !defined(__RESOURCEMGR_H_INCLUDED_8093570542662288__)
#define __RESOURCEMGR_H_INCLUDED_8093570542662288__

#include <mutual/_export_rules.h>
#include <mutual/novtable.h>
#include <mutual/rtti.h>
#include <boost/utility.hpp>
#include <list>

class MlResource;

//=====================================================================================//
//                                 class MlResourceMgr                                 //
//=====================================================================================//
class MUTUAL_API MlResourceMgr
{
	// gvozdoder: �������� ����������� ����������� � �������� ������������
	MlResourceMgr(const MlResourceMgr &);
	MlResourceMgr &operator=(const MlResourceMgr &);

public:

	typedef std::list<MlResource*> container_t;
	typedef std::pair<container_t::iterator,container_t::iterator> slice_t;

private:

	static MlResourceMgr* m_instance;				//	������������ ��������� ������

	// gvozdoder: ����� ��������� � ����� ���������� ��� ���������� ��������
//	container_t m_container;						//	��������� ��� ��������

	// gvozdoder: ��������
	class Impl;
	Impl *m_pimpl;

public:
	MlResourceMgr();
	~MlResourceMgr();
public:
	//	�������� ���� �������� �� ���� �������
	slice_t slice(const class_info& ci);

private:
	//	��������� ������ � �������� (�������� ������ ��� ������ MlResource)
	container_t::iterator insertResource(MlResource* value);
	//	������� ������ �� ��������� (�������� ������ ��� ������ MlResource)
	void removeResource(container_t::iterator handle);

	friend MlResource;

public:
	//	�������� ��������� ������
	static MlResourceMgr* instance(void);

};

//	�������� ��������� ������
inline MlResourceMgr* MlResourceMgr::instance(void)
{
	return m_instance;
}

#endif // !defined(__RESOURCEMGR_H_INCLUDED_8093570542662288__)