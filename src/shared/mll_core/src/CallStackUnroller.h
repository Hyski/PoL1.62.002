#if !defined(__CALL_STACK_UNROLLER_H_INCLUDED_8093570542662288__)
#define __CALL_STACK_UNROLLER_H_INCLUDED_8093570542662288__

#include <dbghelp.h>

//=====================================================================================//
//                               class CallStackUnroller                               //
//=====================================================================================//
class CallStackUnroller
{
public:
	static void start(EXCEPTION_POINTERS *ptrs);
	static void finish();

	static bool isDone();
	static const char *item();
	static const STACKFRAME *currentFrame();
	static void next();
};

#endif // !defined(__CALL_STACK_UNROLLER_H_INCLUDED_8093570542662288__)