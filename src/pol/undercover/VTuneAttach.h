#pragma once

namespace PoL
{

#if defined(_HOME_VERSION)

//=====================================================================================//
//                                  class VTuneAttach                                  //
//=====================================================================================//
class VTuneAttach
{
	typedef void __cdecl VTPause_t(void);
	typedef void __cdecl VTResume_t(void);

	HMODULE m_dll;
	VTPause_t *m_pause;
	VTResume_t *m_resume;

public:
	VTuneAttach()
	:	m_dll(0), m_pause(0), m_resume(0)
	{
		m_dll = LoadLibrary("VTuneApi.dll");
		m_pause = (VTPause_t*)GetProcAddress(m_dll,"VTPause");
		m_resume = (VTResume_t*)GetProcAddress(m_dll,"VTResume");
	}

	void pause()
	{
		if(m_pause) (*m_pause)();
	}

	void resume()
	{
		if(m_resume) (*m_resume)();
	}
};

static VTuneAttach g_vtune;

#endif
}
