#include "precomp.h"
#include "ProgrBar.h"
#include "../Interface/Interface.h"

//=====================================================================================//
//                              void ProgrBar::SetTitle()                              //
//=====================================================================================//
void ProgrBar::SetTitle(const std::string &form, const char *n)
{
	char buff[1024];
	sprintf(buff, form.c_str(), n);
	Interface::Instance()->ProBar()->SetTitle(buff);
}

//=====================================================================================//
//                              void ProgrBar::SetTitle()                              //
//=====================================================================================//
void ProgrBar::SetTitle(const std::string &form, const std::string &n)
{
	char buff[1024];
	sprintf(buff, form.c_str(), n.c_str());
	Interface::Instance()->ProBar()->SetTitle(buff);
}

//=====================================================================================//
//                              void ProgrBar::SetMofN()                               //
//=====================================================================================//
void ProgrBar::SetMofN(int m,int n)
{
	Interface::Instance()->ProBar()->SetMofN(m,n);
}

//=====================================================================================//
//                             void ProgrBar::SetPercent()                             //
//=====================================================================================//
void ProgrBar::SetPercent(float p)
{
	Interface::Instance()->ProBar()->SetPercent(p);
}

//=====================================================================================//
//                              void ProgrBar::SetRange()                              //
//=====================================================================================//
void ProgrBar::SetRange(float a,float b)
{
	Interface::Instance()->ProBar()->SetRange(a,b);
}
