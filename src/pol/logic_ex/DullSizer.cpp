#include "precomp.h"

#include "DullSizer.h"
#include "Persistent.h"

namespace PoL
{

//=====================================================================================//
//                               DullSizer::DullSizer()                                //
//=====================================================================================//
DullSizer::DullSizer(unsigned int ver)
{
	std::string fname = (ver == 14 ? "compat/patch1beta.types" : "compat/release.types");

	VFileWpr fl(fname);
	VFileBuf buf(fl.get());
	std::istream in(&buf);
	mll::io::ibinstream bin(in);

	size_t count;
	bin >> asInt(count);

	for(size_t i = 0; i < count; ++i)
	{
		std::string name;
		SizerTypes type;

		bin >> name;
		bin >> asInt(type);

		m_sizeMap.insert(std::make_pair(name,type));
	}

	bin >> asInt(count);

	for(size_t i = 0; i < count; ++i)
	{
		std::string name;
		int cnt;

		bin >> name;
		bin >> asInt(cnt);

		m_taskListMap.insert(std::make_pair(name,cnt));
	}
}

//=====================================================================================//
//                             void DullSizer::skipEntry()                             //
//=====================================================================================//
void DullSizer::skipEntry(mll::io::ibinstream &in, const std::string &name)
{
	SizerTypes type = getEntryType(in,name);

	int t, i, t2;
	std::string s;

	switch(type)
	{
		case stQuest:
			in >> asInt(t) >> asInt(t);
			in >> asInt(t) >> asInt(t) >> asInt(t);
			in >> asInt(t);
			break;
		case stBringTask:
			in >> asInt(t) >> asInt(t) >> asInt(t);		// Task
			break;
		case stEliminateTask:
			in >> asInt(t) >> asInt(t) >> asInt(t);		// Task
			in >> t >> asInt(t);
			for(i = 0; i < t; ++i)
			{
				in >> s >> asInt(t2);
			}
			break;
		case stKillTask:
			in >> asInt(t) >> asInt(t) >> asInt(t);		// Task
			break;
		case stStubTask:
			in >> asInt(t) >> asInt(t) >> asInt(t);		// Task
			break;
		case stTalkTask:
			in >> asInt(t) >> asInt(t) >> asInt(t);		// Task
			break;
		case stTaskList:
			in >> asInt(t) >> asInt(t) >> asInt(t);		// Task
			t = getTaskListCount(name);
			for(i = 0; i < t; ++i)
			{
				in >> asInt(t2);
			}
			in >> asInt(t) >> asInt(t) >> asInt(t);
			break;
		case stAliveCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stAndCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stCompareCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stContextCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stEpisodeCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stHeroCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stOrCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stQuestCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stTaskCond:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stLeaveLevelSignal:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stSmbUseSignal:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stTaskDbSignal:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stBringSignal:
			in >> asInt(t) >> asInt(t) >> asInt(t) >> asInt(t); // Condition
			break;
		case stRefCounter:
			break;
		case stUsualCounter:
			in >> t;
			break;
		case stTrigger:
			in >> asInt(t);
			break;
	}
}

}
