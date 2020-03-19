#include "precomp.h"
#include "VolumeHelper.h"
#include <Muffle/ISound.h>
#include <common/BinkMgr/BinkMgr.h>

namespace PoL
{

//=====================================================================================//
//                            static void setMuffleVolume()                            //
//=====================================================================================//
static void setMuffleVolume(Muffle::ISound::Channel chan, float vol)
{
	if(Muffle::ISound::isInitialized())
	{
		Muffle::ISound::instance()->setVolume(chan, vol);
	}
}

//=====================================================================================//
//                           static float getMuffleVolume()                            //
//=====================================================================================//
static float getMuffleVolume(Muffle::ISound::Channel chan)
{
	if(Muffle::ISound::isInitialized())
	{
		return Muffle::ISound::instance()->getVolume(chan);
	}

	return 1.0f;
}

//=====================================================================================//
//                        void VolumeHelper::setAmbientVolume()                        //
//=====================================================================================//
void VolumeHelper::setAmbientVolume(float vol)
{
	setMuffleVolume(Muffle::ISound::cAmbient,vol);
}

//=====================================================================================//
//                        void VolumeHelper::setSpeechVolume()                         //
//=====================================================================================//
void VolumeHelper::setSpeechVolume(float vol)
{
	setMuffleVolume(Muffle::ISound::cSpeech,vol);
}

//=====================================================================================//
//                        void VolumeHelper::setEffectsVolume()                        //
//=====================================================================================//
void VolumeHelper::setEffectsVolume(float vol)
{
	setMuffleVolume(Muffle::ISound::cEffects,vol);
	BinkMgr::Instance()->SetVolume(vol * getMuffleVolume(Muffle::ISound::cMaster));
}

//=====================================================================================//
//                        void VolumeHelper::setThemesVolume()                         //
//=====================================================================================//
void VolumeHelper::setThemesVolume(float vol)
{
	setMuffleVolume(Muffle::ISound::cThemes,vol);
}

//=====================================================================================//
//                        void VolumeHelper::setMasterVolume()                         //
//=====================================================================================//
void VolumeHelper::setMasterVolume(float vol)
{
	setMuffleVolume(Muffle::ISound::cMaster,vol);
	BinkMgr::Instance()->SetVolume(vol * getMuffleVolume(Muffle::ISound::cEffects));
}


}
