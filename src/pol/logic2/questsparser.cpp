#pragma warning(disable:4786)
#include "logicdefs.h"
#include "questengine.h"
#include "questengine_loc.h"
#include "dirtylinks.h"
#include "../common/datamgr/txtfile.h" 
#include <algorithm>
#include "../options/options.h"
#include "../options/xregistry.h"
#include "../common/piracycontrol/piracycontrol.h"
#include "entity.h"

enum QUEST_COLUMNS
{  QC_SYSNAME, QC_NAME, QC_DEPENDENCIES, QC_TASKS,
QC_HOLDER, QC_AUTOSTART,
//���������
QC_JOURNAL, QC_DESCRIPTION, QC_STARTMES, QC_COMPLETEMES, QC_INPROGRESSMES,
QC_FAILEDMES, QC_PASSEDMES, QC_PASSEDOKMES,
//�����
QC_COMPLETEBONUS, QC_STARTBONUS, QC_PASSEDOKBONUS, QC_FAILEDBONUS, QC_PASSEDBONUS,


QC_PHASE,
QC_AUTOFINISH,
QC_MAX_COLUMNS
};
struct QuestColumns_s{
	QUEST_COLUMNS Num;
	char   *Name;
	int           PosInTable;
};

//=====================================================================================//
//                            static QuestColumns_s Col[] =                            //
//=====================================================================================//
static QuestColumns_s Col[] =
{
	{QC_SYSNAME,"System Name",-1},
	{QC_NAME,"Quest Name",-1},
	{QC_DEPENDENCIES,"Dependencies",-1},
	{QC_TASKS,"Quest Tasks",-1},
	{QC_HOLDER,"Quest Holder",-1},
	{QC_AUTOSTART,"Autostart",-1},
	{QC_JOURNAL,"Journal message",-1},
	{QC_DESCRIPTION,"Description",-1},
	{QC_STARTMES,"Start message",-1},
	{QC_COMPLETEMES,"Complete message",-1},
	{QC_INPROGRESSMES,"Inprogress message",-1},
	{QC_FAILEDMES,"Failed message",-1},
	{QC_PASSEDMES,"Passed message",-1},
	{QC_PASSEDOKMES,"Passed OK message",-1},
	{QC_COMPLETEBONUS,"Complete Bonus",-1},
	{QC_STARTBONUS,"Start Bonus",-1},
	{QC_PASSEDOKBONUS,"Passed OK Bonus",-1},
	{QC_FAILEDBONUS,"Failed Bonus",-1},
	{QC_PASSEDBONUS,"Passed Bonus",-1},
	{QC_PHASE,"Phase",-1},
	{QC_AUTOFINISH,"AutoFinish",-1},
	{QC_MAX_COLUMNS,"",-1},
};

Quest::TaskList  ParseTasks(std::string &data);
Quest::DepList   ParseDependencies(std::string &data,std::string &phase);
Quest::BonusList ParseBonuses(std::string &data);

//=====================================================================================//
//                          void QuestEngine::ChangeEpisode()                          //
//=====================================================================================//
void QuestEngine::ChangeEpisode(int num, LogicServer1 *serv)
{
	if(num)
	{
		if(
			!PiracyControl::checkFilmsSize(Options::Registry()->Var("Video Path").GetString()+std::string("\\"))
			)
		{
			throw CASUS("��������� ����������� ���������. �������������� ����.");
		}

	}

	m_Episode=num;
	//������� ��������� ��������� ��������.
	Options::Set("game.advance",256-num-1);//������� ������������ ������������ ��� (256-value)
	//256 - ����� ��� ����� (������ 3 ��������)
	//255 +������� 1 �������
	//254 +������� 2 �������
	//253 +������� 3 �������
	//252 +������� 4 �������
	//251 + ��������� �������

	//1.��� �������
	QuestLog("Changing Episode\n");
	if(serv)
	{
		QuestList::iterator it1=Quests.begin(),ite1=Quests.end();
		for(;it1!=ite1;it1++)
		{
			if((*it1)->GetState()==QS_STARTED)
			{
				Quest::BonusList::iterator it=(*it1)->m_Bonuses[QS_STARTED].begin(),
					ite=(*it1)->m_Bonuses[QS_STARTED].end();
				for(;it!=ite;it++)
					(*it)->QuestGone(serv);
			}
			if((*it1)->m_Speeches.Name.size()&&(*it1)->m_Speeches.Name!="-")
				QuestEngineJournal::RemoveTheme(serv, (*it1)->m_Speeches.Name);

		}
	}
	Clear();
	//2.�������� �������
	char table_name[255];
	sprintf(table_name,"scripts/quests/quests_%d.txt",m_Episode+1);

	TxtFile table(DataMgr::Load(table_name));
	DataMgr::Release(table_name);
	if(!table.SizeY()) return;

	//3.��������� ��������� ��������
	int header=0;
	QuestLog("Looking through table...\n");
	for(int i=0;Col[i].Num!=QC_MAX_COLUMNS;i++)
	{
		unsigned num; 
		if(table.FindInRow(Col[i].Name,header,&num))
		{
			Col[i].PosInTable=num;
		}
		else
			throw CASUS(std::string("���������� ����� �������\n � ������� �������:\n")+Col[i].Name);

	}
	//4.�������� ������           
	QuestLog("Loading Quests\n");
	std::string data,phases;
	for(int n=1;n<table.SizeY();n++)
	{
		Quest *q=new Quest;
		table.GetCell(n,Col[QC_SYSNAME].PosInTable,q->m_SysName);
		table.GetCell(n,Col[QC_HOLDER].PosInTable,q->m_Holder);
		q->m_HasHolder=q->m_Holder.size()?true:false;

		table.GetCell(n,Col[QC_NAME].PosInTable,q->m_Speeches.Name);
		table.GetCell(n,Col[QC_DESCRIPTION].PosInTable,q->m_Speeches.Description);
		table.GetCell(n,Col[QC_STARTMES].PosInTable,q->m_Speeches.StartSpeech[0]);
		table.GetCell(n,Col[QC_COMPLETEMES].PosInTable,q->m_Speeches.CompleteSpeech[0]);
		table.GetCell(n,Col[QC_FAILEDMES].PosInTable,q->m_Speeches.FailedSpeech[0]);
		table.GetCell(n,Col[QC_STARTMES].PosInTable,q->m_Speeches.StartSpeech[0]);
		table.GetCell(n,Col[QC_PASSEDMES].PosInTable,q->m_Speeches.PassedSpeech[0]);
		table.GetCell(n,Col[QC_PASSEDOKMES].PosInTable,q->m_Speeches.PassedOKSpeech[0]);
		table.GetCell(n,Col[QC_INPROGRESSMES].PosInTable,q->m_Speeches.InProgressSpeech[0]);
		table.GetCell(n,Col[QC_JOURNAL].PosInTable,q->m_Speeches.JournalRecord);

		q->m_Speeches.CutSoundNames();

		table.GetCell(n,Col[QC_COMPLETEBONUS].PosInTable,data);q->m_Bonuses[QS_COMPLETE]=ParseBonuses(data);
		table.GetCell(n,Col[QC_STARTBONUS].PosInTable,data);q->m_Bonuses[QS_STARTED]=ParseBonuses(data);
		table.GetCell(n,Col[QC_PASSEDOKBONUS].PosInTable,data);q->m_Bonuses[QS_PASSED_OK]=ParseBonuses(data);
		table.GetCell(n,Col[QC_FAILEDBONUS].PosInTable,data);q->m_Bonuses[QS_FAILED]=ParseBonuses(data);
		table.GetCell(n,Col[QC_PASSEDBONUS].PosInTable,data);q->m_Bonuses[QS_PASSED]=ParseBonuses(data);
		table.GetCell(n,Col[QC_AUTOSTART].PosInTable,data);q->m_AutoStart=(data=="yes");
		table.GetCell(n,Col[QC_AUTOFINISH].PosInTable,data);q->m_AutoFinish=(data=="yes");

		table.GetCell(n,Col[QC_DEPENDENCIES].PosInTable,data);
		table.GetCell(n,Col[QC_PHASE].PosInTable,phases);
		q->m_Dependencies=ParseDependencies(data,phases);
		table.GetCell(n,Col[QC_TASKS].PosInTable,data);q->m_Tasks=ParseTasks(data);

		QuestLog("Loaded Quest:%s (%s)\n",q->m_SysName.c_str(),q->m_Speeches.Name.c_str());
		QuestLog("\tDepCount:%d\n",q->m_Dependencies.size());
		QuestLog("\tTaskCount:%d\n",q->m_Tasks.size());
		Quests.push_back(q);
	}
}

/*******************************************************************************************/
//=====================================================================================//
//                        class KillSmbTask : public QuestTask                         //
//=====================================================================================//
class KillSmbTask : public QuestTask
{
public:
	KillSmbTask(const std::string &sysname, bool kbp) : m_SysName(sysname), KillByPlayer(kbp) {}

	virtual bool Complete(LogicServer1 *serv)const
	{
		if(KillByPlayer) return MessageBase::IsKilled(m_SysName);
		else             return MessageBase::IsDead(m_SysName);

	}

	virtual bool CantComplete(LogicServer1 *serv) const
	{
		if(KillByPlayer && serv->IsHeroDead(m_SysName) && !MessageBase::IsKilled(m_SysName))
		{
			return true;
		}
		return false;
	}

	bool KillByPlayer;
	std::string m_SysName;
};

//=====================================================================================//
//                        class KillTeamTask : public QuestTask                        //
//=====================================================================================//
class KillTeamTask : public QuestTask
{
	std::string m_team, m_map;

public:
	KillTeamTask(const std::string &params)
	{
		m_team = params.substr(0,params.find(","));
		m_map = params.substr(m_team.length()+1);
	}

	KillTeamTask(const std::string &team, const std::string &map)
	:	m_team(team), m_map(map)
	{
	}

	virtual bool Complete(LogicServer1 *serv)const
	{
		if(DirtyLinks::GetLevelSysName() != m_map) return false;
		EntityPool *epool = EntityPool::GetInst();

		for(EntityPool::iterator i = epool->begin(); i != epool->end(); ++i)
		{
			if(HumanEntity *human = i->Cast2Human())
			{
				if(human->GetInfo()->GetForce()->GetRID() == m_team) return false;
			}
		}

		return true;
	}

	virtual bool CantComplete(LogicServer1 *serv) const
	{
		return false;
	}
};

//=====================================================================================//
//                         class UseSmbTask : public QuestTask                         //
//=====================================================================================//
class UseSmbTask : public QuestTask
{
public:
	UseSmbTask(const std::string &sysname) : m_SysName(sysname) {}

	virtual bool Complete(LogicServer1 *serv)const
	{
		return MessageBase::IsUsed(m_SysName);
	}

	virtual bool CantComplete(LogicServer1 *serv) const
	{

		return serv->IsHeroDead(m_SysName)&&!Complete(serv);
	}
	std::string m_SysName;
};

//=====================================================================================//
//                       class BringItemsTask : public QuestTask                       //
//=====================================================================================//
class BringItemsTask : public QuestTask
{
public:
	BringItemsTask(const std::string &itset) : m_ItemSet(itset) {}

	virtual bool Complete(LogicServer1 *serv)const
	{
		return serv->CanGetItems(m_ItemSet);
	}

	virtual bool PlayerPermit(LogicServer1 *serv)
	{
		if(serv->ShowMessageBox(DirtyLinks::GetStrRes("qqs_want_give_item")))
			return true;
		else
			return false;
	}

	virtual void Execute(LogicServer1 *serv) 
	{
		serv->TakeItems(m_ItemSet);
	};

	std::string m_ItemSet;
};

//////////////////////////////////////////////////////////////////////
//=====================================================================================//
//                 class WaitQuestDependency : public QuestDependency                  //
//=====================================================================================//
class WaitQuestDependency : public QuestDependency
{
public:
	enum WAIT_TYPE{WT_NOTSTARTED, WT_INPROGRESS, WT_PASSED, WT_FAILED, WT_COMPLETE, WT_GONE};
	virtual bool Passed(LogicServer1 *serv)const   //���� ������� ������� ���������
	{
		QUEST_STATE qs=MessageBase::GetQuestState(m_SysName);
		if(m_Type==WT_PASSED&&qs==QS_PASSED) return true;
		if((m_Type==WT_INPROGRESS)&&(qs==QS_STARTED)) return true;
		if(m_Type==WT_COMPLETE&& ((qs==QS_PASSED_OK)||(qs==QS_COMPLETE)) ) return true;
		if(m_Type==WT_FAILED&& ((qs==QS_PASSED)||(qs==QS_FAILED)) ) return true;
		if(m_Type==WT_GONE&&(qs!=QS_NOTSTARTED)&&(qs!=QS_STARTED)) return true;
		if(m_Type==WT_NOTSTARTED && qs==QS_NOTSTARTED) return true;
		return false;
	}
	virtual bool CantPass(LogicServer1 *serv)const {return false;}; //���� ������� ������� ���������� ��������� (��: ����� quest=notstarted, � �� started)
	std::string m_SysName;
	WAIT_TYPE m_Type;
};

//=====================================================================================//
//                 class SSceneWaitDependency : public QuestDependency                 //
//=====================================================================================//
class SSceneWaitDependency : public QuestDependency
{
public:
	virtual bool Passed(LogicServer1 *serv)const   //���� ������� ������� ���������
	{
		return serv->IsSceneFinished(m_SceneName);
	}
	std::string m_SceneName;
};

//=====================================================================================//
//                    class HeroDependency : public QuestDependency                    //
//=====================================================================================//
class HeroDependency : public QuestDependency
{
public:
	virtual bool Passed(LogicServer1 *serv)const   //���� ������� ������� ���������
	{
		bool Included=Q2LUtilities::IsHeroInTeam(m_SysName,serv);
		return (Included&&m_Included)||(!Included&&!m_Included);
	}
	std::string m_SysName;
	bool        m_Included;
};

//=====================================================================================//
//                   class PhaseDependency : public QuestDependency                    //
//=====================================================================================//
class PhaseDependency : public QuestDependency
{
public:
	virtual bool Passed(LogicServer1 *serv)const   //���� ������� ������� ���������
	{
		int CurPhase=serv->GetCurrentPhase();
		return m_Phases.find(CurPhase)!=m_Phases.end();
	}
	std::set<int> m_Phases;
};

//////////////////////////////////////////////////////////////////////
//=====================================================================================//
//                        class ItemsBonus : public QuestBonus                         //
//=====================================================================================//
class ItemsBonus : public QuestBonus
{
public:
	virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const
	{
		actor->GiveItems(m_ItemSet);
		serv->GetReportBuilder()->AddItems(m_ItemSet);

	}
	std::string m_ItemSet;
};

//=====================================================================================//
//                      class ExperienceBonus : public QuestBonus                      //
//=====================================================================================//
class ExperienceBonus : public QuestBonus
{
public:
	virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const
	{
		actor->GiveExperience(m_Exp);
		serv->GetReportBuilder()->AddExperience(m_Exp);
	}
	int m_Exp;
};

//=====================================================================================//
//                         class HeroBonus : public QuestBonus                         //
//=====================================================================================//
class HeroBonus : public QuestBonus
{
public:
	virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const
	{
		if(m_Give)
		{
			//gqs_add_hero
			if(!Q2LUtilities::IsHeroInTeam(m_SysName,serv))
			{
				std::string hname=Q2LUtilities::GetHeroName(m_SysName,serv);
				std::string mes=mlprintf(DirtyLinks::GetStrRes("gqs_add_hero").c_str(),hname);

				extern std::set<std::string> HerosAskedForJoin;
				if((!serv->IsHeroDead(m_SysName) )&& HerosAskedForJoin.end()==HerosAskedForJoin.find(m_SysName))
				{
					if(serv->ShowMessageBox(mes))
					{
						Q2LUtilities::AddHeroInTeam(m_SysName,serv);
					}
					HerosAskedForJoin.insert(m_SysName);

				}
			}
			MessageBase::HeroWantsJoin(m_SysName);
		}
		else
		{
			Q2LUtilities::RemoveHeroFromTeam(m_SysName,serv);
			MessageBase::HeroWalkout(m_SysName);
		}
	}
	virtual void QuestGone(LogicServer1 *serv) const 
	{
		if(m_Give)
		{
			Q2LUtilities::RemoveHeroFromTeam(m_SysName,serv);
			MessageBase::HeroWalkout(m_SysName);
		}
	}
	bool m_Give;
	std::string m_SysName;
};

//=====================================================================================//
//                     class EnableLevelBonus : public QuestBonus                      //
//=====================================================================================//
class EnableLevelBonus : public QuestBonus
{
public:
	virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const
	{
		serv->EnableLevel(m_LevName,m_Enable);
	}
	std::string m_LevName;
	bool m_Enable;
};

//=====================================================================================//
//                   class ChangeAlignmentBonus : public QuestBonus                    //
//=====================================================================================//
class ChangeAlignmentBonus : public QuestBonus
{
public:
	enum STATE {S_WAR=RT_ENEMY, S_NEUTRAL=RT_NEUTRAL, S_FRIEND=RT_FRIEND};
	virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const
	{
		serv->ChangeAlignment(m_SideA,m_SideB,static_cast<RelationType>(m_State));
		serv->ChangeAlignment(m_SideB,m_SideA,static_cast<RelationType>(m_State));
	}
	STATE m_State;
	std::string m_SideA;
	std::string m_SideB;
};

//=====================================================================================//
//                     class ChangePhaseBonus : public QuestBonus                      //
//=====================================================================================//
class ChangePhaseBonus : public QuestBonus
{
public:
	virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const
	{
		serv->ChangePhase(m_Phase);
	}
	int m_Phase;
};

//=====================================================================================//
//                       class AddNewsBonus : public QuestBonus                        //
//=====================================================================================//
class AddNewsBonus : public QuestBonus
{
public:
	virtual void GiveIt(LogicServer1::EntityManipulator *actor, LogicServer1 *serv)const
	{
		serv->AddNews(m_sysname);
	}
	std::string m_sysname;
};

//=====================================================================================//
//                           Quest::BonusList ParseBonuses()                           //
//=====================================================================================//
Quest::BonusList ParseBonuses(std::string &data)
{
	data = KillSpaces(data);
	Quest::BonusList Bonuses;
	std::string::size_type start,finish,finish1;
	for(start=0;;start=finish+1)
	{
		start=data.find("add_hero(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		HeroBonus *bon=new HeroBonus;
		bon->m_SysName=subject;
		bon->m_Give=true;
		Bonuses.push_back(bon);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("remove_hero(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		HeroBonus *bon=new HeroBonus;
		bon->m_SysName=subject;
		bon->m_Give=false;
		Bonuses.push_back(bon);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("give_items(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		ItemsBonus *bon=new ItemsBonus;
		bon->m_ItemSet=subject;
		Bonuses.push_back(bon);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("add_news(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		AddNewsBonus *bon=new AddNewsBonus;
		bon->m_sysname=subject;
		Bonuses.push_back(bon);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("experience(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		ExperienceBonus *bon=new ExperienceBonus;
		bon->m_Exp=atoi(subject.c_str());
		Bonuses.push_back(bon);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("changephase(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		ChangePhaseBonus *bon=new ChangePhaseBonus;
		bon->m_Phase=atoi(subject.c_str());
		Bonuses.push_back(bon);
	}
	for(start=0;;start=finish1+1)
	{

		start=data.find("alignment(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish1=data.find_first_of(")",start);    if(finish1==data.npos) break;
		std::string subject=std::string(data,start,finish1-start);

		finish=subject.find_first_of(";,.");

		std::string sideA=std::string(subject,0,finish);
		start=finish+1;
		finish=subject.find_first_of(";,.",start);
		std::string sideB=std::string(subject,start,finish-start);
		start=finish+1;
		finish=subject.find_first_of(")",start);
		if(finish==subject.npos) finish=subject.size();
		std::string type=std::string(subject,start,finish-start);
		std::transform(type.begin(),type.end(), type.begin(),tolower);
		//KillSpaces(subject);
		ChangeAlignmentBonus *bon=new ChangeAlignmentBonus;
		bon->m_SideA=sideA;
		bon->m_SideB=sideB;
		if(type=="enemy")  bon->m_State=ChangeAlignmentBonus::S_WAR;
		else if(type=="neutral")  bon->m_State=ChangeAlignmentBonus::S_NEUTRAL;
		else if(type=="friend")  bon->m_State=ChangeAlignmentBonus::S_FRIEND;
		Bonuses.push_back(bon);//fixme:
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("enablelevel(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		EnableLevelBonus *bon=new EnableLevelBonus;
		bon->m_LevName = subject;
		bon->m_Enable=true;
		Bonuses.push_back(bon);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("disablelevel(",start);    if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);    if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		EnableLevelBonus *bon=new EnableLevelBonus;
		bon->m_LevName = subject;
		bon->m_Enable=false;
		Bonuses.push_back(bon);
	}

	return Bonuses;
}

//=====================================================================================//
//                         Quest::DepList ParseDependencies()                          //
//=====================================================================================//
Quest::DepList ParseDependencies(std::string &data,std::string &phase)
{
	data = KillSpaces(data);
	phase = KillSpaces(phase);
	Quest::DepList Deps;
	std::string::size_type start,finish,eos;
	for(start=0;;start=finish+1)
	{
		start=data.find("hero_in_team(",start);
		if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);
		if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		HeroDependency *dep=new HeroDependency;
		dep->m_SysName=subject;
		dep->m_Included=true;
		Deps.push_back(dep);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("hero_not_in_team(",start);
		if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);
		if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		HeroDependency *dep=new HeroDependency;
		dep->m_SysName=subject;
		dep->m_Included=false;
		Deps.push_back(dep);
	}
	for(start=0;;start=finish+1)
	{
		start=data.find("script_scene_finished(",start);
		if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);
		if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		//KillSpaces(subject);
		SSceneWaitDependency *dep=new SSceneWaitDependency;
		dep->m_SceneName=subject;
		Deps.push_back(dep);
	}
	for(start=0;;start=finish+1)
	{
		std::string::size_type comma;
		start=data.find("quest(",start);
		if(start==data.npos) break;
		start=data.find("(",start);start++;
		finish=data.find_first_of(")",start);
		if(finish==data.npos) break;
		std::string subject=std::string(data,start,finish-start);
		comma=subject.find_first_of(",");
		std::string qname=std::string(subject,0,comma);
		//enum WAIT_TYPE{WT_NOTSTARTED, WT_INPROGRESS, WT_PASSED, WT_FAILED};
		WaitQuestDependency *dep=new WaitQuestDependency;
		dep->m_SysName=qname;
		dep->m_Type=WaitQuestDependency::WT_PASSED;
		std::transform(subject.begin(),subject.end(),subject.begin(),tolower);
		if(subject.find("complete")!=subject.npos) dep->m_Type=WaitQuestDependency::WT_COMPLETE;
		else if(subject.find("notstarted")!=subject.npos) dep->m_Type=WaitQuestDependency::WT_NOTSTARTED;
		else if(subject.find("started")!=subject.npos) dep->m_Type=WaitQuestDependency::WT_INPROGRESS;
		else if(subject.find("passed")!=subject.npos) dep->m_Type=WaitQuestDependency::WT_PASSED;
		else if(subject.find("gone")!=subject.npos) dep->m_Type=WaitQuestDependency::WT_GONE;
		else if(subject.find("failed")!=subject.npos) dep->m_Type=WaitQuestDependency::WT_FAILED;
		Deps.push_back(dep);
	}

	std::set<int> phases;
	for(start=0;;start=finish+1)
	{
		finish=phase.find_first_of(".;,",start);
		std::string subject;
		eos = phase.npos;

		if(finish==eos)   subject=std::string(phase,start,phase.size()-start);
		else                     subject=std::string(phase,start,finish-start);
		phases.insert(atoi(subject.c_str()));
		if (finish==eos) break;
	}
	PhaseDependency *dep=new PhaseDependency;
	dep->m_Phases=phases;
	Deps.push_back(dep);

	return Deps;
}

//=====================================================================================//
//                               static void readTask()                                //
//=====================================================================================//
template<typename T, typename It>
static void readTask(It it, const std::string &data, const std::string &pattern)
{
	const std::string ptrn = pattern + "(";
	for(size_t start = 0, finish; ; start = finish + 1)
	{
		if((start = data.find(ptrn,start)) == std::string::npos) break;
		start += ptrn.length();
		if((finish = data.find(")",start)) == std::string::npos) break;

		std::string subject = data.substr(start,finish-start);

		*it++ = new T(subject);
	}
}

//=====================================================================================//
//                               static void readTask()                                //
//=====================================================================================//
template<typename T, typename It, typename Param>
static void readTask(It it, const std::string &data, const std::string &pattern, Param p)
{
	const std::string ptrn = pattern + "(";
	for(size_t start = 0, finish; ; start = finish + 1)
	{
		if((start = data.find(ptrn,start)) == std::string::npos) break;
		start += ptrn.length();
		if((finish = data.find(")",start)) == std::string::npos) break;

		std::string subject = data.substr(start,finish-start);

		*it++ = new T(subject,p);
	}
}

//=====================================================================================//
//                            Quest::TaskList ParseTasks()                             //
//=====================================================================================//
Quest::TaskList ParseTasks(std::string &data)
{
	data = KillSpaces(data);
	Quest::TaskList Tasks;

	readTask<KillSmbTask>(std::back_inserter(Tasks),data,"kill",true);
	readTask<KillSmbTask>(std::back_inserter(Tasks),data,"dead",false);
	readTask<UseSmbTask>(std::back_inserter(Tasks),data,"use");
	readTask<UseSmbTask>(std::back_inserter(Tasks),data,"talk");
	readTask<BringItemsTask>(std::back_inserter(Tasks),data,"bring");
	readTask<KillTeamTask>(std::back_inserter(Tasks),data,"eliminate");

	return Tasks;
}

//=====================================================================================//
//                           void QuestInfo::CutSoundNames()                           //
//=====================================================================================//
void QuestInfo::CutSoundNames()
{
	static const std::string PREFIX="$(";
	static const std::string POSTFIX=")";

	static const std::string SNDDIR="sounds/";
	static const std::string SNDEXT=".wav";
	std::string::size_type start,finish;

	struct {std::string *a, *b;} strings[]=
	{
		{&StartSpeech[0],&StartSpeech[1]},
		{&FailedSpeech[0],&FailedSpeech[1]},
		{&CompleteSpeech[0],&CompleteSpeech[1]},
		{&PassedSpeech[0],&PassedSpeech[1]},
		{&PassedOKSpeech[0],&PassedOKSpeech[1]},
		{&InProgressSpeech[0],&InProgressSpeech[1]},
		{NULL,NULL}
	};

	for(int i=0;strings[i].a&&strings[i].b;i++)
	{
		start=strings[i].a->find(PREFIX);
		if(start!=strings[i].a->npos)
		{
			finish=strings[i].a->find(POSTFIX,start);
			if(finish==strings[i].a->npos) finish=strings[i].a->size();
			*strings[i].b=SNDDIR+std::string(*strings[i].a,start+PREFIX.size(),finish-start-PREFIX.size())+SNDEXT;
			*strings[i].a=std::string(*strings[i].a,0,start);
		}
	}

}
