#include "precomp.h"
#include "dsound.h"
#include "caps.h"

namespace Muffle
{

//=====================================================================================//
//                             DirectSound::DirectSound()                              //
//=====================================================================================//
DirectSound::DirectSound(const GUID &device, HWND wnd)
:	m_device(device)
{
	KERNEL_LOG("—оздаю IDirectSound8 объект с GUID " << device << "\n");

	HRESULT result;

	if(FAILED(result = DirectSoundCreate8(&device,&m_dsound,NULL)))
	{
		KERNEL_LOG("\t—оздание не удалось по следующей причине:\n\t"
			<< get_error_text(result) << "\n");
		std::ostringstream sstr;
		sstr << "Cannot create DirectSound8 due to following problem:\n"
			 << get_error_text(result) << "\n";
		throw mll::debug::exception(sstr.str());
	}

	KERNEL_LOG("\t—оздание удалось. ”казатель равен " << snd << "\n");

	SAFE_CALL(m_dsound->SetCooperativeLevel(wnd,DSSCL_PRIORITY));

	dumpCaps(*this);
}

//=====================================================================================//
//                             DirectSound::~DirectSound()                             //
//=====================================================================================//
DirectSound::~DirectSound()
{
	KERNEL_LOG("”ничтожаю объект DirectSound8\n");
}

}	//	namespace Muffle