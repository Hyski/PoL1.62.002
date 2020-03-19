#pragma warning(disable:4786)
#include "logicdefs.h"
#include "questengine.h"
#include "questengine_loc.h"
#include "dirtylinks.h"
//#include "phrasemanager.h"

/***************************************************************************************
����������
***************************************************************************************/
QuestEngine::QuestList QuestEngine::Quests;
int QuestEngine::m_Episode;

//=====================================================================================//
//                                  class crit_enter                                   //
//=====================================================================================//
class crit_enter
{
private:
	bool &e;
public:
	crit_enter(bool &flag) : e(flag) { e = true; }
	~crit_enter() { e = false; }
};

//fixme: ������, �� ������� �������. ���������� ������� ���-�� ������
std::set<std::string> HerosAskedForJoin;

//=====================================================================================//
//                          void QuestEngine::HandleMessage()                          //
//=====================================================================================//
void QuestEngine::HandleMessage(QuestMessage1 &mes)
{
	static bool entered=false;
	if(entered) return;
	crit_enter __ce__(entered);

	bool any_phrase_said=false; //���� ������ ���-���� ������� - ������ ����� ������.
	HerosAskedForJoin.clear();
#ifdef QUESTS_LOGGING
	bool HolderNotFoe=(mes.Subject->GetRelation2Player())!=RT_ENEMY);
	QuestLog("=> Handling Message:%s actor:%s subject:%s%s\n",(mes.Type==QuestMessage1::MT_USE||mes.Type==QuestMessage1::MT_FIRSTUSE)?"Use":"Kill",mes.Actor.c_str(), mes.Subject->GetSysName().c_str(),HolderNotFoe?"(Friend)":" (Foe)");
#endif

	bool first_use=MessageBase::IsUsed(mes.Subject->GetSysName());
	MessageBase::AddMessage(mes);

	//��������� ����� �� ������ ����� �������� ���� ���������
	QuestList::iterator it=Quests.begin(),ite=Quests.end();
	for(;it!=ite;it++)
	{
		(*it)->CheckDependencies(&mes);
	}

	//�������� ��������� ������� ������
	bool UseMes=(mes.Type==QuestMessage1::MT_USE||mes.Type==QuestMessage1::MT_FIRSTUSE);
	for(it=Quests.begin();it!=ite;it++)
	{
		Quest *q=*it;
		any_phrase_said = q->CheckState(&mes) || any_phrase_said;
	}
	//���� �� ����� ����� �� �������, ������� �����������
	if(/*!any_phrase_said&&*/UseMes)
	{
		if(MessageBase::isHeroWantsJoin(mes.Subject->GetSysName()) && !Q2LUtilities::IsHeroInTeam(mes.Subject))
		{
			if((HerosAskedForJoin.end()==HerosAskedForJoin.find(mes.Subject->GetSysName())))
			{
				if(mes.serv->ShowMessageBox(mlprintf(DirtyLinks::GetStrRes("gqs_add_hero").c_str(),mes.Subject->GetName())))
					mes.Subject->ChangePlayer(PT_PLAYER);
				HerosAskedForJoin.insert(mes.Subject->GetSysName());
			}
		}
		// else
		{
			if(!any_phrase_said /*&& !Q2LUtilities::IsHeroInTeam(mes.Subject)*/)
			{
				mes.Subject->SayPhrase(first_use ? LogicServer1::EntityManipulator::PT_USE : LogicServer1::EntityManipulator::PT_FIRST);
				//mes.m_SayPhraseType=first_use ? QuestMessage::PT_USUAL : QuestMessage::PT_FIRST;
			}
			// mes.serv->SayActorPhrase(mes.Subject,first_use ? LogicServer::PT_USUAL : LogicServer::PT_FIRST);

		}
	}

#ifdef QUESTS_LOGGING
	for(it=Quests.begin();it!=ite;it++)
	{
		Quest *q=*it;
		std::string str,str1;
		if(q->m_CurState==QS_NOTSTARTED)  str="Not started";
		else if(q->m_CurState==QS_STARTED)  str="Started";
		else if(q->m_CurState==QS_PASSED)  str="Passed";
		else if(q->m_CurState==QS_PASSED_OK)  str="Passed OK";
		else if(q->m_CurState==QS_FAILED)  str="Failed";
		else if(q->m_CurState==QS_COMPLETE)  str="Complete";

		if(q->m_AwaitingStatus==Quest::AS_MAYSTART)  str1="May start";
		else if(q->m_AwaitingStatus==Quest::AS_WAIT)  str1="Waiting";
		else if(q->m_AwaitingStatus==Quest::AS_NEVER)  str1="Can't start";
		else if(q->m_AwaitingStatus==Quest::AS_STARTED)  str1="Started";
		QuestLog("Quest: %s - %s  (%s)\n", q->m_Speeches.Name.c_str(), str.c_str(),str1.c_str());
	}
#endif

}

//=====================================================================================//
//                              Quest *QuestEngine::Get()                              //
//=====================================================================================//
Quest *QuestEngine::Get(const std::string &sys_name)
{
	QuestList::iterator it=Quests.begin(),ite=Quests.end();
	for(;it!=ite;it++)
	{
		if((*it)->m_SysName==sys_name) return (*it);
	}
	return NULL;
}

//=====================================================================================//
//                      QUEST_STATE QuestEngine::GetQuestState()                       //
//=====================================================================================//
QUEST_STATE QuestEngine::GetQuestState(const std::string &sys_name)
{
	Quest *q=Get(sys_name);
	if(q)
	{
		return q->GetState();
	}
	else
	{
		return QS_NOTSTARTED;
	}
}

//=====================================================================================//
//                              void QuestEngine::Init()                               //
//=====================================================================================//
void QuestEngine::Init()
{
	Clear();
}

//=====================================================================================//
//                              void QuestEngine::Close()                              //
//=====================================================================================//
void QuestEngine::Close()
{
	Clear();
}

//=====================================================================================//
//                              void QuestEngine::Clear()                              //
//=====================================================================================//
void QuestEngine::Clear()
{
	QuestList::iterator it=Quests.begin(),ite=Quests.end();
	for(;it!=ite;it++)
	{
		delete *it;
	}
	Quests.clear();
	MessageBase::Clear();
}

//=====================================================================================//
//                          void QuestEngine::MakeSaveLoad()                           //
//=====================================================================================//
void QuestEngine::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{
		slot<<m_Episode;

		QuestList::iterator it=Quests.begin(),ite=Quests.end();
		int q_num=0;
		for(it=Quests.begin();it!=ite;it++)
		{//����� ��������� ������ ������, ���-���� ���������� ���������
			Quest *q=*it;
			if(q->m_CurState!=QS_NOTSTARTED||q->m_AwaitingStatus!=Quest::AS_WAIT)
				q_num++;
		}
		slot<<q_num;
		for(it=Quests.begin();it!=ite;it++)
		{
			Quest *q=*it;
			if(q->m_CurState!=QS_NOTSTARTED||q->m_AwaitingStatus!=Quest::AS_WAIT)
			{
				slot<<q->m_SysName; //�� ���� ����� ���������������� ����� ��� �������� ��. ����
				q->MakeSaveLoad(slot);
			}
		}
	}
	else
	{
		slot>>m_Episode;
		int i;
		slot>>i;
		ChangeEpisode(m_Episode, static_cast<LogicServer1*>(NULL));
		while(i--)
		{
			Quest *q;
			std::string sys_name;
			slot>>sys_name;
			q=Get(sys_name);
			if(!q)
			{ //� ����� ���� �����, ������� ��� ������ � �������
				Quest *new_quest=new Quest;
				new_quest->MakeSaveLoad(slot);
				if(new_quest->m_CurState==QS_NOTSTARTED||
					new_quest->m_CurState==QS_STARTED)
				{
					new_quest->m_CurState=QS_COMPLETE;
					new_quest->m_AwaitingStatus=Quest::AS_STARTED;
					Quests.push_back(new_quest);
				}
				else delete new_quest;
			}
			else
				q->MakeSaveLoad(slot);
		}
	}
	MessageBase::MakeSaveLoad(slot);
}

/**************************************************************************/
MessageBase::KilledList  MessageBase::m_Killed;
MessageBase::DeadList    MessageBase::m_Dead;
MessageBase::UsedList    MessageBase::m_Used;
MessageBase::HeroList    MessageBase::m_Heroes;
std::string              MessageBase::StartBonusQuest("");

//=====================================================================================//
//                           void MessageBase::AddMessage()                            //
//=====================================================================================//
void MessageBase::AddMessage(QuestMessage1 &mes)
{
	std::string subj=mes.Subject->GetSysName();
	std::string act=mes.Actor->GetSysName();

	switch(mes.Type)
	{
	case QuestMessage1::MT_USE:
	case QuestMessage1::MT_FIRSTUSE:
		//fixme: ������ ���� ������� ��� �������!
		m_Used.insert(subj);
		break;
	case QuestMessage1::MT_KILL:
		++m_Dead.insert(std::make_pair(subj,1)).first->second;

		if(Q2LUtilities::IsHeroInTeam(mes.Actor))
		{
			++m_Killed.insert(std::make_pair(subj,1)).first->second;
		}
		break;

	}
}

//=====================================================================================//
//                            bool MessageBase::IsKilled()                             //
//=====================================================================================//
bool MessageBase::IsKilled(const std::string sys_name)
{
	return m_Killed.find(sys_name)!=m_Killed.end();
}

//=====================================================================================//
//                             bool MessageBase::IsDead()                              //
//=====================================================================================//
bool MessageBase::IsDead(const std::string sys_name)
{
	return m_Dead.find(sys_name)!=m_Dead.end();
}

//=====================================================================================//
//                             bool MessageBase::IsUsed()                              //
//=====================================================================================//
bool MessageBase::IsUsed(const std::string sys_name)
{
	return m_Used.find(sys_name)!=m_Used.end();
}

//=====================================================================================//
//                      QUEST_STATE MessageBase::GetQuestState()                       //
//=====================================================================================//
QUEST_STATE MessageBase::GetQuestState(const std::string sys_name)
{
	Quest *q=QuestEngine::Get(sys_name);
	if(q)
	{
		return q->GetState();
	}
	else
	{
		return QS_NOTSTARTED;
	}
}

//=====================================================================================//
//                              void MessageBase::Clear()                              //
//=====================================================================================//
void MessageBase::Clear()
{
	m_Killed.clear();
	m_Dead.clear();
	m_Used.clear();
	m_Heroes.clear();
}

//=====================================================================================//
//                         void MessageBase::StartBonusMode()                          //
//=====================================================================================//
//fixme: �������� ��� ������������ ������ ������ �� ���� �����
void MessageBase::StartBonusMode(const std::string &SBQ)
{
	StartBonusQuest=SBQ;
}

//=====================================================================================//
//                          void MessageBase::HeroWantsJoin()                          //
//=====================================================================================//
void MessageBase::HeroWantsJoin(const std::string &Subj)
{
	m_Heroes[Subj]=StartBonusQuest;
}

//=====================================================================================//
//                         bool MessageBase::isHeroWantsJoin()                         //
//=====================================================================================//
bool MessageBase::isHeroWantsJoin(const std::string &Subj)
{
	HeroList::iterator it=m_Heroes.find(Subj);
	if(it==m_Heroes.end()) return false;
	if(!it->second.empty())
	{
		return GetQuestState(it->second)==QS_STARTED;
	}
	else
	{
		return true;
	}
}

//=====================================================================================//
//                           void MessageBase::HeroWalkout()                           //
//=====================================================================================//
void MessageBase::HeroWalkout(const std::string &Subj)
{
	m_Heroes.erase(Subj);
}

//=====================================================================================//
//                          void MessageBase::MakeSaveLoad()                           //
//=====================================================================================//
void MessageBase::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{
		slot<<m_Killed.size();
		KilledList::iterator it=m_Killed.begin(),ite=m_Killed.end();
		for(;it!=ite;it++)
		{
			slot<<it->first<<it->second;

		}
		slot<<m_Used.size();
		UsedList::iterator uit=m_Used.begin(),uite=m_Used.end();
		for(;uit!=uite;uit++)
		{
			slot<<*uit;
		}
		slot<<m_Heroes.size();
		HeroList::iterator hit=m_Heroes.begin(),hite=m_Heroes.end();
		for(;hit!=hite;hit++)
		{
			slot<<hit->first<<hit->second;
		}
		slot<<m_Dead.size();
		DeadList::iterator dit=m_Dead.begin(),dite=m_Dead.end();
		for(;dit!=dite;dit++)
		{
			slot<<dit->first<<dit->second;

		}
	}
	else
	{
		Clear();
		int i;
		slot>>i;
		while(i--)
		{
			int num;
			std::string data;
			slot>>data>>num;
			m_Killed[data]=num;
		}
		slot>>i;
		while(i--)
		{
			std::string data;
			slot>>data;
			m_Used.insert(data);
		}
		slot>>i;
		while(i--)
		{
			std::string data,data1;
			slot>>data>>data1;
			m_Heroes[data]=data1;
		}
		try
		{
			slot>>i;
			while(i--)
			{
				int num;
				std::string data;
				slot>>data>>num;
				m_Dead[data]=num;
			}
		}
		catch(CasusImprovisus &)
		{
			m_Dead=m_Killed;
		}
	}

}

/**************************************************************************/
//=====================================================================================//
//                                   Quest::~Quest()                                   //
//=====================================================================================//
Quest::~Quest()
{
	{
		DepList::iterator it=m_Dependencies.begin(),ite=m_Dependencies.end();
		for(;it!=ite;it++)
			delete *it;
	}
	{
		TaskList::iterator it=m_Tasks.begin(),ite=m_Tasks.end();
		for(;it!=ite;it++)
			delete *it;
	}
	for(int i=0;i<QS_MAXSTATES;i++)
	{
		BonusList::iterator it=m_Bonuses[i].begin(),ite=m_Bonuses[i].end();
		for(;it!=ite;it++)
			delete *it;
	}

}

//=====================================================================================//
//                           void Quest::CheckDependencies()                           //
//=====================================================================================//
//��������� ������� ������ (� ���������� ���� �����)
void Quest::CheckDependencies(QuestMessage1 *mes)
{
	if(m_AwaitingStatus==AS_STARTED) return;
	m_AwaitingStatus=AS_MAYSTART;
	DepList::iterator it=m_Dependencies.begin(),ite=m_Dependencies.end();
	for(;it!=ite;it++)
	{
		if(!(*it)->Passed(mes->serv)) {m_AwaitingStatus=AS_WAIT;}
		if((*it)->CantPass(mes->serv)) {m_AwaitingStatus=AS_NEVER;break;}
	}
}

//=====================================================================================//
//                              bool Quest::CheckState()                               //
//=====================================================================================//
//��������� ���������� ����� (������� ��������� ���� �����)
bool Quest::CheckState(QuestMessage1 *mes)
{
	//��������� �������� ���������
	if(m_CurState==QS_PASSED||m_CurState==QS_PASSED_OK||m_CurState==QS_FAILED||m_CurState==QS_COMPLETE)
		return false;
	bool ret=false;

	if(m_HasHolder && mes->serv->IsHeroDead(m_Holder)/* && !m_AutoFinish*/)
	{	//�������� � �������� ��������.
		if(m_CurState==QS_NOTSTARTED)// ���� �� ��� �� �������
		{
			ret=GoToState(QS_PASSED, mes)||ret;
		}
		else if(m_CurState==QS_STARTED) //���� ��� �������
		{
			ret=GoToState(QS_FAILED, mes)||ret;
		}
		return ret;
	}

	//��������� ���������, �� ��������������� ���
	bool HolderNotFoe=Q2LUtilities::isNotFoe(m_Holder,mes->serv);
	bool UseMes=(mes->Type==QuestMessage1::MT_USE||mes->Type==QuestMessage1::MT_FIRSTUSE);

	if(!(!m_HasHolder||  //���� ����� ��� ��������� ������
		(UseMes&&(mes->Subject->GetSysName()==m_Holder)&&HolderNotFoe )|| //��� �� �� ��������� ������
		(m_AutoStart && m_CurState==QS_NOTSTARTED)||//��� ����� ��������������
		(m_AutoFinish && m_CurState!=QS_NOTSTARTED) //��� ���� ����� ������ ������������� ����������
		))
		return false;


	if(m_AwaitingStatus!=AS_MAYSTART&&m_AwaitingStatus!=AS_STARTED) return false;
	m_AwaitingStatus=AS_STARTED;


	enum TASK_STATE{TS_COMPLETE, TS_NEVER, TS_WAIT};

	TASK_STATE task_state=TS_COMPLETE;

	TaskList::iterator it=m_Tasks.begin(),ite=m_Tasks.end();
	for(;it!=ite;it++)
	{
		if(!(*it)->Complete(mes->serv)) {task_state=TS_WAIT;}
		if((*it)->CantComplete(mes->serv)) {task_state=TS_NEVER;break;}
	}

	bool swh=(!m_HasHolder)||(mes->Subject->GetSysName()==this->m_Holder);

	switch(task_state)
	{
	case TS_COMPLETE:   //��� ������ ���������
		if(m_CurState==QS_NOTSTARTED) //� ����� ��� �� �������
		{
			if(m_AutoStart) //�� ������ ��� �������� �������������
			{
				if(swh||(m_AutoFinish&&m_CurState!=QS_COMPLETE)) 
					ret=GoToState(QS_COMPLETE, mes)||ret;
				else{
					ret=GoToState(QS_STARTED, mes)||ret;
				}
			}
			else if(m_AutoFinish&&m_CurState!=QS_COMPLETE/*&&swh*/)
				ret=GoToState(QS_COMPLETE, mes)||ret;
			else if(!m_AutoFinish&&!m_AutoStart&&swh)//���� ������ ������ ���� ��������� �� ��� ������
				ret=GoToState(QS_PASSED_OK, mes)||ret;

		}
		else if(m_CurState==QS_STARTED) //���� ����� ��� ��� ���������
		{
			ret=GoToState(QS_COMPLETE, mes)||ret;
		}
		break;
	case TS_NEVER: //������ ����� ��� ����� ���� ���������
		if(m_CurState==QS_NOTSTARTED)// ���� �� ��� �� �������
		{
			ret=GoToState(QS_PASSED, mes)||ret;
		}
		else if(m_CurState==QS_STARTED) //���� ��� �������
		{
			ret=GoToState(QS_FAILED, mes)||ret;
		}
		break;
	case TS_WAIT: //������ � �������� ����������
		if(m_CurState==QS_NOTSTARTED) //������ ����� ���� �� �����
		{
			ret=GoToState(QS_STARTED, mes)||ret;
		}
		else if(m_CurState==QS_STARTED) //������ �� ������ ���� �����
		{
			ret=GoToState(QS_STARTED, mes)||ret; //������ ���������
		}
		break;
	}
	return ret;
}

//=====================================================================================//
//                              void Quest::GiveBonuses()                              //
//=====================================================================================//
void Quest::GiveBonuses(QUEST_STATE state, QuestMessage1 *mes)
{
	//1.���� �����
	//***************************************************
	if(state==QS_STARTED) MessageBase::StartBonusMode(m_SysName);
	mes->serv->GetReportBuilder()->Start();//��� ������ - � ������

	if(state!=QS_NOTSTARTED&&state!=QS_STARTED&&state!=QS_PASSED&&m_HasHolder && (!m_AutoStart||state!=QS_FAILED))
		mes->serv->GetReportBuilder()->AddEvent(LogicServer1::ReportBuilder::ET_FINISH);

	BonusList::iterator it=m_Bonuses[state].begin(),ite=m_Bonuses[state].end();
	for(;it!=ite;it++)
		(*it)->GiveIt(mes->Actor, mes->serv);

	mes->serv->GetReportBuilder()->Finish();
	//***************************************************


	if(state!=QS_NOTSTARTED&&state!=QS_STARTED)
	{
		TaskList::iterator tit=m_Tasks.begin(),tite=m_Tasks.end();
		for(;tit!=tite;tit++)
			(*tit)->Execute(mes->serv);  //��������� �������� ��������� ��������� �����
	}
	if(state==QS_STARTED) MessageBase::StartBonusMode("");
}

//=====================================================================================//
//                               bool Quest::GoToState()                               //
//=====================================================================================//
bool Quest::GoToState(QUEST_STATE state, QuestMessage1 *mes)
{
	//2.������� ���������
	std::string *text=NULL,*wavname;
	switch(state)
	{
	case QS_STARTED:
		if(m_CurState!=QS_STARTED) 
		{
			if(!m_AutoStart) 
				mes->serv->Notify(LogicServer1::QET_NEW_QUEST_START);
			text=&m_Speeches.StartSpeech[0];wavname=&m_Speeches.StartSpeech[1];
		}
		else                      
		{
			if(m_HasHolder && mes->Subject->GetSysName()==m_Holder && mes->Type!=QuestMessage1::MT_KILL)
				text=&m_Speeches.InProgressSpeech[0];wavname=&m_Speeches.InProgressSpeech[1];
		}
		break;
	case  QS_FAILED:
		text=&m_Speeches.FailedSpeech[0];wavname=&m_Speeches.FailedSpeech[1];
		break;
	case QS_COMPLETE:
		text=&m_Speeches.CompleteSpeech[0];wavname=&m_Speeches.CompleteSpeech[1];
		break;
	case QS_PASSED:
		text=&m_Speeches.PassedSpeech[0];wavname=&m_Speeches.PassedSpeech[1];
		break;
	case QS_PASSED_OK:
		text=&m_Speeches.PassedOKSpeech[0];wavname=&m_Speeches.PassedOKSpeech[1];
		break;
	};
	bool ret=false;



	if(text && !text->empty() && *text!="-" )
	{
		if(mes->Type!=QuestMessage1::MT_KILL)	//��� ����, ����� ��� ������� ������ ������ �� ������� ��� ������ ������
		{
			ret=true;
			mes->Subject->SayPhrase(phrase_s(*text,*wavname));
		}
	}


	//��� �� ������, ���� ����� �� ����� �������� ��������
	bool Permit=true;
	bool alldone=true;
	if(state==QS_COMPLETE || state==QS_PASSED_OK)
	{
		TaskList::iterator tit=m_Tasks.begin(),tite=m_Tasks.end();
		for(;tit!=tite;tit++)
		{
			if(!(*tit)->Complete(mes->serv)) {alldone=false;break;}
			if((*tit)->CantComplete(mes->serv)) {alldone=false;break;}
			if(state!=QS_STARTED && !(*tit)->PlayerPermit(mes->serv)) {Permit=false;break;}
		}
	}

	if(!alldone || Permit) //���� ��� �� ��� ������ ��������� ��� ���� ��� ��������� � �������� �����
	{
		if(state!=QS_STARTED&&state!=QS_NOTSTARTED)	
		{	//��� ���������� ��� ��������� �������� ����� ����� � ����������� ������
			BonusList::iterator it=m_Bonuses[QS_STARTED].begin(),ite=m_Bonuses[QS_STARTED].end();
			for(;it!=ite;it++)
				(*it)->QuestGone(mes->serv);
		}

		if(state!=m_CurState)
		{//��� �������� ����������� ������ ���� ��� ��� ����� � �����. ���������
			if(!m_HasHolder || !mes->serv->IsHeroDead(m_Holder) )
				GiveBonuses(state,mes);
			//3.�������� ������
			if(m_Speeches.Name.size()&&m_Speeches.Name!="-")
			{
				if(state==QS_STARTED)    
					QuestEngineJournal::AddRecord(mes->serv, m_Speeches.Name, m_Speeches.JournalRecord);
				else if(state!=QS_NOTSTARTED)
					QuestEngineJournal::RemoveTheme(mes->serv, m_Speeches.Name);
				//4.������� ������� ���������
			}

			if(state==QS_COMPLETE||/*state==QS_PASSED||*/state==QS_PASSED_OK)
			{//���������� � ���������� ������
				LogicServer1::quest_info qi(m_SysName);
				mes->serv->Notify(LogicServer1::QET_QUEST_FINISHED,&qi);
			}
		}
		m_CurState=state;
	}		
	return ret;
}

//=====================================================================================//
//                             void Quest::MakeSaveLoad()                              //
//=====================================================================================//
void Quest::MakeSaveLoad(SavSlot &slot)
{
	//��� ������ ����������� � ����������� �������
	//��� ���� �������������� �������� �� ��������
	//�������, ������� ��� ��� � �������
	if(slot.IsSaving())
	{
		slot<<m_CurState;
		slot<<m_AwaitingStatus;
	}
	else
	{
		int state,status;
		slot>>state;
		slot>>status;
		m_CurState=static_cast<QUEST_STATE>(state);
		m_AwaitingStatus=static_cast<AWAITING_STATUS>(status);
		//if(m_CurState==QS_STARTED&&m_Speeches.Name.size())         QuestEngineJournal::AddRecord(m_SysName, m_Speeches.Name, m_Speeches.JournalRecord,false);
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////
//=====================================================================================//
//                        void QuestEngineJournal::AddRecord()                         //
//=====================================================================================//
void QuestEngineJournal::AddRecord(LogicServer1 *serv, const std::string &questname, const std::string &description)
{
	serv->AddJournalRecord(questname, description);
}

//=====================================================================================//
//                       void QuestEngineJournal::RemoveTheme()                        //
//=====================================================================================//
void QuestEngineJournal::RemoveTheme(LogicServer1 *serv, const std::string &questname)
{
	serv->RemoveJournalTheme(questname);
}

