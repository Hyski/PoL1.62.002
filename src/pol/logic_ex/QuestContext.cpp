#include "precomp.h"
#include "QuestContext.h"

namespace PoL
{

namespace QuestContextDetails
{

using QuestContext::ContextId_t;

//=====================================================================================//
//                                    class Context                                    //
//=====================================================================================//
class Context
{
	ContextId_t m_id;
	std::string m_holder;
	std::string m_actor;

public:
	Context(ContextId_t id, const std::string &holder, const std::string &actor)
	:	m_id(id), m_holder(holder), m_actor(actor) {}

	ContextId_t getContextId() const { return m_id; }
	const std::string &getActor() const { return m_actor; }
	const std::string &getHolder() const { return m_holder; }
};

typedef std::list<Context> Contexts_t;

ContextId_t g_id = 0;
Contexts_t g_contexts;

}

using namespace QuestContextDetails;

//=====================================================================================//
//                      ContextId_t QuestContext::createContext()                      //
//=====================================================================================//
ContextId_t QuestContext::createContext(const std::string &holder, const std::string &actor)
{
	g_contexts.push_back(Context(g_id++,holder,actor));
	return g_contexts.back().getContextId();
}

//=====================================================================================//
//                         void QuestContext::removeContext()                          //
//=====================================================================================//
void QuestContext::removeContext(ContextId_t id)
{
	for(Contexts_t::iterator i = g_contexts.begin(); i != g_contexts.end(); ++i)
	{
		if(i->getContextId() == id)
		{
			g_contexts.erase(i);
			return;
		}
	}
}

//=====================================================================================//
//                     const std::string &QuestContext::getActor()                     //
//=====================================================================================//
const std::string &QuestContext::getActor()
{
	static std::string noactor;
	if(g_contexts.empty()) return noactor;
	return g_contexts.back().getActor();
}

//=====================================================================================//
//                    const std::string &QuestContext::getHolder()                     //
//=====================================================================================//
const std::string &QuestContext::getHolder()
{
	static std::string noholder;
	if(g_contexts.empty()) return noholder;
	return g_contexts.back().getHolder();
}

}