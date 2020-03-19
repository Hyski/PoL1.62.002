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
	KERNEL_LOG("������ IDirectSound8 ������ � GUID " << device << "\n");

	HRESULT result;

	if(FAILED(result = DirectSoundCreate8(&device,&m_dsound,NULL)))
	{
		KERNEL_LOG("\t�������� �� ������� �� ��������� �������:\n\t"
			<< get_error_text(result) << "\n");
		std::ostringstream sstr;
		sstr << "Cannot create DirectSound8 due to following problem:\n"
			 << get_error_text(result) << "\n";
		throw mll::debug::exception(sstr.str());
	}

	KERNEL_LOG("\t�������� �������. ��������� ����� " << snd << "\n");

	SAFE_CALL(m_dsound->SetCooperativeLevel(wnd,DSSCL_PRIORITY));

	dumpCaps(*this);
}

//=====================================================================================//
//                             DirectSound::~DirectSound()                             //
//=====================================================================================//
DirectSound::~DirectSound()
{
	KERNEL_LOG("��������� ������ DirectSound8\n");
}

}	//	namespace Muffle