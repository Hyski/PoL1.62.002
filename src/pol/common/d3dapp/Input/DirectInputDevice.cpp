#include "Precomp.h"

#include "WorkingThread.h"
#include "DirectInputDevice.h"

//=====================================================================================//
//                    static PoL::WorkingThread &getWorkingThread()                    //
//=====================================================================================//
static PoL::WorkingThread &getWorkingThread()
{
	static PoL::WorkingThread inst;
	return inst;
}

//=====================================================================================//
//                      CDirectInputDevice::CDirectInputDevice()                       //
//=====================================================================================//
CDirectInputDevice::CDirectInputDevice()
{
}

//=====================================================================================//
//                      CDirectInputDevice::~CDirectInputDevice()                      //
//=====================================================================================//
CDirectInputDevice::~CDirectInputDevice()
{
	Release();
}

//=====================================================================================//
//                         void CDirectInputDevice::Release()                          //
//=====================================================================================//
void CDirectInputDevice::Release(void)
{
	if(m_lpDIDevice)
	{
		m_lpDIDevice->Unacquire();
		m_lpDIDevice->Release();
		m_lpDIDevice = NULL;
	}

	if(m_hDeviceEvent)
	{
		CloseHandle(m_hDeviceEvent);
	}
}

//=====================================================================================//
//                           void CDirectInputDevice::Lock()                           //
//=====================================================================================//
void CDirectInputDevice::Lock(void)
{
	if(m_lpDIDevice) m_lpDIDevice->Acquire();
}

//=====================================================================================//
//                          void CDirectInputDevice::Unlock()                          //
//=====================================================================================//
void CDirectInputDevice::Unlock(void)
{
	if(m_lpDIDevice) m_lpDIDevice->Unacquire();
}

//=====================================================================================//
//                           void CDirectInputDevice::Run()                            //
//=====================================================================================//
void CDirectInputDevice::Run()
{
	getWorkingThread().addWorker(m_hDeviceEvent,this);
}

//=====================================================================================//
//                           void CDirectInputDevice::Stop()                           //
//=====================================================================================//
void CDirectInputDevice::Stop()
{
	getWorkingThread().removeWorker(m_hDeviceEvent);
}

//=====================================================================================//
//                       void CDirectInputDevice::ProcessEvent()                       //
//=====================================================================================//
void CDirectInputDevice::ProcessEvent(void)
{
}

//=====================================================================================//
//                       unsigned int CDirectInputDevice::Main()                       //
//=====================================================================================//
//unsigned int CDirectInputDevice::Main(void)
//{
////	console("- NhtDIDeviceThread created;\n");
//	while(CanRun())
//	{
//		if(WaitForSingleObject(m_hDeviceEvent,1000) == WAIT_OBJECT_0)
//		{
//			ProcessEvent();
//		}
//	}
////	console("- NhtDIDeviceThread destroyed;\n");
//
//	return 0;
//}

//=====================================================================================//
//                           void CDirectInputDevice::work()                           //
//=====================================================================================//
void CDirectInputDevice::work()
{
	ProcessEvent();
}

//=====================================================================================//
//                         bool CDirectInputDevice::IsLocked()                         //
//=====================================================================================//
bool CDirectInputDevice::IsLocked(void)
{
	return false;
}
