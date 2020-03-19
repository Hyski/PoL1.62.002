#include "precomp.h"

#include "VisObj.h"
#include "VisObjMgr.h"

namespace PoL
{

static const int g_initialObjectCount = 1024;
static const int g_additionalObjectCount = 128;

//=====================================================================================//
//                       void VisObjMgr::VisObjData::setObject()                       //
//=====================================================================================//
void VisObjMgr::VisObjData::setObject(VisObj *obj)
{
	m_object = obj;
	m_bboxDirty = true;
}

//=====================================================================================//
//                               VisObjMgr::VisObjMgr()                                //
//=====================================================================================//
VisObjMgr::VisObjMgr()
:	m_objects(g_initialObjectCount),
	m_freeList(g_initialObjectCount,0l),
	m_dirtyBoxCount(0),
	m_dataCtr(1)
{
	for(int i = 0; i < m_freeList.size(); ++i)
	{
		m_freeList[i] = m_freeList.size() - i - 1;
	}
}

//=====================================================================================//
//                               VisObjMgr::~VisObjMgr()                               //
//=====================================================================================//
VisObjMgr::~VisObjMgr()
{
}

//=====================================================================================//
//                            VisObjMgr *VisObjMgr::inst()                             //
//=====================================================================================//
VisObjMgr *VisObjMgr::inst()
{
	static VisObjMgr instance;
	return &instance;
}

//=====================================================================================//
//                             void VisObjMgr::addObject()                             //
//=====================================================================================//
void VisObjMgr::addObject(VisObj *visobj)
{
	assert( visobj->getId() == VisObj::NullId );

	if(m_freeList.empty()) enlarge();

	const long id = m_freeList.back();
	visobj->assignId(id);
	m_objects[id].setObject(visobj);
	++m_dirtyBoxCount;
	m_freeList.pop_back();

	updateObjPlace(&m_objects[id]);
}

//=====================================================================================//
//                           void VisObjMgr::removeObject()                            //
//=====================================================================================//
void VisObjMgr::removeObject(VisObj *visobj)
{
	assert( visobj->getId() != VisObj::NullId );

	const long id = visobj->getId();
	m_objects[id].setObject(0);
	m_freeList.push_back(id);
	visobj->assignId(VisObj::NullId);
}

//=====================================================================================//
//                          void VisObjMgr::setBboxChanged()                           //
//=====================================================================================//
void VisObjMgr::setBboxChanged(VisObj *visobj)
{
	VisObjData &vod = m_objects[visobj->getId()];

	if(!vod.m_bboxDirty)
	{
		vod.m_bboxDirty = true;
		++m_dirtyBoxCount;
	}
}

//=====================================================================================//
//                              void VisObjMgr::enlarge()                              //
//=====================================================================================//
void VisObjMgr::enlarge()
{
	assert( m_freeList.empty() );

	m_freeList.reserve(m_freeList.capacity() + g_additionalObjectCount);
	m_freeList.resize(g_additionalObjectCount);

	for(int i = 0; i < m_freeList.size(); ++i)
	{
		m_freeList[i] = m_objects.size() - i - 1;
	}

	m_objects.resize(m_objects.size() + g_additionalObjectCount);
}

//=====================================================================================//
//                         void VisObjMgr::updateDirtyBoxes()                          //
//=====================================================================================//
void VisObjMgr::updateDirtyBoxes()
{
	if(!m_dirtyBoxCount) return;

	for(Objects_t::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
	{
		if(i->m_object && i->m_bboxDirty)
		{
			updateObjPlace(&*i);
		}
	}

	assert( m_dirtyBoxCount == 0 );
}

//=====================================================================================//
//                              void VisObjMgr::render()                               //
//=====================================================================================//
void VisObjMgr::render(GraphPipe *pipe, bool transp)
{
	updateDirtyBoxes();

	for(Objects_t::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
	{
		if(i->m_object)
		{
			i->m_object->render(pipe,transp);
		}
	}
}

//=====================================================================================//
//                          void VisObjMgr::updateObjPlace()                           //
//=====================================================================================//
void VisObjMgr::updateObjPlace(VisObjData *obj)
{
	assert( obj->m_bboxDirty );

	obj->m_bbox = obj->m_object->getBbox();
	obj->m_bboxDirty = false;
	--m_dirtyBoxCount;
}

}
