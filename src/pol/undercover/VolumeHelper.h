#pragma once

namespace PoL
{

//=====================================================================================//
//                                 class VolumeHelper                                  //
//=====================================================================================//
class VolumeHelper
{
public:
	/// ���������� ���������
	static void setSpeechVolume(float);
	static void setAmbientVolume(float);
	static void setEffectsVolume(float);
	static void setMasterVolume(float);
	static void setThemesVolume(float);
};

}
