#include "precomp.h"

#include "Act.h"
#include "ActTemplate.h"
#include "QuestReportAct.h"

namespace PoL
{

//=====================================================================================//
//                ActTemplate::Iterator_t ActTemplate::addActCreator()                 //
//=====================================================================================//
ActTemplate::Iterator_t ActTemplate::addActCreator(const ActCreator_t &c, const Iterator_t &it)
{
	return m_creators.insert_son(it,c);
}

//=====================================================================================//
//                            HAct ActTemplate::createAct()                            //
//=====================================================================================//
HAct ActTemplate::createAct()
{
	if(m_creators.empty()) return 0;
	return new QuestReportAct(recursiveCreateAct(m_creators.begin()));
}

//=====================================================================================//
//                       HAct ActTemplate::recursiveCreateAct()                        //
//=====================================================================================//
HAct ActTemplate::recursiveCreateAct(const Iterator_t &it)
{
	Acts_t acts;

	for(Iterator_t son = it.son_begin(); son != it.son_end(); son.brother())
	{
		acts.push_back(recursiveCreateAct(son));
	}

	return (*it)(acts);
}

}