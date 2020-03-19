#include "precomp.h"

#include "ActMgr.h"
#include "ActFactory.h"
#include "TaskDatabase.h"

namespace PoL
{

namespace TaskDataBaseDetails
{

//=====================================================================================//
//                              class ProcessMessageEntry                              //
//=====================================================================================//
class ProcessMessageEntry
{
	bool &m_flag;

public:
	ProcessMessageEntry(bool &flag)
	:	m_flag(flag)
	{
		assert( m_flag == false );
		m_flag = true;
	}

	~ProcessMessageEntry()
	{
		m_flag = false;
	}
};

//=====================================================================================//
//                              struct HandlersByPriority                              //
//=====================================================================================//
struct HandlersByPriority
{
	template<typename T>
	bool operator()(const T &l, const T &r) const
	{
		return l->m_priority > r->m_priority;
	}
};

//=====================================================================================//
//                         struct HandlerEnabledAndHasPriority                         //
//=====================================================================================//
struct HandlerEnabledAndHasPriority
{
	int m_priority;
	HandlerEnabledAndHasPriority(int p) : m_priority(p) {}

	template<typename T>
	bool operator()(const T &t) const
	{
		return t->m_enabled && t->m_priority == m_priority;
	}
};

TaskDatabase *g_instance = 0;

//=====================================================================================//
//                              class RandomPhrasesTable                               //
//=====================================================================================//
class RandomPhrasesTable
{
	typedef std::vector< std::vector<std::string> > PhaseMap_t;

	struct PersonalEntry
	{
		PhaseMap_t m_phasePhrase;
	};

	typedef std::hash_map<std::string,PersonalEntry> PersEntries_t;
	PersEntries_t m_persEntries;

public:
	RandomPhrasesTable()
	{
		using mll::utils::table;

		table tbl;

		{
			VFileWpr vfile("scripts/pol/random_phrases.txt");
			if(!vfile.get() || !vfile.get()->Size()) return;
			VFileBuf buf(vfile.get());
			std::istream in(&buf);
			in >> tbl;
		}

		table::row_type hdr = tbl.row(0);
		table::col_type persid = tbl.col(std::find(hdr.begin(),hdr.end(),"sys_name"));
		table::col_type phases = tbl.col(std::find(hdr.begin(),hdr.end(),"phases"));
		table::col_type phrases = tbl.col(std::find(hdr.begin(),hdr.end(),"phrases"));

		for(int i = 1; i < tbl.height(); ++i)
		{
			PersonalEntry &pentry = m_persEntries[persid[i]];
			std::vector< std::string > phrs;

			{
				typedef boost::char_separator<char> sep_t;
				typedef boost::tokenizer<sep_t> tok_t;
				sep_t sep(", ;:.\t");
				tok_t tok(phrases[i], sep);

				for(tok_t::iterator it = tok.begin(); it != tok.end(); ++it)
				{
					phrs.push_back(*it);
				}
			}

			{
				typedef boost::char_separator<char> sep_t;
				typedef boost::tokenizer<sep_t> tok_t;
				sep_t sep(",. :;\t");
				tok_t tok(phases[i], sep);

				for(tok_t::iterator it = tok.begin(); it != tok.end(); ++it)
				{
					int ph = boost::lexical_cast<int>(*it);
					if(pentry.m_phasePhrase.size() <= ph)
					{
						pentry.m_phasePhrase.resize(ph+1);
					}

					pentry.m_phasePhrase[ph].insert(pentry.m_phasePhrase[ph].end(),phrs.begin(),phrs.end());
				}
			}
		}
	}

	phrase_s getPhrase(const std::string &rid, int phase) const
	{
		PersEntries_t::const_iterator i = m_persEntries.find(rid);
		if(i == m_persEntries.end()) return phrase_s();
		const PersonalEntry &pentry = i->second;
		if(pentry.m_phasePhrase.size() <= phase) return phrase_s();
		if(pentry.m_phasePhrase[phase].empty()) return phrase_s();
		int index = rand() % pentry.m_phasePhrase[phase].size();
		return locPhr(pentry.m_phasePhrase[phase][index]);
	}
};

}

using namespace TaskDataBaseDetails;

//=====================================================================================//
//                            TaskDatabase::TaskDatabase()                             //
//=====================================================================================//
TaskDatabase::TaskDatabase()
:	m_processingMessage(false),
	m_processingUseMessage(false),
	m_messageCount(0)
{
	assert( g_instance == 0 );
	g_instance = this;
}

//=====================================================================================//
//                            TaskDatabase::~TaskDatabase()                            //
//=====================================================================================//
TaskDatabase::~TaskDatabase()
{
	assert( g_instance == this );
	g_instance = 0;
}

//=====================================================================================//
//                   TaskDatabase::Connection TaskDatabase::attach()                   //
//=====================================================================================//
TaskDatabase::Connection TaskDatabase::attach(HandlerData::Fn_t fn,
											  const std::string &name,
											  MessageType t)
{
	HandlerData data = {fn,true,0,name,t};
	Connection conn = m_handlers.insert(m_handlers.upper_bound(data),data);

	switch(t)
	{
	case mtKilled:
		if(isKilled(name)) fn();
		break;
	case mtDead:
		if(isDead(name)) fn();
		break;
	}

	return conn;
}

//=====================================================================================//
//                   TaskDatabase::Connection TaskDatabase::attach()                   //
//=====================================================================================//
TaskDatabase::Connection TaskDatabase::attach(HandlerData::Fn_t fn, MessageType t)
{
	HandlerData data = {fn,true,0,std::string(),t};
	Connection conn = m_handlers.insert(m_handlers.upper_bound(data),data);

	return conn;
}

//=====================================================================================//
//                            void TaskDatabase::doNotify()                            //
//=====================================================================================//
void TaskDatabase::doNotify(const HandlerData &dta)
{
	Handlers_t::iterator ib = m_handlers.lower_bound(dta);
	Handlers_t::iterator ie = ib;

	while(ie != m_handlers.end() && ie->m_mt == dta.m_mt && ie->m_subj == dta.m_subj) ++ie;
	
	typedef std::list<const HandlerData *> Hs_t;
	Hs_t handlers;
	for(Handlers_t::iterator i = ib; i != ie; ++i)
	{
		handlers.push_back(&*i);
	}

	if(handlers.empty()) return;

	handlers.sort(HandlersByPriority());
	Hs_t::iterator re = handlers.begin();

	do
	{
		handlers.erase(handlers.begin(),re);

		int prio = 0;
		for(Hs_t::iterator i = handlers.begin(); i != handlers.end(); ++i)
		{
			prio = (*i)->m_priority;
			if((*i)->m_enabled) break;
		}

		re = std::stable_partition(handlers.begin(),handlers.end(),HandlerEnabledAndHasPriority(prio));

		for(Hs_t::iterator i = handlers.begin(); i != re; ++i)
		{
			(*i)->m_fn();
		}
	}
	while(!handlers.empty() && handlers.begin() != re);
}

//=====================================================================================//
//                             void TaskDatabase::notify()                             //
//=====================================================================================//
void TaskDatabase::notify(MessageType mt)
{
	HandlerData dta = {0,true,0,std::string(),mt};
	doNotify(dta);
}

//=====================================================================================//
//                             void TaskDatabase::notify()                             //
//=====================================================================================//
void TaskDatabase::notify(const std::string &n, MessageType mt)
{
	HandlerData dta = {0,true,0,n,mt};
	doNotify(dta);
}

//=====================================================================================//
//                          void TaskDatabase::onUseObject()                           //
//=====================================================================================//
void TaskDatabase::onUseObject(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	if(!data.m_subject.empty()) ++m_usedList[data.m_subject]; //.insert(data.m_subject);

	ACT_LOG("INFO: object [" << data.m_subject << "] added to used list\n");

	notify(data.m_subject,mtItemUsed);
}

//=====================================================================================//
//                         void TaskDatabase::addToSaidList()                          //
//=====================================================================================//
void TaskDatabase::addToSaidList( ::eid_t id )
{
	if(m_processingUseMessage)
	{
		m_saidList.insert(id);
	}
}

//=====================================================================================//
//                       bool TaskDatabase::isInSaidList() const                       //
//=====================================================================================//
bool TaskDatabase::isInSaidList( ::eid_t id ) const
{
	return m_saidList.find(id) != m_saidList.end();
}

//=====================================================================================//
//                         void TaskDatabase::clearSaidList()                          //
//=====================================================================================//
void TaskDatabase::clearSaidList()
{
	m_saidList.clear();
}

//=====================================================================================//
//                          void TaskDatabase::onUseEntity()                           //
//=====================================================================================//
void TaskDatabase::onUseEntity(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	ProcessMessageEntry pmentry2(m_processingUseMessage);
	m_lastMsg = data;
	++m_messageCount;

	//bool needSay = !ActFactory::instance()->hasActiveQuests(data.m_subject);

	if(!data.m_subject.empty())
	{
		++m_usedList[data.m_subject];
		ACT_LOG("INFO: person [" << data.m_subject << "] added to used list\n");
		notify(data.m_subject,mtPersonUsed);

		if(!makeAddHeroToTeam(getEntityById(data.m_subject_id))
			//&& needSay
			//&& !ActFactory::instance()->hasActiveQuests(data.m_subject)
			&& !isInSaidList(data.m_subject_id)
			&& !ActMgr::isPlayingScene())
		{
			sayRandomPhrase(data.m_subject_id);
		}
		clearSaidList();
	}
}

//=====================================================================================//
//                        void TaskDatabase::sayRandomPhrase()                         //
//=====================================================================================//
void TaskDatabase::sayRandomPhrase(eid_t subjid)
{
	static RandomPhrasesTable rpt;

	::BaseEntity *entity = getEntityById(subjid);
	const std::string &rid = entity->GetInfo()->GetRID();

	::phrase_s phr = rpt.getPhrase(rid,getPhase());
	if(!phr.m_text.empty())
	{
		::Forms::GetInst()->ShowTalkDialog(entity,phr);
	}
}

//=====================================================================================//
//                          void TaskDatabase::onKillEntity()                          //
//=====================================================================================//
void TaskDatabase::onKillEntity(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	bool killed = false;

	if(!data.m_subject.empty())
	{
		::BaseEntity *e = ::EntityPool::GetInst()->Get(data.m_subject_id);
		m_deadPositions.insert(std::make_pair(data.m_subject,e->GetGraph()->GetPos3()));
		++m_deadList[data.m_subject]; //.insert(data.m_subject);
		ACT_LOG("INFO: person [" << data.m_subject << "] added to dead list\n");
	}

	if(!data.m_object.empty())
	{
		::BaseEntity *e = ::EntityPool::GetInst()->Get(data.m_object_id);
		if(e && e->GetPlayer() == PT_PLAYER)
		{
			++m_killedList[data.m_subject]; //.insert(data.m_subject);
			killed = true;
			ACT_LOG("INFO: person [" << data.m_subject << "] added to killed list\n");
		}
	}

	if(killed)
	{
		notify(data.m_subject,mtKilled);
	}

	notify(data.m_subject,mtDead);
	notify(mtDead);
}

//=====================================================================================//
//                          void TaskDatabase::onStartTurn()                           //
//=====================================================================================//
void TaskDatabase::onStartTurn(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtStartTurn);
}

//=====================================================================================//
//                           void TaskDatabase::onEndTurn()                            //
//=====================================================================================//
void TaskDatabase::onEndTurn(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtEndTurn);
}

//=====================================================================================//
//                        void TaskDatabase::onEntitySpawned()                         //
//=====================================================================================//
void TaskDatabase::onEntitySpawned(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtEntitySpawned);
	notify(data.m_subject,mtNamedEntitySpawned);
}

//=====================================================================================//
//                      void TaskDatabase::onEntityTeamChanged()                       //
//=====================================================================================//
void TaskDatabase::onEntityTeamChanged(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(data.m_subject,mtHeroTeamChanged);
	notify(mtHeroTeamChanged);
}

//=====================================================================================//
//                        void TaskDatabase::onEpisodeChanged()                        //
//=====================================================================================//
void TaskDatabase::onEpisodeChanged(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtEpisodeChanged);
}

//=====================================================================================//
//                          void TaskDatabase::onLeaveLevel()                          //
//=====================================================================================//
void TaskDatabase::onLeaveLevel(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(data.m_level,mtLeaveLevel);
	notify(mtLeaveLevel);
}

//=====================================================================================//
//                          void TaskDatabase::onEnterLevel()                          //
//=====================================================================================//
void TaskDatabase::onEnterLevel(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtEnterLevel);
}

//=====================================================================================//
//                         void TaskDatabase::onPhaseChange()                          //
//=====================================================================================//
void TaskDatabase::onPhaseChange(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtPhaseChanged);
}

//=====================================================================================//
//                       void TaskDatabase::onEntityDestroyed()                        //
//=====================================================================================//
void TaskDatabase::onEntityDestroyed(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtEntityDestroyed);
}

//=====================================================================================//
//                       void TaskDatabase::onRelationChanged()                        //
//=====================================================================================//
void TaskDatabase::onRelationChanged(const EventData &data)
{
	ProcessMessageEntry pmentry(m_processingMessage);
	m_lastMsg = data;
	++m_messageCount;

	notify(mtRelationChanged);
}

//=====================================================================================//
//                          void TaskDatabase::store() const                           //
//=====================================================================================//
void TaskDatabase::store(mll::io::obinstream &out) const
{
	out << asInt(m_deadList.size());
	for(ItemList_t::const_iterator i = m_deadList.begin(); i != m_deadList.end(); ++i)
	{
		out << i->first << i->second;
	}

	out << asInt(m_killedList.size());
	for(ItemList_t::const_iterator i = m_killedList.begin(); i != m_killedList.end(); ++i)
	{
		out << i->first << i->second;
	}

	out << asInt(m_usedList.size());
	for(ItemList_t::const_iterator i = m_usedList.begin(); i != m_usedList.end(); ++i)
	{
		out << i->first << i->second;
	}
}

//=====================================================================================//
//                            void TaskDatabase::restore()                             //
//=====================================================================================//
void TaskDatabase::restore(mll::io::ibinstream &in)
{
	reset();

	m_deadPositions.clear();
	//assert( m_deadList.empty() );
	//assert( m_killedList.empty() );
	//assert( m_usedList.empty() );

	ItemList_t::size_type size;
	std::string name;
	int ctr;

	in >> asInt(size);
	for(ItemList_t::size_type i = 0; i < size; ++i)
	{
		in >> name >> ctr;
		m_deadList.insert(std::make_pair(name,ctr));
	}

	in >> asInt(size);
	for(ItemList_t::size_type i = 0; i < size; ++i)
	{
		in >> name >> ctr;
		m_killedList.insert(std::make_pair(name,ctr));
	}

	in >> asInt(size);
	for(ItemList_t::size_type i = 0; i < size; ++i)
	{
		in >> name >> ctr;
		m_usedList.insert(std::make_pair(name,ctr));
	}
}

//=====================================================================================//
//                             void TaskDatabase::reset()                              //
//=====================================================================================//
void TaskDatabase::reset()
{
	m_deadList.clear();
	m_killedList.clear();
	m_usedList.clear();
	m_deadPositions.clear();
}

//=====================================================================================//
//                        bool TaskDatabase::getDeadEntityPos()                        //
//=====================================================================================//
bool TaskDatabase::getDeadEntityPos(const std::string &name, point3 *pos)
{
	DeadPositions_t::iterator i = m_deadPositions.find(name);
	if(i == m_deadPositions.end()) return false;
	*pos = i->second;
	return true;
}

//=====================================================================================//
//                       TaskDatabase *TaskDatabase::instance()                        //
//=====================================================================================//
TaskDatabase *TaskDatabase::instance()
{
	return g_instance;
}

}