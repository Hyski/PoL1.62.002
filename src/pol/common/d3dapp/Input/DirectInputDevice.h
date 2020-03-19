/*------------------------------------------------------*/
//     Class Name: CDirectInputDevice
//     Coder: Naughty
//     Date: 21.03.2000 - ..
//     Company: Naughty
/*------------------------------------------------------*/
#ifndef _DIRECT_INPUT_DEVICE_H_
#define _DIRECT_INPUT_DEVICE_H_

//#include "NhtThread.h"
#include "Worker.h"

//=====================================================================================//
//                              class CDirectInputDevice                               //
//=====================================================================================//
class CDirectInputDevice : PoL::Worker
	// : public CNhtThread
{
public:
	enum {BUFFER_SIZE=16};

	LPDIRECTINPUTDEVICE m_lpDIDevice;
	HANDLE	m_hDeviceEvent;

public:
	CDirectInputDevice();
	virtual ~CDirectInputDevice();

public:
	//unsigned int Main(void);
	virtual void ProcessEvent(void);

	void Run();
	void Stop();

public:
	void Release(void);
	virtual void Lock(void);
	virtual void Unlock(void);
	//	доступно ли устройство
	virtual bool IsLocked(void);

private:
	void work();
};

#endif