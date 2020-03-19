#pragma once

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: основные структуры AI
                                                                                
                                                                                
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
// получить по метке описание узла AI
void getAINodeDesc(const std::string& label, std::string& type, std::string& subteam);
}

//=====================================================================================//
//                                   class EnemyRoot                                   //
//=====================================================================================//
/// корень дерева для врагов
class EnemyRoot
:	public CompoundNode<SubteamNode>, 
	private GameObserver,
	private SpawnObserver
{
public:
	EnemyRoot();
	~EnemyRoot();

    DCTOR_DEF(EnemyRoot)

    //сохранение/востановление AI
    void MakeSaveLoad(SavSlot& st);

private:
    
 	virtual const char *getEndTurnMessage() const { return "EnemyRoot: ход врагов закончен."; }
    //обработка расстановки персонажа
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка начала хода
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //создать узел AI по строке параметров
	boost::shared_ptr<AIEnemyNode> CreateNode(BaseEntity* entity, const std::string type);
};

//=====================================================================================//
//                                   class CivilRoot                                   //
//=====================================================================================//
/// корень дерева для штатских
class CivilRoot
:	public CompoundNode<AINode,AINode>,
	private GameObserver, 
	private SpawnObserver
{
public:

    CivilRoot();
    ~CivilRoot();

    DCTOR_DEF(CivilRoot)

    //сохранение/востановление AI
    void MakeSaveLoad(SavSlot& st);

private:
    
 	virtual const char *getEndTurnMessage() const { return "CivilRoot: ход мирных жителей закончен."; }
   //обработка расстановки персонажа
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //обработка начала хода
    void Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t info);

    //создать узел AI по строке параметров
	Ptr_t CreateNode(BaseEntity* entity);

    //перевести все в нижний регистр
    void ToLower(std::string& str);
    //получить слово в  строке
    void CutWord(std::string& word, std::string& str);
};
