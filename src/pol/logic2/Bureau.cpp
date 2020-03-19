#include "logicdefs.h"
#include "Bureau.h"
#include "DirtyLinks.h"
#include "DataBox.h"
#include "StoreBox.h"

namespace BureauDetails
{

//=====================================================================================//
//                     class BureauBoxSpawner : public BoxSpawner                      //
//=====================================================================================//
class BureauBoxSpawner : public BoxSpawner
{
public:
	BureauBoxSpawner(Bureau* bureau) : m_bureau(bureau) {}

	void CreateDataBox(const rid_t& rid)
	{
		m_bureau->Insert(new DataBox(rid));
	}

	void CreateStoreBox(const rid_t& rid)
	{
		m_bureau->Insert(new StoreBox(rid));
	}

private:

	Bureau* m_bureau;
};

}

using namespace BureauDetails;

//=====================================================================================//
//                              Bureau* Bureau::GetInst()                              //
//=====================================================================================//
Bureau* Bureau::GetInst()
{
	static Bureau imp;
	return &imp;
}

//=====================================================================================//
//                                  Bureau::Bureau()                                   //
//=====================================================================================//
Bureau::Bureau()
{
}

//=====================================================================================//
//                                  Bureau::~Bureau()                                  //
//=====================================================================================//
Bureau::~Bureau()
{
	Reset();
}

//=====================================================================================//
//                               BaseBox* Bureau::Get()                                //
//=====================================================================================//
BaseBox* Bureau::Get(const rid_t& rid)
{
	for(size_t k = 0; k < m_boxes.size(); k++)
		if(m_boxes[k] && m_boxes[k]->GetRID() == rid)
			return m_boxes[k];

	return 0;
}

//=====================================================================================//
//                                void Bureau::Reset()                                 //
//=====================================================================================//
void Bureau::Reset()
{
	for(size_t k = 0; k < m_boxes.size(); delete m_boxes[k++]);
	m_boxes.clear();

	m_fcan_spawn = true;
}

//=====================================================================================//
//                             void Bureau::MakeSaveLoad()                             //
//=====================================================================================//
void Bureau::MakeSaveLoad(SavSlot& slot)
{
	if(slot.IsSaving())
	{
		//сохраним список ящиков
		for(size_t k = 0; k < m_boxes.size(); k++)
		{
			if(m_boxes[k])
			{
				slot << true;

				DynUtils::SaveObj(slot, m_boxes[k]);
				m_boxes[k]->MakeSaveLoad(slot);
			}
		}

		//признак конца
		slot << false;
		slot << m_fcan_spawn;
	}
	else
	{
		bool flag;

		slot >> flag;       
		while(flag)
		{
			m_boxes.push_back(0);
			DynUtils::LoadObj(slot, m_boxes.back());           
			m_boxes.back()->MakeSaveLoad(slot);

			slot >> flag;           
		}

		slot >> m_fcan_spawn;
	}
}

//=====================================================================================//
//                                void Bureau::Insert()                                //
//=====================================================================================//
void Bureau::Insert(BaseBox* new_box)
{
	box_vec_t::iterator i = std::find(m_boxes.begin(),m_boxes.end(),(BaseBox*)0);
	if(i == m_boxes.end())
	{
		m_boxes.push_back(new_box);
	}
	else
	{
		*i = new_box;
	}
}

//=====================================================================================//
//                           void Bureau::HandleLevelEntry()                           //
//=====================================================================================//
void Bureau::HandleLevelEntry()
{
	if(m_fcan_spawn)
	{
		m_fcan_spawn = false;

		BureauBoxSpawner bureau_spawner(this);
		DirtyLinks::EnumerateAllLevelCases(&bureau_spawner);
	}
}

//=====================================================================================//
//                          void Bureau::HandleObjectDamage()                          //
//=====================================================================================//
void Bureau::HandleObjectDamage(const rid_t& rid)
{
	for(size_t k = 0; k < m_boxes.size(); k++)
	{
		if(m_boxes[k] && m_boxes[k]->GetRID() == rid)
		{
			delete m_boxes[k];
			m_boxes[k] = 0;
			return;
		}
	}
}
