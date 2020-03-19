#include "precomp.h"
#include "StoreBox2.h"
#include <logic2/Thing.h>

namespace PoL
{

//=====================================================================================//
//                               StoreBox2::StoreBox2()                                //
//=====================================================================================//
StoreBox2::StoreBox2()
:	m_fullId("ѕлохойЅоксЌа”ровне"),
	m_maxWeight(-1.0f)
{
}

//=====================================================================================//
//                               StoreBox2::StoreBox2()                                //
//=====================================================================================//
StoreBox2::StoreBox2(const std::string &fullId, float maxWeight)
:	m_fullId(fullId),
	m_maxWeight(maxWeight),
	m_curWeight(0.0f)
{
}

//=====================================================================================//
//                               StoreBox2::~StoreBox2()                               //
//=====================================================================================//
StoreBox2::~StoreBox2()
{
	std::for_each(m_things.begin(),m_things.end(),boost::checked_delete<BaseThing>);
}

//=====================================================================================//
//                   StoreBox2::Iterator_t StoreBox2::thingsBegin()                    //
//=====================================================================================//
StoreBox2::Iterator_t StoreBox2::thingsBegin() const
{
	return m_things.begin();
}

//=====================================================================================//
//                    StoreBox2::Iterator_t StoreBox2::thingsEnd()                     //
//=====================================================================================//
StoreBox2::Iterator_t StoreBox2::thingsEnd() const
{
	return m_things.end();
}

//=====================================================================================//
//                                void StoreBox2::add()                                //
//=====================================================================================//
void StoreBox2::add(BaseThing *thing)
{
	assert( std::find(m_things.begin(),m_things.end(),thing) == m_things.end() );
	m_things.push_back(thing);
	updateCurWeight();
}

//=====================================================================================//
//                              void StoreBox2::remove()                               //
//=====================================================================================//
void StoreBox2::remove(BaseThing *thing)
{
	Things_t::iterator i = std::find(m_things.begin(),m_things.end(),thing);
	if(m_things.end() != i) m_things.erase(i);
	updateCurWeight();
}

//=====================================================================================//
//                          void StoreBox2::updateCurWeight()                          //
//=====================================================================================//
void StoreBox2::updateCurWeight()
{
	m_curWeight = 0.0f;

	for(Things_t::iterator i = m_things.begin(); i != m_things.end(); ++i)
	{
		m_curWeight += (*i)->GetWeight();
	}
}

//=====================================================================================//
//                            void StoreBox2::store() const                            //
//=====================================================================================//
void StoreBox2::store(mll::io::obinstream &out) const
{
	out << m_fullId;
	out << m_maxWeight;
	out << m_curWeight;

	out << m_things.size();

	for(Things_t::const_iterator i = m_things.begin(); i != m_things.end(); ++i)
	{
		SavSlotStr slot;
		DynUtils::SaveObj(slot,*i);
		(*i)->MakeSaveLoad(slot);
		
		std::string data = slot.GetData();
		out << data.size();
		out.stream().write(&data[0],data.size());
	}
}

//=====================================================================================//
//                              void StoreBox2::restore()                              //
//=====================================================================================//
void StoreBox2::restore(mll::io::ibinstream &in)
{
	assert( m_things.empty() );

	in >> m_fullId;
	in >> m_maxWeight;
	in >> m_curWeight;

	size_t tcount;
	in >> tcount;

	while(tcount--)
	{
		size_t datasize;
		in >> datasize;

		std::string data;
		data.resize(datasize);

		in.stream().read(&data[0],datasize);
		SavSlotStr slot(data);

		BaseThing *thing = 0;
		DynUtils::LoadObj(slot,thing);
		thing->MakeSaveLoad(slot);
		m_things.push_back(thing);
	}
}

}
