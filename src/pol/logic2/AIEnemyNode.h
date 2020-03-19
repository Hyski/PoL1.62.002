#pragma once

#include "AINode.h"
#include "AIDefs.h"

class SubteamNode;

//=====================================================================================//
//                                  class AIEnemyNode                                  //
//=====================================================================================//
/// ����� - ���� ���������� ���� ������
class AIEnemyNode : public AINode, public AIDefs
{
public:
	// ��� ���������
	enum MessageType
	{
		MT_ENEMYSPOTTED,
		MT_NEEDCURE
	};

	AIEnemyNode() : m_subteam(0) {}

	virtual ~AIEnemyNode(){}
	// �������� ��������� �� ������� ����� ����������
	virtual void recieveMessage(MessageType type, void * data) = 0;
	// �������� ������� ������� ��������, ���������� � �����
	virtual point3 getPosition() const = 0;
	// �������� ��������� �� ����������
	SubteamNode* getSubteam() const { return m_subteam; }
	// ���������� ��������� �� ����������
	void setSubteam(SubteamNode* subteam) { m_subteam = subteam; }
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

//protected:
//	virtual bool thinkFn(state_type *) = 0;

private:
	// ��������� �� ����������
	SubteamNode* m_subteam;
};
