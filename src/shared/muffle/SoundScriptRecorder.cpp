#include "precomp.h"
#include "SoundScriptRecorder.h"
#include "SoundScriptLexer.hpp"
#include "scriptmgr.h"
#include "Settings2.h"

namespace Muffle
{

//=====================================================================================//
//                     SoundScriptRecorder::SoundScriptRecorder()                      //
//=====================================================================================//
SoundScriptRecorder::SoundScriptRecorder(SoundScriptLexer &lexer, ScriptMgr *mgr)
:	SoundScriptParser(lexer),
	m_mgr(mgr)
{
}

//=====================================================================================//
//                          void SoundScriptRecorder::parse()                          //
//=====================================================================================//
void SoundScriptRecorder::parse()
{
	program();
}

//=====================================================================================//
//                     Script *SoundScriptRecorder::createScript()                     //
//=====================================================================================//
const Muffle2::Script *SoundScriptRecorder::createScript(const ScriptInfo &info)
{
	return m_mgr->embedScript(info);
}

//=====================================================================================//
//                    void SoundScriptRecorder::initDefaultParams()                    //
//=====================================================================================//
void SoundScriptRecorder::initDefaultParams(ScriptInfo &info)
{
	switch(info.channel)
	{
		case ISound::cThemes:
			info.is3d = false;
			info.looped = true;
			info.fadeTime = Muffle2::Settings::ThemeFadeTime * 1e-3f;
			break;
		case ISound::cSpeech:
			info.is3d = false;
			info.looped = false;
			info.fadeTime = Muffle2::Settings::SoundFadeTime * 1e-3f;
			break;
		case ISound::cEffects:
			info.is3d = true;
			info.looped = false;
			info.fadeTime = Muffle2::Settings::SoundFadeTime * 1e-3f;
			break;
		case ISound::cMenu:
			info.is3d = true;
			info.looped = false;
			info.fadeTime = Muffle2::Settings::SoundFadeTime * 1e-3f;
			break;
		case ISound::cAmbient:
			info.is3d = true;
			info.looped = true;
			info.fadeTime = Muffle2::Settings::SoundFadeTime * 1e-3f;
			break;
		case ISound::cDebug:
			info.is3d = true;
			info.looped = false;
			info.fadeTime = Muffle2::Settings::SoundFadeTime * 1e-3f;
			break;
	}
}

}	//	namespace Muffle
