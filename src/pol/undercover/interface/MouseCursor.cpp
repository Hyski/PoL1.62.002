#include "Precomp.h"
#include "MenuMgr.h"
#include "MouseCursor.h"

void MouseCursor::SetCursor(CURSOR_TYPE ctype,
							const TextField *t1,
							const TextField *t2,
							const TextField *t3,
							const TextField *t4)
{
	static MenuMgr::ic_MouseCursor::TextField hard_belt[4];
	MenuMgr::ic_MouseCursor::TextField* belt[4] = {0,0,0,0};
	const TextField* local_belt[4] = {t1,t2,t3,t4};

	for(int i=0; i<4; i++)
	{
		if(local_belt[i])
		{
			hard_belt[i].m_Text = local_belt[i]->m_Text;
			hard_belt[i].m_Color = local_belt[i]->m_Color;
			belt[i] = &hard_belt[i];
		}
	}

	MenuMgr::MouseCursor()->SetCursor((MenuMgr::ic_MouseCursor::CURSOR_TYPE)ctype,
									  belt[0],belt[1],belt[2],belt[3]);
}

void MouseCursor::SetVisible(bool visible)
{
	MenuMgr::MouseCursor()->SetVisible(visible);
}
