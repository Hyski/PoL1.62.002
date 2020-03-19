#include "precomp.h"

#include "SavedData.h"
#include <undercover/IWorld.h>
#include <undercover/skin/skanim.h>
#include <undercover/interface/screens.h>
#include <undercover/gamelevel/gamelevel.h>
#include <undercover/interface/gamescreen.h>
#include <undercover/interface/dialogscreen.h>
#include <undercover/interface/loadingscreen.h>

extern float SpeedMul;

namespace PoL
{

static int g_counter = 0;

namespace SavedDataDetails
{

//=====================================================================================//
//                                class SavedDataState                                 //
//=====================================================================================//
class SavedDataState
{
	typedef std::vector<std::string> Widgets_t;
	typedef std::vector<bool> Flags_t;

	float m_animSpeed1, m_animSpeed2;
	//bool m_roofVisible;

	bool m_gameScreenVisible;
	bool m_hidMoveVisible;
	bool m_gameScreenEnabled;
	Widgets_t m_widgets;
	Flags_t m_enabled, m_visible;

public:
	void enter()
	{
		m_animSpeed1 = SpeedMul;
		m_animSpeed2 = SkAnim::SkSpeed;
		m_gameScreenVisible = Screens::Instance()->Game()->IsVisible();
		m_hidMoveVisible = Screens::Instance()->HiddenMovement()->IsVisible();
		m_gameScreenEnabled = Screens::Instance()->Game()->IsEnable();

		SpeedMul = 2.0f;
		SkAnim::SkSpeed = 2.0f;
		//IWorld::Get()->GetLevel()->SetRoofVisible(true);

		for(Widget::Iterator i = Screens::Instance()->Game()->Begin(); i != Screens::Instance()->Game()->End(); ++i)
		{
			if(&*i != Screens::Instance()->Dialog())
			{
				m_widgets.push_back(i->Name());
				//m_widgets.push_back(&*i);
				m_enabled.push_back(i->IsEnable());
				m_visible.push_back(i->IsVisible());
				i->Enable(false);
				i->SetVisible(false);
			}
		}

		Screens::Instance()->Game()->Enable(true);
		Screens::Instance()->Game()->SetVisible(true);
		Screens::Instance()->Game()->OpenScriptSceneScreen();
		Screens::Instance()->HiddenMovement()->SetVisible(false);
        DirtyLinks::EnableLevelRender(true);
        DirtyLinks::EnableLevelAndEffectSound(true);

		GraphEntity::RaiseFlags(GraphEntity::FT_ALWAYS_VISIBLE);
		GraphEntity::ForceUpdate();
	}

	void leave()
	{
		GraphEntity::DropFlags(GraphEntity::FT_ALWAYS_VISIBLE);
		GraphEntity::ForceUpdate();

		////	выключим крыши если надо
		//IWorld::Get()->GetLevel()->SetRoofVisible(m_roofVisible);

		//	отключим интерфейс
		Screens::Instance()->Game()->Enable(m_gameScreenEnabled);
		Screens::Instance()->Game()->CloseScriptSceneScreen();
		Screens::Instance()->Game()->SetVisible(m_gameScreenVisible);
        DirtyLinks::EnableLevelRender(!m_hidMoveVisible);
        DirtyLinks::EnableLevelAndEffectSound(!m_hidMoveVisible);
		Screens::Instance()->HiddenMovement()->SetVisible(m_hidMoveVisible);

		for(size_t i = 0; i < m_widgets.size(); ++i)
		{
			Widget *w = Screens::Instance()->Game()->Child(m_widgets[i].c_str());
			if(w)
			{
				w->Enable(m_enabled[i]);
				w->SetVisible(m_visible[i]);
			}
		}

		// установим скорость анимации, которую хотел пользователь
		SpeedMul = m_animSpeed1;
		SkAnim::SkSpeed = m_animSpeed2;
	}
};

}

using namespace SavedDataDetails;

static SavedDataState g_sds;

//=====================================================================================//
//                               SavedData::SavedData()                                //
//=====================================================================================//
SavedData::SavedData()
{
	if(!g_counter++)
	{
		g_sds.enter();
	}
}

//=====================================================================================//
//                               SavedData::~SavedData()                               //
//=====================================================================================//
SavedData::~SavedData()
{
	if(!--g_counter)
	{
		g_sds.leave();
	}
}

}