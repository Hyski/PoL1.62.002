#if !defined(__PRIMARYBUFFER_H_INCLUDED_3051904287262003__)
#define __PRIMARYBUFFER_H_INCLUDED_3051904287262003__

namespace Muffle
{

//=====================================================================================//
//                                 class PrimaryBuffer                                 //
//=====================================================================================//
class PrimaryBuffer : private noncopyable
{
	ATL::CComPtr<IDirectSoundBuffer> m_buffer;
	ATL::CComPtr<IDirectSound3DListener> m_listener;

public:
	PrimaryBuffer(const class DirectSound &);
	~PrimaryBuffer();

	IDirectSoundBuffer *getBuffer() const { return m_buffer; }
	IDirectSound3DListener *getListener() const { return m_listener; }
};

}	//	namespace Muffle

#endif // !defined(__PRIMARYBUFFER_H_INCLUDED_3051904287262003__)