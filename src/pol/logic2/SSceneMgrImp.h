#pragma once

#include "Spawn.h"
#include "SScene.h"
#include "GameObserver.h"

class ScriptSceneFactory;

//=====================================================================================//
//                        class SSceneMgrImp : public SSceneMgr                        //
//=====================================================================================//
// ����� ��� ���������� � ������������ ���������� ����
class SSceneMgrImp : public SSceneMgr, private GameObserver, private SpawnObserver
{
    //typedef std::vector<ScriptSceneFactory*> factory_vec_t;
    //factory_vec_t m_factories;
public:
    //�������� ���������� ����� ��� ������������
    //Activity* CreateScriptScene();

    //��������� ����������� �����
    void MakeSaveLoad(SavSlot& slot);

    void Init();
    void Shut();
    //bool IsScriptSceneFinished(const rid_t& rid) const;

private:
    void MakeSave(SavSlot& slot);
    void MakeLoad7(SavSlot& slot);

    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info);
    void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info);
};
