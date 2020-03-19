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
	// gvozdoder: запретим конструктор копирования и оператор присваивания
	MlResourceMgr(const MlResourceMgr &);
	MlResourceMgr &operator=(const MlResourceMgr &);

public:

	typedef std::list<MlResource*> container_t;
	typedef std::pair<container_t::iterator,container_t::iterator> slice_t;

private:

	static MlResourceMgr* m_instance;				//	единственный экземпляр класса

	// gvozdoder: убрал контейнер в дебри реализации для устранения варнинга
//	container_t m_container;						//	контейнер для ресурсов

	// gvozdoder: изоляция
	class Impl;
	Impl *m_pimpl;

public:
	MlResourceMgr();
	~MlResourceMgr();
public:
	//	получить срез ресурсов по типу ресурса
	slice_t slice(const class_info& ci);

private:
	//	поместить ресурс в менеджер (доступно только для класса MlResource)
	container_t::iterator insertResource(MlResource* value);
	//	удалить ресурс из менеджера (доступно только для класса MlResource)
	void removeResource(container_t::iterator handle);

	friend MlResource;

public:
	//	получить инстанцию класса
	static MlResourceMgr* instance(void);

};

//	получить инстанцию класса
inline MlResourceMgr* MlResourceMgr::instance(void)
{
	return m_instance;
}

#endif // !defined(__RESOURCEMGR_H_INCLUDED_8093570542662288__)