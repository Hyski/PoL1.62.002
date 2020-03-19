#include "logicdefs.h"
#include "InfoUtils.h"

static int g_messageOldCounter = 0;
static int g_messageCounter = 0;

//=====================================================================================//
//                              void IncMessageCounter()                               //
//=====================================================================================//
void MessageCounter::IncMessageCounter()
{
	++g_messageCounter;
}

//=====================================================================================//
//                                bool HasNewMessages()                                //
//=====================================================================================//
bool MessageCounter::HasNewMessages()
{
	return g_messageOldCounter != g_messageCounter;
}

//=====================================================================================//
//                              void UpdateNewMessages()                               //
//=====================================================================================//
void MessageCounter::UpdateNewMessages()
{
	g_messageOldCounter = g_messageCounter;
}

//=====================================================================================//
//                         void MessageCounter::MakeSaveLoad()                         //
//=====================================================================================//
void MessageCounter::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{
		slot << g_messageCounter;
		slot << g_messageOldCounter;
	}
	else
	{
		slot >> g_messageCounter;
		slot >> g_messageOldCounter;
	}
}
