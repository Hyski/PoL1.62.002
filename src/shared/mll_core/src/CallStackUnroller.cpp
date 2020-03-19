#include "precomp.h"
#include "CallStackUnroller.h"
#include "SymEngine.h"
#include <tchar.h>

namespace
{
	const char *g_currentLine = 0;
	const unsigned g_flags = GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE;
	EXCEPTION_POINTERS *g_ptrs;
}

//=====================================================================================//
//                           void CallStackUnroller::start()                           //
//=====================================================================================//
void CallStackUnroller::start(EXCEPTION_POINTERS *ptrs)
{
	SymEngine::init();
	g_ptrs = ptrs;
	g_currentLine = SymEngine::getFirstStackTraceString(g_flags,g_ptrs) ;
}

//=====================================================================================//
//                          void CallStackUnroller::finish()                           //
//=====================================================================================//
void CallStackUnroller::finish()
{
	g_currentLine = 0;
	g_ptrs = 0;
	SymEngine::shut();
}

//=====================================================================================//
//                          bool CallStackUnroller::isDone()                           //
//=====================================================================================//
bool CallStackUnroller::isDone()
{
	return g_currentLine == 0;
}

//=====================================================================================//
//                        const char *CallStackUnroller::item()                        //
//=====================================================================================//
const char *CallStackUnroller::item()
{
	return g_currentLine;
}

//=====================================================================================//
//                           void CallStackUnroller::next()                            //
//=====================================================================================//
void CallStackUnroller::next()
{
	g_currentLine = SymEngine::getNextStackTraceString(g_flags, g_ptrs);
}

//=====================================================================================//
//                 const STACKFRAME *CallStackUnroller::currentFrame()                 //
//=====================================================================================//
const STACKFRAME *CallStackUnroller::currentFrame()
{
	return SymEngine::getCurrentFrame();
}