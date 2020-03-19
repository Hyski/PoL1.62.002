#include "precomp.h"
#include "primarybuffer.h"
#include "dsound.h"

namespace Muffle
{

//=====================================================================================//
//                           PrimaryBuffer::PrimaryBuffer()                            //
//=====================================================================================//
PrimaryBuffer::PrimaryBuffer(const DirectSound &dsound)
{
	DSBUFFERDESC desc;

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
	desc.lpwfxFormat = NULL;
	desc.dwBufferBytes = 0;
	desc.dwReserved = 0;
	desc.guid3DAlgorithm = GUID_NULL;

	HRESULT result;

	KERNEL_LOG("Создаю первичный звуковой буфер (Primary Buffer)\n");

	if(FAILED(result = dsound.dsound()->CreateSoundBuffer(&desc,&m_buffer,NULL)))
	{
		KERNEL_LOG("\tСоздание не удалось по следующей причине:\n\t"
			<< get_error_text(result));
		std::ostringstream sstr;
		sstr << "Cannot create Primary Buffer due to following problem:\n"
			 << get_error_text(result) << "\n";
		throw mll::debug::exception(sstr.str());
	}

	KERNEL_LOG("\tСоздание удалось. Указатель равен " << buf << "\n");

	WAVEFORMATEX wfmt;
	wfmt.cbSize = sizeof(wfmt);
	wfmt.wFormatTag = WAVE_FORMAT_PCM;
	wfmt.wBitsPerSample = sizeof(short)*8;
	wfmt.nSamplesPerSec = 44100;
	wfmt.nChannels = 2;
	wfmt.nBlockAlign = wfmt.nChannels * wfmt.wBitsPerSample / 8;
	wfmt.nAvgBytesPerSec = wfmt.nSamplesPerSec * wfmt.nBlockAlign;

	m_buffer->SetFormat(&wfmt);
	m_buffer->QueryInterface(IID_IDirectSound3DListener,(void **)&m_listener);
}

//=====================================================================================//
//                           PrimaryBuffer::~PrimaryBuffer()                           //
//=====================================================================================//
PrimaryBuffer::~PrimaryBuffer()
{
	KERNEL_LOG("Уничтожаю первичный звуковой буфер\n");
}

}	//	namespace Muffle