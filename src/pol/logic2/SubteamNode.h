#pragma once

#include "AIEnemyNode.h"
#include "CompoundNode.h"
#include "GameObserver.h"

//=====================================================================================//
//                                  class SubteamNode                                  //
//=====================================================================================//
/// узел для управления подкомандой
class SubteamNode
:	public CompoundNode<AIEnemyNode,AINode>,
	private GameObserver
{
public:

	SubteamNode();
	SubteamNode(const std::string &teamname);
	~SubteamNode();

	DCTOR_DEF(SubteamNode)

	//сохранение/востановление подкоманды
	void MakeSaveLoad(SavSlot& st);

	// добавить узел в список узлов
	void addNode(Ptr_t node) { insert(node); }

	// разослать сообщение всем членам команды (кроме себя)
	void sendMessage(const AIEnemyNode * sender, AIEnemyNode::MessageType type, void * data, float radius = 0);

	// получить имя подкоманды
	std::string getName() const { return m_name; }

	// установить имя подкоманды
	void setName(const std::string& str)  { m_name = str; }

private:

 	virtual const char *getEndTurnMessage() const { return "SubteamNode: ход команды закончен."; }

	// имя подкоманды
	std::string m_name;

	//обработка начала хода
	void Update(subject_t subj, event_t event, info_t info);
};
