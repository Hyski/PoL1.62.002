#if !defined(__DSOUND_H_INCLUDED_5698059465543420__)
#define __DSOUND_H_INCLUDED_5698059465543420__

namespace Muffle
{

//=====================================================================================//
//                                  class DirectSound                                  //
//=====================================================================================//
class DirectSound
{
	GUID m_device;
	ATL::CComPtr<IDirectSound8> m_dsound;

public:
	DirectSound(const GUID &device, HWND);
	~DirectSound();

	IDirectSound8 *dsound() const { return m_dsound; }
};

}	//	namespace Muffle

#endif // !defined(__DSOUND_H_INCLUDED_5698059465543420__)