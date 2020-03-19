#include "precomp.h"
#include "scriptmgr.h"
#include "SoundScriptLexer.hpp"
#include "SoundScriptRecorder.h"

namespace Muffle
{

//=====================================================================================//
//                               ScriptMgr::ScriptMgr()                                //
//=====================================================================================//
ScriptMgr::ScriptMgr(SndServices* services)
{
	ci_VFileSystem* vfs = services->fs();
	ci_VDirIt *it = vfs->getDir(services->getScriptBasePath()+"*.seng");

	for(; !it->isDone(); it->next())
	{
		const std::string &name = it->getFullName();
		ci_VFile *file = vfs->openFile(name.c_str());
		std::istrstream in((char *)file->data(),file->size());
		SoundScriptLexer lexer(in);
		SoundScriptRecorder parser(lexer,this);

		try
		{
			parser.parse();
		}
		catch(const antlr::ANTLRException &)
		{
			//MessageBox(NULL,e.toString(),"ScriptMgr",MB_OK);
		}
		file->release();
	}

	it->release();

	dump();
}

//=====================================================================================//
//                   const Muffle2::Script *ScriptMgr::embedScript()                   //
//=====================================================================================//
const Muffle2::Script *ScriptMgr::embedScript(const ScriptInfo &info_)
{
	ScriptInfo info(info_);

	if(info.name.empty())
	{
		std::ostringstream sstr;
		static int counter = 0;
		sstr << "un#" << counter++;

		info.name = sstr.str();
	}

	return &m_scripts.insert(std::make_pair(info.name,Muffle2::Script(info))).first->second;
}

//=====================================================================================//
//                              Script *ScriptMgr::get()                               //
//=====================================================================================//
const Muffle2::Script* ScriptMgr::get(const std::string &name) const
{
	Scripts_t::const_iterator iscript = m_scripts.find(name);
	if(iscript != m_scripts.end()) return &iscript->second;
	return 0;
}

//=====================================================================================//
//                               void ScriptMgr::dump()                                //
//=====================================================================================//
void ScriptMgr::dump() const
{
	for(Scripts_t::const_iterator i = m_scripts.begin(); i != m_scripts.end(); ++i)
	{
		KERNEL_LOG(i->second);
	}
}

}	//	namespace Muffle