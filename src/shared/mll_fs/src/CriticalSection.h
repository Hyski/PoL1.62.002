#if !defined(__CRITICAL_SECTION_H_INCLUDED_1235328334847649__)
#define __CRITICAL_SECTION_H_INCLUDED_1235328334847649__


#include <windows.h>

namespace mll
{

namespace fs
{

/******************************************************************************
Module name: Optex.h
Written by:  Jeffrey Richter
******************************************************************************/

//=====================================================================================//
//                                    class COptex                                     //
//=====================================================================================//
class COptex
{
public:
   COptex(DWORD dwSpinCount = 4000);
   COptex(PCSTR  pszName, DWORD dwSpinCount = 4000);
   COptex(PCWSTR pszName, DWORD dwSpinCount = 4000);
   ~COptex();

   void SetSpinCount(DWORD dwSpinCount);
   void Enter();
   BOOL TryEnter();
   void Leave();
   BOOL IsSingleProcessOptex() const;

private:
   typedef struct {
      DWORD m_dwSpinCount;
      long  m_lLockCount;
      DWORD m_dwThreadId;
      long  m_lRecurseCount;
   } SHAREDINFO, *PSHAREDINFO;

   HANDLE      m_hevt;
   HANDLE      m_hfm;
   PSHAREDINFO m_psi;

private:
   static BOOL sm_fUniprocessorHost;

private:
   void CommonConstructor(DWORD dwSpinCount, BOOL fUnicode, PVOID pszName);
   PSTR ConstructObjectName(PSTR pszResult, PCSTR pszPrefix,
							BOOL fUnicode, PVOID pszName);
};

//=====================================================================================//
//                               inline COptex::COptex()                               //
//=====================================================================================//
inline COptex::COptex(DWORD dwSpinCount)
{
   CommonConstructor(dwSpinCount, FALSE, NULL);
}

//=====================================================================================//
//                               inline COptex::COptex()                               //
//=====================================================================================//
inline COptex::COptex(PCSTR pszName, DWORD dwSpinCount)
{
   CommonConstructor(dwSpinCount, FALSE, (PVOID) pszName);
}

//=====================================================================================//
//                               inline COptex::COptex()                               //
//=====================================================================================//
inline COptex::COptex(PCWSTR pszName, DWORD dwSpinCount)
{
   CommonConstructor(dwSpinCount, TRUE, (PVOID) pszName);
}

//=====================================================================================//
//                  inline BOOL COptex::IsSingleProcessOptex() const                   //
//=====================================================================================//
inline BOOL COptex::IsSingleProcessOptex() const
{
   return(m_hfm == NULL);
}

//=====================================================================================//
//                               class critical_section                                //
//=====================================================================================//
class critical_section
{
	COptex m_optex;

public:
	critical_section();
	~critical_section();

	void enter();
	void leave();
};

}

}

#endif // !defined(__CRITICAL_SECTION_H_INCLUDED_1235328334847649__)