#if !defined(__SOUND_SCRIPT_RECORDER_H_INCLUDED_1104954923325159__)
#define __SOUND_SCRIPT_RECORDER_H_INCLUDED_1104954923325159__

#include "Script2.h"
#include "SoundScriptParser.hpp"

namespace Muffle
{

class ScriptMgr;

//=====================================================================================//
//                class SoundScriptRecorder : public SoundScriptParser                 //
//=====================================================================================//
class SoundScriptRecorder : private SoundScriptParser
{
	ScriptMgr *m_mgr;

private:
	virtual const Muffle2::Script *createScript(const ScriptInfo &);
	virtual void initDefaultParams(ScriptInfo &);

public:
	SoundScriptRecorder(class SoundScriptLexer &, ScriptMgr *);
	void parse();
};

}	//	namespace Muffle

#endif // !defined(__SOUND_SCRIPT_RECORDER_H_INCLUDED_1104954923325159__)