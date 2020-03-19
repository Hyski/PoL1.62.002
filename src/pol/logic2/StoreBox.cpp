#include "logicdefs.h"

#include "Thing.h"
#include "StoreBox.h"
#include "DirtyLinks.h"
#include "BoxVisitor.h"

DCTOR_IMP(StoreBox)

//=====================================================================================//
//                                StoreBox::StoreBox()                                 //
//=====================================================================================//
StoreBox::StoreBox(const rid_t& rid)
:	BaseBox(rid),
	m_pack(1)
{
}

//=====================================================================================//
//                                StoreBox::~StoreBox()                                //
//=====================================================================================//
StoreBox::~StoreBox()
{
	for(size_t k = 0; k < m_pack.size(); delete m_pack[k++]);
}

//=====================================================================================//
//                          StoreBox* StoreBox::Cast2Store()                           //
//=====================================================================================//
StoreBox* StoreBox::Cast2Store()
{
	return this;
}

//=====================================================================================//
//                               void StoreBox::Accept()                               //
//=====================================================================================//
void StoreBox::Accept(BoxVisitor& visitor)
{
	visitor.Visit(this);
}

//=====================================================================================//
//                            void StoreBox::MakeSaveLoad()                            //
//=====================================================================================//
void StoreBox::MakeSaveLoad(SavSlot& slot)
{
	BaseBox::MakeSaveLoad(slot);

	if(slot.IsSaving())
	{
		for(size_t k = 0; k < m_pack.size(); k++)
		{
			if(m_pack[k])
			{
				slot << k;
				DynUtils::SaveObj(slot, m_pack[k]);
				m_pack[k]->MakeSaveLoad(slot);
			}
		}

		//признак конца
		slot << 0;
	}
	else
	{
		tid_t tid;

		slot >> tid;
		while(tid)
		{
			m_pack.resize(tid + 1);            
			DynUtils::LoadObj(slot, m_pack[tid]);
			m_pack[tid]->MakeSaveLoad(slot);

			slot  >> tid;
		}
	}
}

////=====================================================================================//
////                               void StoreBox::Remove()                               //
////=====================================================================================//
//void StoreBox::Remove(BaseThing* thing)
//{
//	if(m_pack[thing->GetTID()] == thing)
//	{
//		m_pack[thing->GetTID()] = 0;
//		thing->SetTID(0);
//		thing->SetEntity(0);
//	}
//}

//=====================================================================================//
//                              tid_t StoreBox::Insert()                               //
//=====================================================================================//
tid_t StoreBox::Insert(BaseThing* thing)
{
	for(size_t k = 1; k < m_pack.size(); k++)
	{
		if(m_pack[k] == 0)
		{
			m_pack[k] = thing;
			thing->SetTID(k);
			thing->SetEntity(0);
			return k;        
		}
	}

	thing->SetTID(m_pack.size());
	thing->SetEntity(0);
	m_pack.push_back(thing);
	return thing->GetTID();
}

//=====================================================================================//
//                          float StoreBox::GetWeight() const                          //
//=====================================================================================//
float StoreBox::GetWeight() const
{
	return GetInfo()->GetWeight();
}

//=====================================================================================//
//                          int StoreBox::GetRespawn() const                           //
//=====================================================================================//
int StoreBox::GetRespawn() const
{
	return GetInfo()->GetRespawn();
}

//=====================================================================================//
//                         int StoreBox::GetExperience() const                         //
//=====================================================================================//
int StoreBox::GetExperience() const
{
	return GetInfo()->GetExperience();
}

//=====================================================================================//
//                     const rid_t& StoreBox::GetScriptRID() const                     //
//=====================================================================================//
const rid_t& StoreBox::GetScriptRID() const
{
	return GetInfo()->GetItems();
}

//=====================================================================================//
//                        StoreBox::iterator StoreBox::begin()                         //
//=====================================================================================//
StoreBox::iterator StoreBox::begin(unsigned mask)
{
	return iterator(&m_pack, 0, mask);
}

//=====================================================================================//
//                         StoreBox::iterator StoreBox::end()                          //
//=====================================================================================//
StoreBox::iterator StoreBox::end()
{
	return iterator(0, m_pack.size(), 0);
}

//=====================================================================================//
//                           StoreBox::Iterator::Iterator()                            //
//=====================================================================================//
StoreBox::Iterator::Iterator(StoreBox::pack_t* pack, int first, unsigned mask)
:	m_pack(pack),
	m_mask(mask),
	m_first(first)
{
	if(m_pack && !IsSuitable((*m_pack)[m_first])) operator++();
}

//=====================================================================================//
//                StoreBox::Iterator& StoreBox::Iterator::operator ++()                //
//=====================================================================================//
StoreBox::Iterator& StoreBox::Iterator::operator ++()
{
	for(m_first++; m_first < m_pack->size(); m_first++)
	{
		if(IsSuitable((*m_pack)[m_first]))
		{
			return *this;
		}
	}

	m_first = m_pack->size(); 
	return *this;
}

//=====================================================================================//
//                     bool StoreBox::Iterator::IsSuitable() const                     //
//=====================================================================================//
bool StoreBox::Iterator::IsSuitable(const BaseThing* thing) const
{
	return thing && thing->GetInfo()->GetType() & m_mask; 
}
