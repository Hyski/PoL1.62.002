#pragma once

#include "AINode.h"
#include "AIDefs.h"

class SubteamNode;

//=====================================================================================//
//                                  class AIEnemyNode                                  //
//=====================================================================================//
/// класс - узел интеллекта всех врагов
class AIEnemyNode : public AINode, public AIDefs
{
public:
	// тип сообщения
	enum MessageType
	{
		MT_ENEMYSPOTTED,
		MT_NEEDCURE
	};

	AIEnemyNode() : m_subteam(0) {}

	virtual ~AIEnemyNode(){}
	// получить сообщение от другого члена подкоманды
	virtual void recieveMessage(MessageType type, void * data) = 0;
	// получить текущую позицию существа, связанного с узлом
	virtual point3 getPosition() const = 0;
	// получить указатель на подкоманду
	SubteamNode* getSubteam() const { return m_subteam; }
	// установить указатель на подкоманду
	void setSubteam(SubteamNode* subteam) { m_subteam = subteam; }
	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

//protected:
//	virtual bool thinkFn(state_type *) = 0;

private:
	// указатель на подкоманду
	SubteamNode* m_subteam;
};
