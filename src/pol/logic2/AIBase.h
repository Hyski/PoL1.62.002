#pragma once

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� ��������� AI
                                                                                
                                                                                
   Author:   Alexander Garanin (Punch)
				Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "GameObserver.h"
#include "CompoundNode.h"
#include "SubteamNode.h"
//#include "EnemyAI.h"
#include "AINode.h"
#include "Spawn.h"

class Activity;
class BusRoute;
class CheckPoints;

class AIEnemyNode;

namespace AIBase
{
// �������� �� ����� �������� ���� AI
void getAINodeDesc(const std::string& label, std::string& type, std::string& subteam);
}

//=====================================================================================//
//                                   class EnemyRoot                                   //
//=====================================================================================//
/// ������ ������ ��� ������
class EnemyRoot
:	public CompoundNode<SubteamNode>, 
	private GameObserver,
	private SpawnObserver
{
public:
	EnemyRoot();
	~EnemyRoot();

    DCTOR_DEF(EnemyRoot)

    //����������/������������� AI
    void MakeSaveLoad(SavSlot& st);

private:
    
 	virtual const char *getEndTurnMessage() const { return "EnemyRoot: ��� ������ ��������."; }
    //��������� ����������� ���������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ������ ����
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //������� ���� AI �� ������ ����������
	boost::shared_ptr<AIEnemyNode> CreateNode(BaseEntity* entity, const std::string type);
};

//=====================================================================================//
//                                   class CivilRoot                                   //
//=====================================================================================//
/// ������ ������ ��� ��������
class CivilRoot
:	public CompoundNode<AINode,AINode>,
	private GameObserver, 
	private SpawnObserver
{
public:

    CivilRoot();
    ~CivilRoot();

    DCTOR_DEF(CivilRoot)

    //����������/������������� AI
    void MakeSaveLoad(SavSlot& st);

private:
    
 	virtual const char *getEndTurnMessage() const { return "CivilRoot: ��� ������ ������� ��������."; }
   //��������� ����������� ���������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ������ ����
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //������� ���� AI �� ������ ����������
	Ptr_t CreateNode(BaseEntity* entity);

    //��������� ��� � ������ �������
    void ToLower(std::string& str);
    //�������� ����� �  ������
    void CutWord(std::string& word, std::string& str);
};
