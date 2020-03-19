#include "Vis.h"

namespace Vis
{

//=====================================================================================//
//                           HOption Option::restoreOption()                           //
//=====================================================================================//
HOption Option::restoreOption(mll::io::ibinstream &in)
{
	unsigned int type;
	in >> type;

	Option *opt = 0;

	if(type != 0xFFFFFFFF)
	{
		switch(type)
		{
#define VIS_MAKE_SWITCH(A) case ot##A: opt = new Option##A; opt->restore(in); break;
		VIS_FOREACH_PARAMETER_TYPE(VIS_MAKE_SWITCH)
#undef VIS_MAKE_SWITCH
		}
	}

	return opt;
}

//=====================================================================================//
//                             void Option::storeOption()                              //
//=====================================================================================//
void Option::storeOption(HOption option, mll::io::obinstream &out)
{
	if(option.get())
	{
		out << option->getType();
		option->store(out);
	}
	else
	{
		out << 0xFFFFFFFFu;
	}
}

}