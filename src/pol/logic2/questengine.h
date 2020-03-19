#ifndef QUESTENGINE_H
#define QUESTENGINE_H

//#define QUESTS_LOGGING

#ifdef QUESTS_LOGGING
#define QuestLog logFile["quest_test.log"]
#define JournalLog logFile["quest_journal.log"]
#else
#define QuestLog //logFile["quest_test.log"]
#define JournalLog //logFile["quest_journal.log"]
#endif
//fixme:������!
//#include "questManager.h"

struct phrase_s;

class Quest;

//=====================================================================================//
//                                 class LogicServer1                                  //
//=====================================================================================//
class LogicServer1
{
public:

    virtual ~LogicServer1(){}
    
    //
    // ������ ���������� ������ �� ������������ quest'y
    //
    class ReportBuilder
	{
    public:
        
        virtual ~ReportBuilder(){}
        
        //������ ����� ��� ������
        virtual void Start() = 0;
        //��������� ����� (������� ����� �� �����)
        virtual void Finish() = 0;
        
        enum event_type
		{
            ET_NONE,
            ET_START,  //������ ������
            ET_FINISH, //���������� ������
        };
        
        //����� ��������
        virtual void AddEvent(event_type type) = 0;
        //������� ���������� �� �����
        virtual void AddExperience(int experience) = 0;
        //������� ���������� �� �������� ���������
        virtual void AddItems(const std::string& sys_name) = 0;
    };
    
    //�������� ������ � ������ ����������� ������
    virtual ReportBuilder* GetReportBuilder() = 0;
    
    enum quest_event_type
	{
        QET_QUEST_FINISHED,   //��������� ������ 
        QET_NEW_QUEST_START,  //������ ������ (�� ����������) ������
    };

    //���������� � ������ ���������� ������ � QET_QUEST_FINISHED 
    struct quest_info
	{
        std::string m_quest;

        quest_info(const std::string& sys_name) : m_quest(sys_name) {}
    };
        
    //����������� � ������� � �������
    virtual void Notify(quest_event_type event, void* info = 0) = 0;

    //
    // ����� ��������������� �������� ��� ������ � ���������
    //
    class EntityManipulator
	{
    public:

        virtual ~EntityManipulator(){}

        //������ ��� ���������
		//    ET_NONE ,ET_HUMAN,ET_VEHICLE ,ET_TRADER, ET_ALL_ENTS 
        virtual entity_type GetType() const = 0;
        //������ ������ � �������� ����������� ��������
		//    PT_NONE,PT_ENEMY,PT_PLAYER,PT_CIVILIAN ,PT_ALL_PLAYERS
        virtual player_type GetPlayer() const = 0;
        //������ ��������� �������� � ������� ������ 
		//	  RT_NEUTRAL, RT_ENEMY, RT_FRIEND
        virtual RelationType GetRelation2Player() const = 0;

        //������ ��� ���������
        virtual const std::string& GetName() const = 0;
        //������ ��������� ��� ���������
        virtual const std::string& GetSysName() const = 0;

        //���� ������� ����
        virtual void GiveExperience(int value) = 0;
        //���� �������� �������
        virtual void GiveItems(const std::string &itemset_name) = 0;

        enum phrase_type
		{
            PT_USE,     //����� �� use
            PT_FIRST,   //������ �����
        };
        
        //������� �����
        virtual void SayPhrase(phrase_type type) = 0;
        virtual void SayPhrase(const phrase_s& phrase) = 0;

        //��������� �������� � ������ �������
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        //���� new_ai.empty() == true, �� � �������� �������� 
        //�� ������ ai � ������� ��� ���� ����������� �� ������
        virtual void ChangePlayer(player_type new_player, const std::string& new_ai = std::string()) = 0;
    };

    //
    // ������ ��� �������� �� ������� �������
    //
    class EntityIterator
	{
    public:

        virtual ~EntityIterator(){}

        //���������� ��������� ������������, ������� � ������� ��������
        virtual void First(const std::string& sys_name, player_type player) = 0;

        //������� � ��������� ��������
        virtual void Next() = 0;
        //��������� �� ��������
        virtual bool IsDone() const = 0;

        //�������� ����� �� ����������� ��������
        virtual EntityManipulator* Get() = 0;
    };

    //�������� ������ �� ������ ��������
    virtual EntityIterator* GetEntityIterator() = 0;
    
    //�������� MessageBox 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //�������:
    //         true  -  ���� ������ ������ OK(Yes)
    //         false -  ���� ������ ������ Cancel(No) 
    //
    virtual bool ShowMessageBox(const std::string& text) = 0;
    
    //�������� �������� � �������
    virtual void TakeItems(const std::string &itemset_name) = 0;
    //����� �� ������� ����� ��������� � �������
    virtual bool CanGetItems(const std::string &itemset_name) const = 0;

    //����������� �� ���������� �����
    virtual bool IsSceneFinished(const std::string &scene_name) const = 0;

    //�������� ������� ����
    virtual void ChangePhase(int Phase) = 0;
    //���������� ������� ������� ����
    virtual int  GetCurrentPhase() const = 0;
    //���������/��������� ����� �� �������
    virtual void EnableLevel(const std::string &lev_name, bool enable) = 0;

    // �������� ��������� ������ ���� � �����
    virtual void ChangeAlignment(const std::string &side_a, const std::string &side_b, RelationType type) = 0;
    
    //��������� ������� � ������
    virtual void AddNews(const std::string& sys_name) = 0;
    //��������� ������� ������ �� �������
    virtual void RemoveJournalTheme(const std::string& theme) = 0;
    //��������� ������ � ������
    virtual void AddJournalRecord(const std::string& theme, const std::string& text) = 0;
	//��������� ��� �� ��������� ���������� ������� (�����, ���������, ��������)
	virtual bool IsHeroDead(const std::string &sys_name) const = 0;

	static std::auto_ptr<LogicServer1> CreateLogicServer();
};

//=====================================================================================//
//                                 class QuestMessage1                                 //
//=====================================================================================//
class QuestMessage1
{
public:
	enum MESSAGE_TYPE {MT_USE, MT_KILL, MT_FIRSTUSE};

	LogicServer1::EntityManipulator *Subject; //�� ���� ���������
	LogicServer1::EntityManipulator *Actor;		//��� ���������     

	MESSAGE_TYPE Type;
	LogicServer1 *serv;

	QuestMessage1(){};
};

//=====================================================================================//
//                                  enum QUEST_STATE                                   //
//=====================================================================================//
enum QUEST_STATE
{
	QS_NOTSTARTED,      //��� �� �����
	QS_STARTED,         //�����
	QS_FAILED,          //��������  (� ���� ����������)
	QS_COMPLETE,        //��������
	QS_PASSED,          //�������� (������ ���� ��������� ��� �� ������)
	QS_PASSED_OK,       //�������� (��� �������� �� ������� ������)
	QS_MAXSTATES
};

//=====================================================================================//
//                                  class QuestEngine                                  //
//=====================================================================================//
class QuestEngine
{
public:
	typedef std::vector<Quest *> QuestList;
	//   static void  SendMessage (message_t& event);					// Send message to all quests

	static Quest *Get(const std::string &sys_name);
	static QUEST_STATE GetQuestState(const std::string &sys_name);
	static void Init();
	static void Close();
	static void Clear();
	static void MakeSaveLoad(SavSlot &slot);

	static void HandleMessage(QuestMessage1 &mes);
	static void ChangeEpisode(int num, LogicServer1 *serv);

private:
	static QuestList Quests;
	static int m_Episode;
};


//=====================================================================================//
//                              class QuestEngineJournal                               //
//=====================================================================================//
class QuestEngineJournal
{
public:
	friend class QuestEngine;
	friend class Quest;
protected:
	static void AddRecord(LogicServer1 *serv, const std::string &QuestName, const std::string &Description);
	static void RemoveTheme(LogicServer1 *serv, const std::string &QuestName);
};

#endif