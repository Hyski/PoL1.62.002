#include "precomp.h"

#include "Act.h"
#include "Task.h"
#include "Quest.h"
#include "Trigger.h"
#include "Counter.h"
#include "TaskList.h"
#include "Condition.h"
#include "DullSizer.h"
#include "ActFactory.h"
#include "ActTemplate.h"
#include "PreciousSizer.h"
#include "FileSystemFront.h"
#include "ActTreeParser.hpp"
#include "ActScriptLexer.hpp"
#include "ActScriptParser.hpp"

namespace PoL
{

namespace ActFactoryDetail
{

static const Persistent::Id_t PrefixMask		= 0xFF000000;
static const Persistent::Id_t InvPrefixMask		= 0x00FFFFFF;
static const Persistent::Id_t PrefixShift		= 24;

static const Persistent::Id_t QuestPrefix		= 0x01000000;
static const Persistent::Id_t ConditionPrefix	= 0x02000000;
static const Persistent::Id_t TaskPrefix		= 0x03000000;
static const Persistent::Id_t TriggerPrefix		= 0x04000000;
static const Persistent::Id_t CounterPrefix		= 0x05000000;

//=====================================================================================//
//                                   struct Compare                                    //
//=====================================================================================//
struct Compare
{
	bool operator()(const HPersistent &l, const HPersistent &r) const
	{
		return l->getId() < r->getId();
	}

	template<typename T>
	bool operator()(const T &l, const HPersistent &r) const
	{
		return l->getId() < r->getId();
	}

	template<typename T>
	bool operator()(const HPersistent &l, const T &r) const
	{
		return l->getId() < r->getId();
	}
};

//=====================================================================================//
//                                struct PersistByName                                 //
//=====================================================================================//
struct PersistByName
{
	const std::string &m_name;

	PersistByName(const std::string &n) : m_name(n) {}

	bool operator()(const HPersistent &l) const
	{
		return l->getName() == m_name;
	}
};

//=====================================================================================//
//                                struct ActFactoryData                                //
//=====================================================================================//
struct ActFactoryData
{
	struct Item
	{
		std::string m_name;
		HActTemplate m_template;

		Item(const std::string &name, const HActTemplate &tmpl)
		:	m_name(name),
			m_template(tmpl)
		{
		}

		friend bool operator<(const Item &l, const Item &r)
		{
			return l.m_name < r.m_name;
		}

		friend bool operator<(const std::string &l, const Item &r)
		{
			return l < r.m_name;
		}

		friend bool operator<(const Item &l, const std::string &r)
		{
			return l.m_name < r;
		}
	};

	typedef std::vector<Item> Items_t;
	Items_t m_items;

	ActFactoryData()
	{
	}

	~ActFactoryData()
	{
	}
};

static ActAttachQueue *g_instance = 0;
static ActFactory *g_ainstance = 0;
static ActFactoryData *g_dinstance = 0;

//=====================================================================================//
//                              ActFactoryData &getData()                              //
//=====================================================================================//
ActFactoryData &getData()
{
	if(!g_dinstance) g_dinstance = new ActFactoryData;
	return *g_dinstance;
}

}

using namespace ActFactoryDetail;

//=====================================================================================//
//                                     void walk()                                     //
//=====================================================================================//
void walk(const ActScriptParser &parser, antlr::RefAST t, unsigned int depth = 0)
{
	if(t == antlr::nullAST) return;

	for(antlr::RefAST a = t; a != antlr::nullAST; a = a->getNextSibling())
	{
		ACT_LOG(std::string(depth*4,' '));
		ACT_LOG(parser.getTokenName(a->getType()) << " " << a->getText() << "\n");
		walk(parser,a->getFirstChild(),depth+1);
	}
}

//=====================================================================================//
//                     ActAttachQueue *ActAttachQueue::instance()                      //
//=====================================================================================//
ActAttachQueue *ActAttachQueue::instance()
{
	return g_instance;
}

//=====================================================================================//
//                          ActAttachQueue::ActAttachQueue()                           //
//=====================================================================================//
ActAttachQueue::ActAttachQueue()
{
	assert( g_instance == 0 );
	g_instance = this;
}

//=====================================================================================//
//                          ActAttachQueue::~ActAttachQueue()                          //
//=====================================================================================//
ActAttachQueue::~ActAttachQueue()
{
	assert( g_instance == this );
	g_instance = 0;
}

//=====================================================================================//
//                        void ActAttachQueue::queueActAttach()                        //
//=====================================================================================//
void ActAttachQueue::queueActAttach(HActTemplate at, const std::string &n,
									ActTemplate::Iterator_t it)
{
	ActAttach aa = {at,n,it};
	m_actAttachQueue.push_back(aa);
}

//=====================================================================================//
//                              static void makeRefAct()                               //
//=====================================================================================//
static void makeRefAct(HActTemplate tmpl,
					   ActTemplate::Iterator_t sit,
					   ActTemplate::Iterator_t seit,
					   ActTemplate::Iterator_t oit)
{
	for(ActTemplate::Iterator_t i = sit; i != seit; i.brother())
	{
		ActTemplate::Iterator_t noit = tmpl->addActCreator(*i,oit);
		if(i.has_son())
		{
			makeRefAct(tmpl,i.get_son(),seit,noit);
		}
	}
}

//=====================================================================================//
//                              ActFactory::ActFactory()                               //
//=====================================================================================//
ActFactory::ActFactory()
:	m_uniqueCtr(0)
{
	assert( g_ainstance == 0 );
	g_ainstance = this;

	ActAttachQueue aaq;

	typedef std::vector<std::string> Files_t;
	Files_t files = FileSystemFront::getFiles("scripts/acts/","act");
	std::sort(files.begin(),files.end());

	for(Files_t::iterator i = files.begin(); i != files.end(); ++i)
	{
		std::istringstream sstr(FileSystemFront::getFileContent(*i),std::ios::binary);
	
		ACT_LOG("----INFO: parsing file [" << *i << "]\n");

		try
		{
			ActScriptLexer lexer(sstr);
			lexer.setTabsize(4);

			ActScriptParser parser(lexer);
			antlr::ASTFactory astfactory;

			parser.initializeASTFactory(astfactory);
			parser.setASTFactory(&astfactory);

			parser.setFilename(*i);
			parser.program();

			ActTreeParser treeparser;
			treeparser.setFilename(*i);
			treeparser.program(parser.getAST());

			ACT_LOG("----INFO: parsed file [" << *i << "]\n");

			//ACT_LOG("====================== " << *i << "\n");
			//walk(parser,parser.getAST());
		}
		catch(antlr::ANTLRException &ae)
		{
			ACT_LOG("----WARNING: finished parsing file [" << *i << "] with error: " << ae.getMessage() << "\n");
		}
	}

	using ActAttachQueue::ActAttachQueue_t;
	for(ActAttachQueue_t::iterator i = aaq.m_actAttachQueue.begin(); i != aaq.m_actAttachQueue.end(); ++i)
	{
		PoL::HActTemplate tmpl2 = getActTemplate(i->m_name);
		if(tmpl2)
		{
			makeRefAct(i->m_at,tmpl2->m_creators.begin(),tmpl2->m_creators.end(),i->m_itor);
		}
	}

	ACT_LOG("INFO: Initialization phase over; PoL::ActFactory initialized\n");
}

//=====================================================================================//
//                              ActFactory::~ActFactory()                              //
//=====================================================================================//
ActFactory::~ActFactory()
{
	delete g_dinstance;
	assert( g_ainstance == this );
	g_ainstance = 0;
}

//=====================================================================================//
//                              HAct ActFactory::create()                              //
//=====================================================================================//
HAct ActFactory::create(const std::string &name)
{
	HActTemplate tmpl = getActTemplate(name);
	if(tmpl) return tmpl->createAct();
	return 0;
}

//=====================================================================================//
//                      HActTemplate ActFactory::getActTemplate()                      //
//=====================================================================================//
HActTemplate ActFactory::getActTemplate(const std::string &name)
{
	std::string realname = name;
	std::transform(realname.begin(),realname.end(),realname.begin(),std::tolower);

	ActFactoryData::Items_t::iterator i = 
		std::lower_bound(getData().m_items.begin(), getData().m_items.end(), realname);

	if(i == getData().m_items.end() || i->m_name != realname)
	{
		ACT_LOG("WARNING: tried to create act [" << realname << "] but it haven't found; returned 0\n");
		return 0;
	}

	ACT_LOG("INFO: created act [" << realname << "]\n");
	return i->m_template;
}

//=====================================================================================//
//                         ActFactory *ActFactory::instance()                          //
//=====================================================================================//
ActFactory *ActFactory::instance()
{
	return g_ainstance;
}

//=====================================================================================//
//                       void ActFactory::registerActTemplate()                        //
//=====================================================================================//
void ActFactory::registerActTemplate(const std::string &name, HActTemplate tmpl)
{
	std::string realname = name;
	std::transform(realname.begin(),realname.end(),realname.begin(),std::tolower);

	ActFactoryData::Items_t::iterator i = 
		std::lower_bound(getData().m_items.begin(), getData().m_items.end(), realname);

	if(i != getData().m_items.end() && i->m_name == realname)
	{
		ACT_LOG("WARNING: name intersection [" << realname << "]; act not registered\n");
	}
	else
	{
		//ACT_LOG("INFO: registered act [" << realname << "]\n");
		getData().m_items.insert(i,ActFactoryData::Item(realname,tmpl));
	}
}

//=====================================================================================//
//                          void ActFactory::registerQuest()                           //
//=====================================================================================//
void ActFactory::registerQuest(HQuest quest)
{
	if(m_persists.empty() || (quest->getId()&InvPrefixMask) > (m_persists.back()->getId()&InvPrefixMask))
	{
		m_persists.resize((quest->getId()&InvPrefixMask)+1);
	}

	m_persists[quest->getId()&InvPrefixMask] = quest.get();

	QuestRefs_t::iterator qr = m_questRefs.find(quest->getName());
	if(qr != m_questRefs.end()) qr->second->setQuest(quest.get());

	//ACT_LOG("INFO: registered quest [" << m_idMap[quest->getId()&InvPrefixMask] << "] with id ["
		//<< std::hex << quest->getId() << "]\n");
}

//=====================================================================================//
//                           void ActFactory::registerTask()                           //
//=====================================================================================//
void ActFactory::registerTask(HTask task)
{
	if(m_persists.empty() || (task->getId()&InvPrefixMask) > (m_persists.back()->getId()&InvPrefixMask))
	{
		m_persists.resize((task->getId()&InvPrefixMask)+1);
	}

	m_persists[task->getId()&InvPrefixMask] = task.get();

	//ACT_LOG("INFO: registered task [" << m_idMap[task->getId()&InvPrefixMask] << "] with id ["
	//	<< std::hex << task->getId() << "]\n");
}

//=====================================================================================//
//                        void ActFactory::registerCondition()                         //
//=====================================================================================//
void ActFactory::registerCondition(HCondition cond)
{
	if(m_persists.empty() || (cond->getId()&InvPrefixMask) > (m_persists.back()->getId()&InvPrefixMask))
	{
		m_persists.resize((cond->getId()&InvPrefixMask)+1);
	}

	m_persists[cond->getId()&InvPrefixMask] = cond.get();

	//ACT_LOG("INFO: registered condition [" << m_idMap[cond->getId()&InvPrefixMask] << "] with id ["
	//	<< std::hex << cond->getId() << "]\n");
}

//=====================================================================================//
//                         void ActFactory::registerCounter()                          //
//=====================================================================================//
void ActFactory::registerCounter(HCounter counter)
{
	if(m_persists.empty() || (counter->getId()&InvPrefixMask) > (m_persists.back()->getId()&InvPrefixMask))
	{
		m_persists.resize((counter->getId()&InvPrefixMask)+1);
	}

	m_persists[counter->getId()&InvPrefixMask] = counter.get();

	//ACT_LOG("INFO: registered counter [" << m_idMap[counter->getId()&InvPrefixMask] << "] with id ["
	//	<< std::hex << counter->getId() << "]\n");
}

//=====================================================================================//
//                         void ActFactory::registerTrigger()                          //
//=====================================================================================//
void ActFactory::registerTrigger(HTrigger trigger)
{
	if(m_persists.empty() || (trigger->getId()&InvPrefixMask) > (m_persists.back()->getId()&InvPrefixMask))
	{
		m_persists.resize((trigger->getId()&InvPrefixMask)+1);
	}

	m_persists[trigger->getId()&InvPrefixMask] = trigger.get();

	//ACT_LOG("INFO: registered trigger [" << m_idMap[trigger->getId()&InvPrefixMask] << "] with id ["
	//	<< std::hex << trigger->getId() << "]\n");
}

//=====================================================================================//
//                      HQuest ActFactory::getQuestByName() const                      //
//=====================================================================================//
HQuest ActFactory::getQuestByName(const std::string &name) const
{
	NameMap_t::const_iterator i = m_questMap.find(name);
	if(i != m_questMap.end()) return static_cast<Quest*>(m_persists[i->second&InvPrefixMask].get());
	return 0;
}

//=====================================================================================//
//                       HTask ActFactory::getTaskByName() const                       //
//=====================================================================================//
HTask ActFactory::getTaskByName(const std::string &name) const
{
	NameMap_t::const_iterator i = m_taskMap.find(name);
	if(i != m_taskMap.end()) return static_cast<Task*>(m_persists[i->second&InvPrefixMask].get());
	return 0;
}

//=====================================================================================//
//                  HCondition ActFactory::getConditionByName() const                  //
//=====================================================================================//
HCondition ActFactory::getConditionByName(const std::string &name) const
{
	NameMap_t::const_iterator i = m_conditionMap.find(name);
	if(i != m_conditionMap.end()) return static_cast<Condition*>(m_persists[i->second&InvPrefixMask].get());
	return 0;
}

//=====================================================================================//
//                      HQuestRef ActFactory::getQuestRefByName()                      //
//=====================================================================================//
HQuestRef ActFactory::getQuestRefByName(const std::string &name)
{
	QuestRefs_t::iterator i = m_questRefs.find(name);
	if(i == m_questRefs.end())
	{
		i = m_questRefs.insert(std::make_pair(name,new QuestRef)).first;
		if(HQuest quest = getQuestByName(name))
		{
			i->second->setQuest(quest.get());
		}
	}

	return i->second;
}

//=====================================================================================//
//                    HCounter ActFactory::getCounterByName() const                    //
//=====================================================================================//
HCounter ActFactory::getCounterByName(const std::string &name) const
{
	NameMap_t::const_iterator i = m_counterMap.find(name);
	if(i != m_counterMap.end()) return static_cast<Counter*>(m_persists[i->second&InvPrefixMask].get());
	return 0;
}

//=====================================================================================//
//                    HTrigger ActFactory::getTriggerByName() const                    //
//=====================================================================================//
HTrigger ActFactory::getTriggerByName(const std::string &name) const
{
	NameMap_t::const_iterator i = m_triggerMap.find(name);
	if(i != m_triggerMap.end()) return static_cast<Trigger*>(m_persists[i->second&InvPrefixMask].get());
	return 0;
}

//=====================================================================================//
//                    Persistent::Id_t ActFactory::reserveQuestId()                    //
//=====================================================================================//
Persistent::Id_t ActFactory::reserveQuestId(const std::string &name)
{
    assert( m_questMap.find(name) == m_questMap.end() );
	Persistent::Id_t result = m_questMap[name] = QuestPrefix | m_uniqueCtr++;
	m_idMap.push_back(name);
	//ACT_LOG("INFO: reserved quest id [" << std::hex << result << "] for name [" << name << "]\n");
	return result;
}

//=====================================================================================//
//                    Persistent::Id_t ActFactory::reserveTaskId()                     //
//=====================================================================================//
Persistent::Id_t ActFactory::reserveTaskId(const std::string &name)
{
	assert( m_taskMap.find(name) == m_taskMap.end() );
	Persistent::Id_t result = m_taskMap[name] = TaskPrefix | m_uniqueCtr++;
	m_idMap.push_back(name);
	//ACT_LOG("INFO: reserved task id [" << std::hex << result << "] for name [" << name << "]\n");
	return result;
}

//=====================================================================================//
//                  Persistent::Id_t ActFactory::reserveConditionId()                  //
//=====================================================================================//
Persistent::Id_t ActFactory::reserveConditionId(const std::string &name)
{
	assert( m_conditionMap.find(name) == m_conditionMap.end() );
	Persistent::Id_t result = m_conditionMap[name] = ConditionPrefix | m_uniqueCtr++;
	m_idMap.push_back(name);
	//ACT_LOG("INFO: reserved condition id [" << std::hex << result << "] for name [" << name << "]\n");
	return result;
}

//=====================================================================================//
//                   Persistent::Id_t ActFactory::reserveTriggerId()                   //
//=====================================================================================//
Persistent::Id_t ActFactory::reserveTriggerId(const std::string &name)
{
	assert( m_triggerMap.find(name) == m_triggerMap.end() );
	Persistent::Id_t result = m_triggerMap[name] = TriggerPrefix | m_uniqueCtr++;
	m_idMap.push_back(name);
	//ACT_LOG("INFO: reserved trigger id [" << std::hex << result << "] for name [" << name << "]\n");
	return result;
}

//=====================================================================================//
//                   Persistent::Id_t ActFactory::reserveCounterId()                   //
//=====================================================================================//
Persistent::Id_t ActFactory::reserveCounterId(const std::string &name)
{
	assert( m_counterMap.find(name) == m_counterMap.end() );
	Persistent::Id_t result = m_counterMap[name] = CounterPrefix | m_uniqueCtr++;
	m_idMap.push_back(name);
	//ACT_LOG("INFO: reserved counter id [" << std::hex << result << "] for name [" << name << "]\n");
	return result;
}

//=====================================================================================//
//                const std::string &ActFactory::getNameFromId() const                 //
//=====================================================================================//
const std::string &ActFactory::getNameFromId(Persistent::Id_t id) const
{
	static std::string errorName = "Error: id not found";
	if(m_idMap.size() < (id&InvPrefixMask)) return errorName;
	return m_idMap[id&InvPrefixMask];
}

//=====================================================================================//
//                         void ActFactory::addAvailableHero()                         //
//=====================================================================================//
void ActFactory::addAvailableHero(const std::string &h)
{
	++m_availHeroes[h];
}

//=====================================================================================//
//                       void ActFactory::removeAvailableHero()                        //
//=====================================================================================//
void ActFactory::removeAvailableHero(const std::string &h)
{
	--m_availHeroes[h];
}

//=====================================================================================//
//                      bool ActFactory::isHeroAvailable() const                       //
//=====================================================================================//
bool ActFactory::isHeroAvailable(const std::string &h) const
{
	HeroList_t::const_iterator i = m_availHeroes.find(h);
	return i != m_availHeroes.end() && i->second > 0;
}

//=====================================================================================//
//                      bool ActFactory::hasActiveQuests() const                       //
//=====================================================================================//
bool ActFactory::hasActiveQuests(const std::string &holder) const
{
	for(NameMap_t::const_iterator i = m_questMap.begin(); i != m_questMap.end(); ++i)
	{
		HQuest quest = static_cast<Quest*>(m_persists[i->second&InvPrefixMask].get());
		if(quest->getHolder() == holder && quest->isStarted() && !quest->isPassed()) return true;
	}
	return false;
}

//=====================================================================================//
//                           void ActFactory::store() const                            //
//=====================================================================================//
void ActFactory::store(mll::io::obinstream &out) const
{
	out << asInt(0xFFFFFFFE);
	out << asInt(m_availHeroes.size());

	for(HeroList_t::const_iterator i = m_availHeroes.begin(); i != m_availHeroes.end(); ++i)
	{
		out << i->first;
		out << asInt(i->second);
	}

	out << asInt(m_persists.size());

	for(Persists_t::const_iterator i = m_persists.begin(); i != m_persists.end(); ++i)
	{
		std::ostringstream sstr;
		mll::io::obinstream iout(sstr);
		(*i)->store(iout);

		out << (*i)->getName();
		out << asInt((*i)->getType());
		out << asInt(sstr.str().size());
		(*i)->store(out);
	}
}

//=====================================================================================//
//                             void ActFactory::restore()                              //
//=====================================================================================//
void ActFactory::restore(mll::io::ibinstream &in, unsigned int ver)
{
	size_t count;
	in >> asInt(count);

	size_t countsave = count;

	if(count == 0xFFFFFFFF || count == 0xFFFFFFFE)
	{
		size_t herocount;
		in >> asInt(herocount);

		for(size_t i = 0; i < herocount; ++i)
		{
			std::string name;
			int num;
			in >> name >> asInt(num);
			m_availHeroes.insert(std::make_pair(name,num));
		}

		in >> count;
	}

	Persists_t::iterator it = m_persists.begin();
	std::auto_ptr<Sizer> sizer;
	
	if(countsave != 0xFFFFFFFF && countsave != 0xFFFFFFFE)
	{
		countsave = 0xFFFFFFFF;
	}

	if(countsave < 0xFFFFFFFF)
	{
		sizer.reset(new PreciousSizer);
	}
	else
	{
		sizer.reset(new DullSizer(ver));
	}

	for(size_t i = 0; i < count; ++i)
	{
		std::string name;
		in >> name;

		SizerTypes tps = sizer->getEntryType(in,name);

		Persists_t::iterator pit = std::find_if(it,m_persists.end(),PersistByName(name));
		if(pit == m_persists.end()) pit = std::find_if(m_persists.begin(),it,PersistByName(name));
		if(pit == m_persists.end())
		{
			sizer->skipEntry(in,name);
		}
		else
		{
			if((*pit)->getType() == tps)
			{
				if(countsave != 0xFFFFFFFF)
				{
					int sz;
					in >> asInt(sz);
				}
				(*pit)->restore(in);
			}
			else
			{
				ACT_LOG("WARNING: load: skipped object [" << name << "] due to type mismatch. Are scripts changed after save?\n");
				sizer->skipEntry(in,name);
			}

			it = pit;
			++it;
		}
	}
}

//=====================================================================================//
//                              void ActFactory::reset()                               //
//=====================================================================================//
void ActFactory::reset()
{
	for(Persists_t::iterator i = m_persists.begin(); i != m_persists.end(); ++i)
	{
		if(*i) (*i)->reset();
	}
	m_availHeroes.clear();
}

//=====================================================================================//
//                            void ActFactory::dumpState()                             //
//=====================================================================================//
void ActFactory::dumpState()
{
	std::ostringstream sstr;
	for(Persists_t::iterator i = m_persists.begin(); i != m_persists.end(); ++i)
	{
		if(*i) (*i)->dumpState(sstr);
	}
	ACT_LOG("-------- storyline state dump\n");
	ACT_LOG(sstr.str());
	ACT_LOG("-------- storyline state dump end\n");
}

//=====================================================================================//
//                       void ActFactory::dumpScriptParameters()                       //
//=====================================================================================//
void ActFactory::dumpScriptParameters()
{
	std::ofstream out("compat/dump.types",std::ios::binary);
	mll::io::obinstream bout(out);

	typedef std::vector<std::string> TaskLists_t;
	TaskLists_t taskLists;

	bout << asInt(m_persists.size());

	for(Persists_t::iterator i = m_persists.begin(); i != m_persists.end(); ++i)
	{
		HPersistent p = *i;
		if(p->getType() == stTaskList)
		{
			taskLists.push_back(p->getName());
		}

		bout << p->getName();
		bout << asInt(p->getType());
	}

	bout << asInt(taskLists.size());

	for(TaskLists_t::iterator i = taskLists.begin(); i != taskLists.end(); ++i)
	{
		bout << *i;
		HTask tsk = getTaskByName(*i);
		MlHandle<TaskList> tlst = static_cast<TaskList *>(tsk.get());
		bout << asInt(tlst->getSubTaskCount());
	}
}

}