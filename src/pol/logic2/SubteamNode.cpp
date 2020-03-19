#include "logicdefs.h"
#include "SubteamNode.h"

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������� �����������
//
/////////////////////////////////////////////////////////////////////////////
DCTOR_IMP(SubteamNode)

//=====================================================================================//
//                             SubteamNode::SubteamNode()                              //
//=====================================================================================//
SubteamNode::SubteamNode()
{
	GameEvMessenger::GetInst()->Attach(this, EV_TURN);
}

//=====================================================================================//
//                             SubteamNode::SubteamNode()                              //
//=====================================================================================//
SubteamNode::SubteamNode(const std::string &teamname)
:	m_name(teamname)
{
	GameEvMessenger::GetInst()->Attach(this, EV_TURN);
}

//=====================================================================================//
//                             SubteamNode::~SubteamNode()                             //
//=====================================================================================//
SubteamNode::~SubteamNode()
{
	GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                          void SubteamNode::MakeSaveLoad()                           //
//=====================================================================================//
//����������/������������� ����������
void SubteamNode::MakeSaveLoad(SavSlot& st)
{
    //��������/����������� ��������� ����������
	if(st.IsSaving())
	{
		// �����������
		st << m_name;
		st << size();

		for(Iterator_t it = begin(); it != end(); ++it)
		{
			DynUtils::SaveObj(st, it->get());
			(*it)->MakeSaveLoad(st);
		}
	}
	else
	{
		// ��������
		st >> m_name;
		int num;
		st >> num;
		for(int i = 0; i < num; i++)
		{
			AIEnemyNode* node = 0;
			DynUtils::LoadObj(st, node);
			node->MakeSaveLoad(st);
			node->setSubteam(this);
			insert(Ptr_t(node));
		}
	}
}

//=====================================================================================//
//                             void SubteamNode::Update()                              //
//=====================================================================================//
/// ��������� ������ ����
void SubteamNode::Update(subject_t subj, event_t event, info_t info)
{
    if(static_cast<turn_info*>(info)->m_player == PT_ENEMY)
	{
		resetCur();
	}

	resetComplexity();
}

//=====================================================================================//
//                           void SubteamNode::sendMessage()                           //
//=====================================================================================//
/// ��������� ��������� ���� ������ ������� (����� ����)
void SubteamNode::sendMessage(const AIEnemyNode * sender, AIEnemyNode::MessageType type, void * data, float radius/*=0*/)
{
	for(Iterator_t i = begin(); i != end(); ++i)
	{
		const float dist = ((*i)->getPosition() - sender->getPosition()).Length();
		if(i->get() != sender && radius >= dist)
		{
			(*i)->recieveMessage(type,data);
		}
	}
}
