/***********************************************************************

                               Alfa Project

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   10.12.2001

************************************************************************/
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <mll/debug/exception.h>

MLL_MAKE_EXCEPTION(CasusImprovisus,mll::debug::exception);

class NoBreak {};

/////////////////////////////////////////////////////////////////////////
//////////////////////    class CasusImprovisus    //////////////////////
/////////////////////////////////////////////////////////////////////////
//class CasusImprovisus
//{
//public:
//	CasusImprovisus();
//	CasusImprovisus(const std::string msg);
//	CasusImprovisus(const std::string file,unsigned int line,const std::string msg);
//	CasusImprovisus(const NoBreak&);
//	CasusImprovisus(const std::string msg,const NoBreak&);
//	CasusImprovisus(const std::string file,unsigned int line,const std::string msg,const NoBreak&);
//	~CasusImprovisus();
//public:
//	//	�������� ��������� ���������� ����������
//	virtual const char* Content(void);
//	//	��������� ����� ��������
//	static void DbgBreak(void);
//};

typedef CasusImprovisus CASUS;

///////////////////////////////////////////////////////////////////////////
//////////////////////////    class CommonCasus    ////////////////////////
///////////////////////////////////////////////////////////////////////////
//class CommonCasus : public CasusImprovisus
//{
//private:
//	std::string m_Content;
//public:
//	CommonCasus(const std::string file,unsigned int line,const std::string msg);
//	const char* Content(void);
//};

//#define CASUS(msg) CommonCasus(msg)

#endif	//_EXCEPTION_H_