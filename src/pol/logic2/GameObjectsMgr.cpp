#include "logicdefs.h"
#include "GameObjectsMgr.h"
#include "AIUtils.h"
#include "DirtyLinks.h"

//=====================================================================================//
//                      GameObjectsMgr* GameObjectsMgr::GetInst()                      //
//=====================================================================================//
GameObjectsMgr* GameObjectsMgr::GetInst()
{
    static GameObjectsMgr imp;
    return &imp;
}

//=====================================================================================//
//                         void GameObjectsMgr::MakeSaveLoad()                         //
//=====================================================================================//
void GameObjectsMgr::MakeSaveLoad(SavSlot& slot)
{
    AIUtils::SaveLoadRIDSet(slot, m_used);
    AIUtils::SaveLoadRIDSet(slot, m_destroyed);

    if(slot.IsSaving())
	{
        rid_set_t::iterator itor = m_used.begin();
        while(itor != m_used.end())
		{
            slot << DirtyLinks::GetObjState(*itor);
            ++itor;
        }
    }
	else
	{
        bool state;

        rid_set_t::iterator itor = m_used.begin();
        while(itor != m_used.end())
		{
            slot >> state;

            DirtyLinks::SetObjState(*itor, state);
            ++itor;
        }

        itor = m_destroyed.begin();
        while(itor != m_destroyed.end())
		{
            //DirtyLinks::EraseLevelObject(*itor);
            ++itor;
        }
    }
}

//=====================================================================================//
//                         bool GameObjectsMgr::IsUsed() const                         //
//=====================================================================================//
bool GameObjectsMgr::IsUsed(const rid_t& rid) const
{
    return m_used.count(rid) != 0;
}

//=====================================================================================//
//                      bool GameObjectsMgr::IsDestroyed() const                       //
//=====================================================================================//
bool GameObjectsMgr::IsDestroyed(const rid_t& rid) const
{
    return m_destroyed.count(rid) != 0;
}

//=====================================================================================//
//                          void GameObjectsMgr::MarkAsUsed()                          //
//=====================================================================================//
void GameObjectsMgr::MarkAsUsed(const rid_t& rid)
{
    if(!IsUsed(rid)) m_used.insert(rid);
}

//=====================================================================================//
//                       void GameObjectsMgr::MarkAsDestroyed()                        //
//=====================================================================================//
void GameObjectsMgr::MarkAsDestroyed(const rid_t& rid)
{
    if(IsUsed(rid)) m_used.erase(rid);
    if(!IsDestroyed(rid)) m_destroyed.insert(rid);
}

//=====================================================================================//
//                            void GameObjectsMgr::Reset()                             //
//=====================================================================================//
void GameObjectsMgr::Reset()
{
    m_used.clear();
    m_destroyed.clear();
}
