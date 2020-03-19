#include "Vis.h"

namespace Vis
{

ML_PERSISTENT_IMPL2(PolyGroup,VIS_BASIC_PTAG);

//=====================================================================================//
//                               PolyGroup::PolyGroup()                                //
//=====================================================================================//
PolyGroup::PolyGroup()
:	m_name("**wrong_poly_group_name**"),
	m_isSorted(false)
{
}

//=====================================================================================//
//                               PolyGroup::PolyGroup()                                //
//=====================================================================================//
PolyGroup::PolyGroup(const PolyGroup &p)
:	m_name(p.getName()),
	m_isSorted(p.m_isSorted),
	m_polys(p.m_polys)
{
}

//=====================================================================================//
//                               PolyGroup::PolyGroup()                                //
//=====================================================================================//
PolyGroup::PolyGroup(const std::string &name)
:	m_name(name),
	m_isSorted(false)
{
}

//=====================================================================================//
//                               PolyGroup::PolyGroup()                                //
//=====================================================================================//
PolyGroup::PolyGroup(Storage::ReadSlot &slot)
:	Storage::PersistentBase(slot)
{
	m_isSorted = true;
	unsigned int size;
	slot.getStream() >> m_name;
	slot.getStream() >> size;

	std::transform(m_name.begin(),m_name.end(),m_name.begin(),std::tolower);

	m_polys.resize(size);
	for(unsigned int i = 0; i < m_polys.size(); ++i)
	{
		m_polys[i].m_channel = slot.restoreWeakReference<Vis::ChannelIndex16>();
		slot.getStream() >> m_polys[i].m_index;
	}

	sort();
}

//=====================================================================================//
//                               void PolyGroup::sort()                                //
//=====================================================================================//
void PolyGroup::sort()
{
	std::sort(m_polys.begin(),m_polys.end());
	m_isSorted = true;
}

//=====================================================================================//
//                              void PolyGroup::addPoly()                              //
//=====================================================================================//
void PolyGroup::addPoly(const PolyId &id)
{
	if(!isYourPoly(id))
	{
		m_polys.push_back(id);
		m_isSorted = false;
	}
}

//=====================================================================================//
//                            bool PolyGroup::isYourPoly()                             //
//=====================================================================================//
bool PolyGroup::isYourPoly(const PolyId &id)
{
	if(!m_isSorted) sort();
	return std::binary_search(m_polys.begin(),m_polys.end(),id);
}

//=====================================================================================//
//                               void PolyGroup::store()                               //
//=====================================================================================//
void PolyGroup::store(Storage::WriteSlot &slot) const
{
	slot.getStream() << m_name;
	slot.getStream() << m_polys.size();

	for(unsigned int i = 0; i < m_polys.size(); ++i)
	{
		slot.storeReference(m_polys[i].m_channel);
		slot.getStream() << m_polys[i].m_index;
	}
}

//=====================================================================================//
//                              void PolyGroup::migrate()                              //
//=====================================================================================//
void PolyGroup::migrate(Vis::ChannelBase *from, Vis::ChannelBase *to, unsigned short base)
{
	for(Polys_t::iterator i = m_polys.begin(); i != m_polys.end(); ++i)
	{
		if(i->m_channel.get() == from)
		{
			i->m_channel = static_cast<Vis::ChannelIndex16 *>(to);
			i->m_index += base;
		}
	}
	m_isSorted = false;
}

//=====================================================================================//
//                  std::pair<> PolyGroup::getPolyIdsOfPiece() const                   //
//=====================================================================================//
PolyGroup::PolyRange_t PolyGroup::getPolyIdsOfPiece(ChannelIndex16 *c) const
{
	return std::equal_range(m_polys.begin(),m_polys.end(),c,PolyId::Compare());
}

}