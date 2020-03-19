#pragma once

#include "AIEnemyNode.h"
#include "CompoundNode.h"
#include "GameObserver.h"

//=====================================================================================//
//                                  class SubteamNode                                  //
//=====================================================================================//
/// ���� ��� ���������� �����������
class SubteamNode
:	public CompoundNode<AIEnemyNode,AINode>,
	private GameObserver
{
public:

	SubteamNode();
	SubteamNode(const std::string &teamname);
	~SubteamNode();

	DCTOR_DEF(SubteamNode)

	//����������/������������� ����������
	void MakeSaveLoad(SavSlot& st);

	// �������� ���� � ������ �����
	void addNode(Ptr_t node) { insert(node); }

	// ��������� ��������� ���� ������ ������� (����� ����)
	void sendMessage(const AIEnemyNode * sender, AIEnemyNode::MessageType type, void * data, float radius = 0);

	// �������� ��� ����������
	std::string getName() const { return m_name; }

	// ���������� ��� ����������
	void setName(const std::string& str)  { m_name = str; }

private:

 	virtual const char *getEndTurnMessage() const { return "SubteamNode: ��� ������� ��������."; }

	// ��� ����������
	std::string m_name;

	//��������� ������ ����
	void Update(subject_t subj, event_t event, info_t info);
};
