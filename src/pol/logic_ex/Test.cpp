#include "precomp.h"

#include "ActMgr.h"

#include "ActFactory.h"
#include "MovePersonAct.h"

#include "ActScriptLexer.hpp"
#include "ActScriptParser.hpp"

namespace PoL
{

namespace TestDetail
{

////=====================================================================================//
////                                     void walk()                                     //
////=====================================================================================//
//void walk(const ActScriptParser &parser, antlr::RefAST t, unsigned int depth = 0)
//{
//	if(t == antlr::nullAST) return;
//
//	for(antlr::RefAST a = t; a != antlr::nullAST; a = a->getNextSibling())
//	{
//		ACT_LOG(std::string(depth*4,' '));
//		ACT_LOG(parser.getTokenName(a->getType()) << " " << a->getText() << "\n");
//		walk(parser,a->getFirstChild(),depth+1);
//	}
//}

}

using namespace TestDetail;

//=====================================================================================//
//                                 class ActMgrTester                                  //
//=====================================================================================//
class ActMgrTester
{
public:
	ActMgrTester()
	{
		//std::ifstream s("test.act",std::ios::binary);
		//ActScriptLexer lexer(s);
		//ActScriptParser parser(lexer);
		//antlr::ASTFactory astfactory;

		//parser.initializeASTFactory(astfactory);
		//parser.setASTFactory(&astfactory);

		//ACT_LOG("Act script log started\n");
		//parser.program();

		//antlr::RefAST ast = parser.getAST();
		//walk(parser,ast);
	}
};

ActMgrTester tester;

//=====================================================================================//
//                                 void ActMgr::test()                                 //
//=====================================================================================//
void ActMgr::test()
{
	test("quest_test");
	//HAct act = ActFactory::instance()->create("quest_test");
	//ActMgr::start(act);
	//while(ActMgr::isPlayingScene()) ActMgr::play();

	//ActMgr::store(*(std::ostream*)0);
	//ActFactory::instance();
}

//=====================================================================================//
//                                 void ActMgr::test()                                 //
//=====================================================================================//
void ActMgr::test(const std::string &name)
{
	HAct act = ActFactory::instance()->create(name);
	if(act)
	{
		ActMgr::start(act);
		//while(ActMgr::isPlayingScene()) ActMgr::play();
	}

	//ActMgr::store(*(std::ostream*)0);
	//ActFactory::instance();
}

}