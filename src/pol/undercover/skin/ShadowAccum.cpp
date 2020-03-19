#include "precomp.h"

#include "Shadow.h"
#include "ShadowAccum.h"
#include "ComplexShadow.h"

namespace Shadows
{

namespace ShadowsDetails
{

typedef WINBASEAPI BOOL (__stdcall * pQueueUserWorkItem_t)(LPTHREAD_START_ROUTINE, PVOID, ULONG);
pQueueUserWorkItem_t pQueueUserWorkItem = 0;

//=====================================================================================//
//                                 class QueueUserLoad                                 //
//=====================================================================================//
class QueueUserLoad
{
	HMODULE m_kern;

public:
	QueueUserLoad()
	{
		m_kern = LoadLibrary("kernel32.dll");
		pQueueUserWorkItem = (pQueueUserWorkItem_t)GetProcAddress(m_kern,"QueueUserWorkItem");
	}

	~QueueUserLoad()
	{
		FreeLibrary(m_kern);
	}
};

static QueueUserLoad qul;

}

using namespace ShadowsDetails;

extern bool g_useHT;

//=====================================================================================//
//                            namespace ShadowAccumDetails                             //
//=====================================================================================//
namespace ShadowAccumDetails
{

HANDLE g_saEvent = NULL;

}

using namespace ShadowAccumDetails;

//=====================================================================================//
//                          struct ShadowAccum::UpdateParams                           //
//=====================================================================================//
struct ShadowAccum::UpdateParams
{
	ShadowAccum *m_accum;
	ComplexShadows_t *m_shadows;
	float m_tau;
};

//=====================================================================================//
//                             ShadowAccum::ShadowAccum()                              //
//=====================================================================================//
ShadowAccum::ShadowAccum()
{
	g_saEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}

//=====================================================================================//
//                             ShadowAccum::~ShadowAccum()                             //
//=====================================================================================//
ShadowAccum::~ShadowAccum()
{
	CloseHandle(g_saEvent);
}

//=====================================================================================//
//                          ShadowAccum *ShadowAccum::inst()                           //
//=====================================================================================//
ShadowAccum *ShadowAccum::inst()
{
	static ShadowAccum instance;
	return &instance;
}

//=====================================================================================//
//                                  static bool has()                                  //
//=====================================================================================//
template<typename T, typename R>
static bool has(const T &t, const R &r)
{
	return std::find(t.begin(),t.end(),r) != t.end();
}

//=====================================================================================//
//                            void ShadowAccum::addShadow()                            //
//=====================================================================================//
void ShadowAccum::addShadow(BaseShadow *shdw)
{
	if(shdw->IsComplex())
	{
		if(!has(m_complexShadows,shdw))
		{
			m_complexShadows.push_back(static_cast<ComplexShadow*>(shdw));
		}
	}
	else
	{
		if(!has(m_nonComplexShadows,shdw))
		{
			m_nonComplexShadows.push_back(shdw);
		}
	}
}

//=====================================================================================//
//                         void ShadowAccum::updateFromList()                          //
//=====================================================================================//
void ShadowAccum::updateFromList(float tau, ComplexShadows_t *cs)
{
	for(ComplexShadows_t::iterator i = cs->begin(); i != cs->end(); ++i)
	{
		(*i)->Update(tau);
	}
}

//=====================================================================================//
//                         DWORD WINAPI ShadowAccum::worker()                          //
//=====================================================================================//
DWORD WINAPI ShadowAccum::worker(LPVOID p)
{
	UpdateParams *up = reinterpret_cast<UpdateParams*>(p);
	up->m_accum->updateFromList(up->m_tau,up->m_shadows);
	SetEvent(g_saEvent);
	return 0;
}

//=====================================================================================//
//                          void ShadowAccum::updateShadows()                          //
//=====================================================================================//
void ShadowAccum::updateShadows(float tau)
{
	if(g_useHT && pQueueUserWorkItem)
	{
		updateShadowsHT(tau);
	}
	else
	{
		updateShadowsNoHT(tau);
	}

	m_complexShadows.clear();
	m_nonComplexShadows.clear();
}

//=====================================================================================//
//                         void ShadowAccum::updateShadowsHT()                         //
//=====================================================================================//
void ShadowAccum::updateShadowsHT(float tau)
{
	for(ComplexShadows_t::iterator i = m_complexShadows.begin(); i != m_complexShadows.end(); ++i)
	{
		(*i)->LockSurface();
	}

    for(Shadows_t::iterator i = m_nonComplexShadows.begin(); i != m_nonComplexShadows.end(); ++i)
	{
		(*i)->Update(tau);
	}

	int countSecondary = m_complexShadows.size()/2;
	int countPrimary = m_complexShadows.size() - countSecondary;

	if(countSecondary)
	{
		m_mainThreadShadows.assign(m_complexShadows.begin(),m_complexShadows.begin()+countPrimary);
		m_secondaryThreadShadows.assign(m_complexShadows.begin()+countPrimary,m_complexShadows.end());

		UpdateParams up = {this, &m_secondaryThreadShadows, tau};
		pQueueUserWorkItem(worker,reinterpret_cast<LPVOID>(&up),0);

		updateFromList(tau,&m_mainThreadShadows);

		WaitForSingleObject(g_saEvent,INFINITE);
	}
	else
	{
		updateFromList(tau,&m_complexShadows);
	}

	for(ComplexShadows_t::iterator i = m_complexShadows.begin(); i != m_complexShadows.end(); ++i)
	{
		(*i)->UnlockSurface();
	}
}

//=====================================================================================//
//                        void ShadowAccum::updateShadowsNoHT()                        //
//=====================================================================================//
void ShadowAccum::updateShadowsNoHT(float tau)
{
	for(ComplexShadows_t::iterator i = m_complexShadows.begin(); i != m_complexShadows.end(); ++i)
	{
		(*i)->LockSurface();
	}

    for(Shadows_t::iterator i = m_nonComplexShadows.begin(); i != m_nonComplexShadows.end(); ++i)
	{
		(*i)->Update(tau);
	}

	updateFromList(tau,&m_complexShadows);

	for(ComplexShadows_t::iterator i = m_complexShadows.begin(); i != m_complexShadows.end(); ++i)
	{
		(*i)->UnlockSurface();
	}
}

}
