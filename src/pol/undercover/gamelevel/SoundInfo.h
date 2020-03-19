#pragma once

//=====================================================================================//
//                                  struct SoundInfo                                   //
//=====================================================================================//
struct SoundInfo
{
	enum State
	{
		stateAmbient,
		stateOpening,
		stateClosing,
		stateOpened,
		stateClosed,
		statesCount
	};

	std::string m_sounds[statesCount];

	SoundInfo()
	{
		std::fill(m_sounds,m_sounds+statesCount,"none");
	}
};