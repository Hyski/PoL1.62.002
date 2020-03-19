#include "precomp.h"
#include "Bureau2.h"
#include "Utils.h"
#include <logic2/Bureau.h>
#include <logic2/StoreBox.h>
#include <logic2/ThingFactory.h>

namespace PoL
{

static Bureau2 *g_inst = 0;

namespace Bureau2Details
{

//=====================================================================================//
//             class StoreBoxAcceptor : public ThingScriptParser::Acceptor             //
//=====================================================================================//
class StoreBoxAcceptor : public ThingScriptParser::Acceptor
{
	HStoreBox2 m_box;

public:
	StoreBoxAcceptor(HStoreBox2 box) : m_box(box) {}

	bool AcceptThing(BaseThing *thing)
	{
		m_box->add(thing);
		return true;
	}

	//принять информцию об организации
	void AcceptOrgInfo(const rid_t rid) {}
	//принять информацию о существах
	void AcceptEntInfo(entity_type type, const rid_t &rid) {}
	//принять информацию о предметах
	void AcceptThingInfo(thing_type type, const rid_t &rid) {}
};

}

using namespace Bureau2Details;

//=====================================================================================//
//                           static std::string makeFullId()                           //
//=====================================================================================//
static std::string makeFullId(const std::string &id)
{
	if(!id.empty() && id[0] == '$') return id;
	return getLevel() + ":" + id;
}

//=====================================================================================//
//                                 Bureau2::Bureau2()                                  //
//=====================================================================================//
Bureau2::Bureau2()
{
	assert( g_inst == 0 );
	g_inst = this;
}

//=====================================================================================//
//                                 Bureau2::~Bureau2()                                 //
//=====================================================================================//
Bureau2::~Bureau2()
{
	assert( g_inst == this );
	g_inst = 0;
}

//=====================================================================================//
//                               void Bureau2::addBox()                                //
//=====================================================================================//
void Bureau2::addBox(HStoreBox2 box)
{
	assert( m_boxes.find(box->getFullId()) == m_boxes.end() );
	m_boxes.insert(std::make_pair(box->getFullId(),box));
}

//=====================================================================================//
//                           HStoreBox2 Bureau2::getBoxEx()                            //
//=====================================================================================//
HStoreBox2 Bureau2::getBoxEx(const std::string &id)
{
	std::string fullId = makeFullId(id);
	if(hasBox(id)) return getBox(fullId);

	BaseBox *box = Bureau::GetInst()->Get(id);
	if(StoreBox *sbox = box->Cast2Store())
	{
		HStoreBox2 box(new StoreBox2(fullId,sbox->GetWeight()));
		StoreBoxAcceptor acceptor(box);
		ThingScriptParser parser(&acceptor, ThingFactory::GetInst());

		int line;
		std::string script;

		if(!AIUtils::GetItemSetStr(sbox->GetScriptRID(), &script, &line))
		{
			std::ostringstream ostr;

			ostr << "GetItemSetStr: нет набора <" << sbox->GetScriptRID() << ">" << std::endl;
			ostr << "\tящик:    " << sbox->GetRID() << std::endl;
			ostr << "\tуровень: " << getLevel() << std::endl;

			MLL_MAKE_LOG("itemsets.log",ostr.str());

			assert( !"Не найден набор предметов для ящика. Подробности в itemsets.log" );
		}
		else
		{
			if(!parser.Parse(script))
			{
				std::ostringstream ostr;

				ostr << "ScriptParser: ошибка разбора в наборе <" << sbox->GetScriptRID() << ">" << std::endl;
				ostr << "\tящик:    " << sbox->GetRID() << std::endl;
				ostr << "\tуровень: " << getLevel() << std::endl;
				ostr << "\titem_sets.xls: line = " << line + 1 << " pos = " << parser.GetLastPos() << std::endl;

				MLL_MAKE_LOG("itemsets.log",ostr.str());

				assert( !"Ошибка в наборе предметов для ящика. Подробности в itemsets.log" );
			}
		}

		addBox(box);
		return box;
	}
	else
	{
		std::ostringstream ostr;

		ostr << "ScriptParser: не найден ящик <" << box->GetRID() << ">" << std::endl;
		ostr << "\tуровень: " << getLevel() << std::endl;

		MLL_MAKE_LOG("itemsets.log",ostr.str());

		assert( !"Не найден ящик. Подробности в itemsets.log" );
		return HStoreBox2();
	}
}

//=====================================================================================//
//                            HStoreBox2 Bureau2::getBox()                             //
//=====================================================================================//
HStoreBox2 Bureau2::getBox(const std::string &fullId)
{
	assert( m_boxes.find(fullId) != m_boxes.end() );
	return m_boxes.find(fullId)->second;
}

//=====================================================================================//
//                            bool Bureau2::hasBox() const                             //
//=====================================================================================//
bool Bureau2::hasBox(const std::string &id) const
{
	std::string fullId = makeFullId(id);
	return m_boxes.find(fullId) != m_boxes.end();
}

//=====================================================================================//
//                             void Bureau2::store() const                             //
//=====================================================================================//
void Bureau2::store(mll::io::obinstream &out) const
{
	out << m_boxes.size();

	for(Boxes_t::const_iterator i = m_boxes.begin(); i != m_boxes.end(); ++i)
	{
		i->second->store(out);
	}
}

//=====================================================================================//
//                               void Bureau2::restore()                               //
//=====================================================================================//
void Bureau2::restore(mll::io::ibinstream &in)
{
	assert( m_boxes.empty() );

	size_t count;
	in >> count;

	while(count--)
	{
		HStoreBox2 box(new StoreBox2);
		box->restore(in);
		addBox(box);
	}
}

//=====================================================================================//
//                              Bureau2 &Bureau2::inst()                               //
//=====================================================================================//
Bureau2 &Bureau2::inst()
{
	return *g_inst;
}

}
