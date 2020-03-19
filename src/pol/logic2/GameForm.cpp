//
// игровое меню iteration 2
//
#include "logicdefs.h"
#include "assert.h"

#include <undercover/interface/screens.h>
#include <undercover/interface/mousecursor.h>
//#include "../interface/inventoryscreen.h"
//#include "../interface/tradescreen.h"
//#include "../interface/dialogscreen.h"
//#include "../interface/journalscreen.h"
//#include "../interface/optionsscreen.h"
//#include "../interface/descriptionscreen.h"

#include "Thing.h"
//#include "sscene.h"
#include "Phrase.h"
#include "Entity.h"
#include "VisMap.h"
#include "Marker.h"
#include "bureau.h"
#include "AIUtils.h"
#include "HexGrid.h"
#include "Graphic.h"
#include "strategy.h"
#include "sndutils.h"
#include "HexUtils.h"
#include "Activity.h"
#include "AIContext.h"
#include "Cameraman.h"
#include "Spectator.h"
#include "PathUtils.h"
#include "TraceUtils.h"
#include "DirtyLinks.h"
#include "GameFormImp.h"
//#include "QuestServer.h"
#include "thingfactory.h"
#include "MultiMoveUtils.h"
//#include "phrasemanager.h"
#include <logic_ex/ActMgr.h>
#include <logic_ex/Bureau2.h>
#include <logic_ex/StoreBox2.h>
#include <common/Utils/VFileWpr.h>

#include "DataBox.h"
#include "StoreBox.h"
#include "BoxVisitor.h"
#include "GameObjectsMgr.h"

#include <common/D3DApp/Input/Input.h>

#include <common/CmdLine/CmdLine.h>
#include <common/GraphPipe/GraphPipe.h>

#include <Muffle/ISound.h>

const float ground_scroll_radius       = 2.0f;
const float default_hero_scroll_radius = 5.0f;

const int   HumanIcons::m_dead_lvl  = 10;
const float HumanIcons::m_wound_lvl = 0.5f;

namespace GameFormDetail
{
}

using namespace GameFormDetail;

namespace PoL
{

//=====================================================================================//
//                                struct AlternateKeys                                 //
//=====================================================================================//
struct AlternateKeys
{
	bool m_altKeys;

	AlternateKeys()
	:	m_altKeys(false)
	{
		VFileWpr vfile("scripts/version.txt");

		if(vfile.get() && vfile.get()->Size() > 0)
		{
			VFileBuf buf(vfile.get());
			std::istream in(&buf);

			std::string option;

			in >> option;

			if(option == "alternate-sit-key")
			{
				int val;
				in >> val;
				m_altKeys = (val == 1);
			}
		}
	}
};

//=====================================================================================//
//                          const AlternateKeys &GetAltKeys()                          //
//=====================================================================================//
const AlternateKeys &GetAltKeys()
{
	static AlternateKeys inst;
	return inst;
}

//=====================================================================================//
//                            unsigned long GetCrouchKey()                             //
//=====================================================================================//
unsigned long GetCrouchKey()
{
	if(GetAltKeys().m_altKeys)
	{
		return DIK_C;
	}
	else
	{
		return DIK_S;
	}
}

//=====================================================================================//
//                           unsigned long GetContinueKey()                            //
//=====================================================================================//
unsigned long GetContinueKey()
{
	if(GetAltKeys().m_altKeys)
	{
		return DIK_T;
	}
	else
	{
		return DIK_C;
	}
}

//=====================================================================================//
//                           unsigned long GetAutoShotKey()                            //
//=====================================================================================//
unsigned long GetAutoShotKey()
{
	if(GetAltKeys().m_altKeys)
	{
		return DIK_SLASH;
	}
	else
	{
		return DIK_COMMA;
	}
}

//=====================================================================================//
//                           unsigned long GetSnapShotKey()                            //
//=====================================================================================//
unsigned long GetSnapShotKey()
{
	if(GetAltKeys().m_altKeys)
	{
		return DIK_PERIOD;
	}
	else
	{
		return DIK_PERIOD;
	}
}

//=====================================================================================//
//                            unsigned long GetAimShotKey()                            //
//=====================================================================================//
unsigned long GetAimShotKey()
{
	if(GetAltKeys().m_altKeys)
	{
		return DIK_COMMA;
	}
	else
	{
		return DIK_SLASH;
	}
}

}

//==============================================================

void DirtyLinks::MakeQuickSaveLoad(GenericStorage::storage_mode mode)
{
	Screens::Instance()->Game()->MakeSpecialSaveLoad(GameScreen::SST_QUICKSAVE, 
		mode == GenericStorage::SM_LOAD ? GameScreen::SSM_LOAD : GameScreen::SSM_SAVE);
}

//==============================================================

GameFormImp *FormFactory::CreateGameForm()
{
	return new GameFormImp;
}

//==============================================================

void DirtyLinks::Print(const std::string& str, message_type type)
{
	if(!Screens::Instance()->Game()->MsgWindow()->IsPulled())
		Screens::Instance()->Game()->MsgWindow()->IncMsgWindow();

	Screens::Instance()->Game()->MsgWindow()->AddText(str.c_str());

	if(type==MT_DENIAL) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_LOGIC_DENIED));
}

//==============================================================

namespace GameFormDetail
{
	//получить указатель на реализацию меню
	GameFormImp* GetGameFormImp()
	{
		return static_cast<GameFormImp*>(Forms::GetInst()->GetGameForm());
	}

	//переключить крыши на уровне
	void SwitchRoofs()
	{
		GameScreen::CHECKBOX_STATE state = Screens::Instance()->Game()->GetCBState(GameScreen::CB_ROOF);
		state = (state == GameScreen::CBS_ON) ? GameScreen::CBS_OFF : GameScreen::CBS_ON;
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_ROOF, state);
		DirtyLinks::EnableRoofs(state == GameScreen::CBS_OFF);
	}

	//переключить вид "из глаз"
	void SwitchEyes()
	{
		GameScreen::CHECKBOX_STATE state = Screens::Instance()->Game()->GetCBState(GameScreen::CB_EYES);
		state = (state == GameScreen::CBS_ON) ? GameScreen::CBS_OFF : GameScreen::CBS_ON;
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_EYES, state);
		Camera *Cam=IWorld::Get()->GetPipe()->GetCam();
		if(Cam->GetMoveType() != cmtPersonBone)
		{
			EntityPool *pool = EntityPool::GetInst();
			for(EntityPool::Iterator i = pool->begin(ET_ALL_ENTS,PT_PLAYER1); i != pool->end(); ++i)
			{
				ActiveContext *ac = static_cast<ActiveContext*>(i->GetEntityContext());
				if(ac->IsSelected())
				{
					Cam->AttachToEntity(i->GetEID());
					break;
				}
			}
		}
		else
		{
			Cam->DetachFromEntity();
		}
	}

	bool IsEnemy4Player(BaseEntity* entity)
	{
		//RelationType relation = EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity);
		return EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity) == RT_ENEMY;
	}

	void SetShotCBs(GameScreen::CHECKBOX active_box)
	{
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_AIMSHOT, active_box == GameScreen::CB_AIMSHOT  ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_AUTOSHOT, active_box == GameScreen::CB_AUTOSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_SNAPSHOT, active_box == GameScreen::CB_SNAPSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);   
	}

	void SetRunSitCBs(HumanEntity* human)
	{
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_RUN, human->GetGraph()->IsRunMove() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_SIT, human->GetGraph()->IsSitPose() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
	}

	bool IsLeftMouseButtonClick()
	{
		return  Input::MouseState().LButtonFront
			&& !Input::MouseState().RButtonState
			&& !Input::MouseState().MButtonState;
	}

	bool IsLeftMouseButtonDoubleClick(float click_time)
	{
		return Input::MouseState().LDblClick
			&& click_time + Input::Mouse()->DoubleClickTime() > Timer::GetSeconds();
	}

	template<class Icons, class Entity>
	Entity* FindNextEntity(Icons* icons, Entity* current)
	{
		int icon = current ? icons->Entity2Icon(current) : -1;

		//найдем следующее существо
		for(icon++; icon < icons->GetMaxCount(); icon++)
		{
			if(Entity* next = icons->Icon2Entity(icon))
				return next;
		}

		return 0;
	}

	bool CanChangeHumanPose(HumanEntity* human)
	{
		AIUtils::reason_type reason;

		if(AIUtils::CanChangePose(human, &reason))
			return true;

		DirtyLinks::Print( AIUtils::Reason2Str(reason), DirtyLinks::MT_DENIAL);
		return false;
	}

	bool IsInCrew(HumanEntity *human)
	{
		if(human->GetEntityContext()->GetCrew())
		{
			DirtyLinks::Print( AIUtils::Reason2Str(AIUtils::RT_IN_CREW), DirtyLinks::MT_DENIAL);
			return true;
		}
		return false;
	}

	bool IsChecked(GameScreen::CHECKBOX check_box)
	{
		return Screens::Instance()->Game()->GetCBState(check_box) == GameScreen::CBS_ON;
	}

	int GetPressedIndex()
	{
		if(Input::KeyBack(DIK_0)) return 9;
		if(Input::KeyBack(DIK_1)) return 0;
		if(Input::KeyBack(DIK_2)) return 1;
		if(Input::KeyBack(DIK_3)) return 2;
		if(Input::KeyBack(DIK_4)) return 3;
		if(Input::KeyBack(DIK_5)) return 4;
		if(Input::KeyBack(DIK_6)) return 5;
		if(Input::KeyBack(DIK_7)) return 6;
		if(Input::KeyBack(DIK_8)) return 7;
		if(Input::KeyBack(DIK_9)) return 8;

		return -1;
	}

	bool IsAnyNewJournalInfo()
	{
		return false;
	}

	void SetNextFastThing(HumanEntity* human, FastAccessStrategy::iter_type type)
	{        
		FastAccessStrategy* fast_access = human->GetEntityContext()->GetFastAccessStrategy();
		std::auto_ptr<FastAccessStrategy::Iterator> itor(fast_access->CreateIterator(type));

		//найдем следущий предмет
		itor->First(fast_access->GetThing());

		//если такого нет то уст. первый в посл-ти
		if(itor->IsDone()) itor->First(0);

		//установим предмет
		fast_access->SetThing(itor->Get());
	}
}

//==============================================================

namespace GameFormDetail{

	class PanicForm : public BaseForm{
	public:

		void Show()
		{
			Screens::Instance()->Game()->ShowControls(false);
			Screens::Instance()->Activate(Screens::SID_PANIC);
		}

		void Init(const ini_s& ini){}
		void HandleInput(state_type* st){}
		void MakeSaveLoad(SavSlot& slot){}
	};
}

BaseForm* FormFactory::CreatePanicForm()
{
	return new PanicForm();
}

//==============================================================

namespace GameFormDetail {

	class Switch2Aimshot : public EntityVisitor{
	public:

		void Visit(HumanEntity* human)
		{   
			HumanContext* context = human->GetEntityContext();

			if(!context->GetThingInHands(TT_WEAPON))
				return;

			context->SetShotType(SHT_AIMSHOT);
			SetShotCBs(GameScreen::CB_AIMSHOT);
		}
	};
}

//==============================================================

namespace GameFormDetail {

	class Switch2Autoshot : public EntityVisitor{
	public:

		void Visit(HumanEntity* human)
		{
			HumanContext* context = human->GetEntityContext();

			WeaponThing* thing = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON));

			if(thing == 0 || !thing->GetInfo()->IsAuto())
				return;

			context->SetShotType(SHT_AUTOSHOT);
			SetShotCBs(GameScreen::CB_AUTOSHOT);
		}        
	};
}

//==============================================================

namespace GameFormDetail {

	class Switch2Snapshot : public EntityVisitor{
	public:

		void Visit(HumanEntity* human)
		{
			HumanContext* context = human->GetEntityContext();

			if(!context->GetThingInHands(TT_WEAPON)) return;

			context->SetShotType(SHT_SNAPSHOT);
			SetShotCBs(GameScreen::CB_SNAPSHOT);
		}
	};
}

//==============================================================

namespace GameFormDetail {

	class Switch2Sit : public EntityVisitor
	{
	public:

		void Visit(HumanEntity* human)
		{
			if((human->GetGraph()->IsSitPose() || !CanChangeHumanPose(human)) || IsInCrew(human))
			{
				return;
			}

			human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_WALK);
			human->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_SIT);

			SetRunSitCBs(human);
			AIUtils::CalcAndShowPassField(human);
		}
	};

	class Switch2Stand : public EntityVisitor
	{
	public:

		void Visit(HumanEntity* human)
		{
			if((human->GetGraph()->IsSitPose() && !CanChangeHumanPose(human)) || IsInCrew(human))
			{
				return;
			}

			human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_WALK);
			human->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_STAND);

			SetRunSitCBs(human);
			AIUtils::CalcAndShowPassField(human);
		}
	};
}    

//==============================================================

namespace GameFormDetail {

	class Switch2Run : public EntityVisitor
	{
	public:

		void Visit(HumanEntity* human)
		{
			if((human->GetGraph()->IsSitPose() && !CanChangeHumanPose(human)) || IsInCrew(human))
			{
				return;
			}

			human->GetGraph()->Cast2Human()->ChangeMoveType(GraphHuman::MT_RUN);
			human->GetGraph()->Cast2Human()->ChangePose(GraphHuman::PT_STAND);

			SetRunSitCBs(human);
			AIUtils::CalcAndShowPassField(human);
		}
	};
}

//==============================================================

namespace GameFormDetail
{
	//
	// обычная стратегия перемотки персонажей через меню GameFormImp
	//
	class GameFormHeroScroller : public AbstractScroller{
	public:

		static GameFormHeroScroller* GetInst()
		{
			static GameFormHeroScroller imp;
			return &imp;
		}

		void Prev()
		{
			HumanIcons* icons = GetGameFormImp()->GetHumanIcons();

			for(--m_index; m_index >= 0; --m_index){

				if(IsSuitable(icons->Icon2Entity(m_index)))
					return;
			}

			if(m_index < 0) m_index = -1;
		}

		bool IsREnd() const
		{
			return m_index == -1;
		}

		void Last(HumanEntity* human)
		{
			for(int k = MAX_TEAMMATESTOTAL - 1; human && k >= 0; k--)
			{
				HumanEntity* hero = GetGameFormImp()->GetHumanIcons()->Icon2Entity(k);

				if(IsSuitable(hero) && human->GetEID() == hero->GetEID())
				{
					m_index = k;
					return;
				} 
			}

			m_index = MAX_TEAMMATESTOTAL;

			Prev();
		}

		void Next()
		{
			HumanIcons* icons = GetGameFormImp()->GetHumanIcons();

			for(++m_index; m_index < MAX_TEAMMATESTOTAL; ++m_index)
			{
				if(IsSuitable(icons->Icon2Entity(m_index)))
					return;
			}

			if(m_index >= MAX_TEAMMATESTOTAL) m_index = MAX_TEAMMATESTOTAL;
		}

		bool IsEnd() const
		{
			return m_index == MAX_TEAMMATESTOTAL;
		}

		void First(HumanEntity* human = 0)
		{
			for(int k = 0; human && k < MAX_TEAMMATESTOTAL; k++)
			{
				HumanEntity* hero = GetGameFormImp()->GetHumanIcons()->Icon2Entity(k);

				if(IsSuitable(hero) && human->GetEID() == hero->GetEID())
				{                    
					m_index = k;
					return;
				}
			}

			m_index = -1;

			Next();
		}

		HumanEntity* Get()
		{
			return (IsEnd() || IsREnd()) ? 0 :  GetGameFormImp()->GetHumanIcons()->Icon2Entity(m_index);
		}

	protected:

		virtual bool IsSuitable(HumanEntity* hero) const
		{
			return hero != 0;
		}

		GameFormHeroScroller(){}    

	private:

		int m_index;
	};
}

//==============================================================

namespace GameFormDetail{

	//
	// скролинг по всем персонажам в нек. радиусе
	//
	class RadiusHeroScroller : public GameFormHeroScroller {
	public:

		static RadiusHeroScroller* GetInst()
		{ 
			static RadiusHeroScroller imp;
			return &imp;
		}

		void Init(const point3& from, float radius = default_hero_scroll_radius)
		{
			m_from = from;
			m_radius = radius;
		}

	private:

		bool IsSuitable(HumanEntity* hero) const
		{
			return hero && (m_from - hero->GetGraph()->GetPos3()).Length() < m_radius;            
		}

		RadiusHeroScroller() : m_radius(default_hero_scroll_radius) {}

	private:

		point3 m_from;
		float  m_radius;
	};
}

//==============================================================

namespace GameFormDetail{

	//
	//  скроллинг только по одному человеку
	//
	class SingleHumanScroller : public AbstractScroller{
	public:

		static SingleHumanScroller* GetInst()
		{
			static SingleHumanScroller imp;
			return &imp;
		}

		void Prev() {}

		void Next() {}

		bool IsEnd() const { return true; }

		bool IsREnd() const { return true; }

		void Last(HumanEntity* human) { }

		void First(HumanEntity* human) { }

		HumanEntity* Get() { return m_human; }

		void Init(HumanEntity* human) { m_human = human; }

	private:

		HumanEntity* m_human;
	};
}

//==============================================================

namespace GameFormDetail{

	//узнать позицию целовека
	ipnt2_t GetHumanPos(HumanEntity* human)
	{
		if(VehicleEntity* vehicle = human->GetEntityContext()->GetCrew())
			return vehicle->GetGraph()->GetPos2();

		return human->GetGraph()->GetPos2();
	}

	//
	// брать предметы с обычной земли
	//
	class UsualGround : public AbstractGround{
	public:

		static UsualGround* GetInst()
		{
			static UsualGround imp;
			return &imp;
		}

		bool CanInsert(HumanEntity* human, BaseThing* thing, std::string* reason) const 
		{
			return true;
		}

		void Insert(HumanEntity* human, BaseThing* thing)
		{   
			Depot::GetInst()->Push(GetHumanPos(human), thing);
		}

		void Remove(HumanEntity* human, BaseThing* thing)
		{
			Depot::GetInst()->Remove(thing->GetTID());
		}

		AbstractGround::Iterator* CreateIterator(HumanEntity* human)        
		{
			return new UsualGroundItor(human);
		}

		void PlaySound(sound_event sound)
		{
			if(sound == SE_DROP) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DROP_TO_GROUND));
			if(sound == SE_DRAG) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DRAG_FROM_GROUND));
		}

	private:

		UsualGround(){}

		class UsualGroundItor : public AbstractGround::Iterator{
		public:

			UsualGroundItor(HumanEntity* human) : m_actor(human) { First(); }

			BaseThing* Get() { return &*m_first; }

			void Next() { m_first ++; }

			void First() { m_first = Depot::GetInst()->begin(GetHumanPos(m_actor), ground_scroll_radius); }

			bool IsNotDone() const { return m_first != Depot::GetInst()->end(); }

		private:

			HumanEntity*    m_actor;
			Depot::iterator m_first;            
		};
	};
}

//==============================================================

namespace GameFormDetail{

	//
	// стратегия для сбрасывания предметов на землю
	//
	class GroundBin : public AbstractBin{
	public:

		static GroundBin* GetInst()
		{
			static GroundBin bin;
			return &bin;
		} 

		void Insert(HumanEntity* actor, BaseThing* thing)
		{
			Depot::GetInst()->Push(GetHumanPos(actor), thing);
		}

	private:

		GroundBin() {}
	};
}

//==============================================================

namespace GameFormDetail
{
	////
	//// стратегия для вынимания предметов из ящика
	////
	//class StoreBoxGround : public AbstractGround
	//{
	//public:
	//	static StoreBoxGround* GetInst()
	//	{ 
	//		static StoreBoxGround imp;
	//		return &imp;
	//	}

	//	void SetStoreBox(StoreBox* box) { m_box = box; }

	//	bool CanInsert(HumanEntity* human, BaseThing* thing, std::string* reason) const 
	//	{
	//		//вычислим вес ящика с предметом thing
	//		float weight = thing->GetWeight();
	//		for(StoreBox::iterator itor = m_box->begin(); itor != m_box->end(); ++itor)
	//			weight += itor->GetWeight();

	//		if(weight > m_box->GetWeight()){
	//			if(reason) *reason = DirtyLinks::GetStrRes("box_full");
	//			return false;
	//		}

	//		return true;
	//	}

	//	void Insert(HumanEntity* human, BaseThing* thing)
	//	{   
	//		m_box->Insert(thing);
	//	}

	//	void Remove(HumanEntity* human, BaseThing* thing)
	//	{
	//		m_box->Remove(thing);
	//	}

	//	AbstractGround::Iterator* CreateIterator(HumanEntity* human)
	//	{
	//		return new StoreBoxItor(m_box);
	//	}

	//	void PlaySound(sound_event sound)
	//	{
	//		if(sound == SE_DROP) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DROP_TO_BOX));
	//		if(sound == SE_DRAG) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DRAG_FROM_BOX));
	//	}

	//private:

	//	StoreBoxGround(){}

	//	class StoreBoxItor : public AbstractGround::Iterator{
	//	public:

	//		StoreBoxItor(StoreBox* box) : m_box(box) { First(); }

	//		void Next() { ++ m_first; }

	//		BaseThing* Get() { return &*m_first; }

	//		void First() { m_first = m_box->begin(); }

	//		bool IsNotDone() const { return m_first != m_box->end(); }

	//	private:

	//		StoreBox*          m_box;
	//		StoreBox::iterator m_first;        
	//	};

	//private:

	//	StoreBox* m_box;
	//};

//=====================================================================================//
//                    class StoreBoxGround2 : public AbstractGround                    //
//=====================================================================================//
	// стратегия для вынимания предметов из ящика
	class StoreBoxGround2 : public AbstractGround
	{
		PoL::HStoreBox2 m_box;

	public:
		static StoreBoxGround2 *GetInst()
		{ 
			static StoreBoxGround2 imp;
			return &imp;
		}

		void SetStoreBox(PoL::HStoreBox2 box)
		{
			m_box = box;
		}

		bool CanInsert(HumanEntity* human, BaseThing* thing, std::string* reason) const 
		{
			//вычислим вес ящика с предметом thing
			const float weight = thing->GetWeight() + m_box->getCurWeight();

			if(weight > m_box->getMaxWeight())
			{
				if(reason) *reason = DirtyLinks::GetStrRes("box_full");
				return false;
			}

			return true;
		}

		void Insert(HumanEntity* human, BaseThing* thing)
		{
			m_box->add(thing);
		}

		void Remove(HumanEntity* human, BaseThing* thing)
		{
			m_box->remove(thing);
		}

		AbstractGround::Iterator* CreateIterator(HumanEntity* human)
		{
			return new StoreBoxItor(m_box);
		}

		void PlaySound(sound_event sound)
		{
			if(sound == SE_DROP) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DROP_TO_BOX));
			if(sound == SE_DRAG) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_DRAG_FROM_BOX));
		}

	private:
		StoreBoxGround2() {}

		class StoreBoxItor : public AbstractGround::Iterator
		{
			PoL::HStoreBox2 m_box;
			PoL::StoreBox2::Iterator_t m_itor;

		public:
			StoreBoxItor(PoL::HStoreBox2 box) : m_box(box) { First(); }

			void First() { m_itor = m_box->thingsBegin(); }
			void Next() { ++m_itor; }

			BaseThing* Get() { return *m_itor; }
			bool IsNotDone() const { return m_itor != m_box->thingsEnd(); }
		};
	};
};

//==============================================================

namespace GameFormDetail
{

	using PoL::BaseEntityRef;
	using PoL::HumanEntityRef;
	using PoL::VehicleEntityRef;

	//
	// механизм подхода к существу
	//
	class Approach2EntityManager : public ActionManager
	{
	protected:
		enum rotate_type
		{
			RT_ACTOR2ENTITY, //actor поворачивается к существу
			RT_ENTITY2ACTOR, //существо повворачивается к actor
		};

	private:
		rotate_type m_rotate;

		BaseEntityRef m_actor;
		BaseEntityRef m_entity;

	public:
		Activity* CreateMove()
		{
			if(m_actor.valid())
			{
				pnt_vec_t path;
				PathUtils::CalcPath(m_actor.get(), PathUtils::GetNearPnt(m_entity.get()).m_pnt, path);

				return path.empty() ? 0 : ActivityFactory::CreateMove(m_actor.get(), path, ActivityFactory::CT_PLAYER_MOVE);
			}
			else
			{
				return 0;
			}
		}

		Activity* CreateRotate()
		{
			if(m_actor.valid() && m_entity.valid())
			{
				return      m_rotate == RT_ACTOR2ENTITY
					?   CreateRotate(m_actor.get(), m_entity.get())
					:   CreateRotate(m_entity.get(), m_actor.get());
			}
			else
			{
				return 0;
			}
		}

		bool NeedMove() const
		{
			if(!m_actor.valid() || !m_entity.valid()) return false;

			if(m_actor->GetEID() == m_entity->GetEID())
			{
				return false; 
			}

			PathUtils::CalcPassField(m_actor.get());
			return PathUtils::GetNearPnt(m_entity.get()).m_cost != 1;
		}

		bool NeedRotate() const
		{
			if(!m_actor.valid() || !m_entity.valid()) return false;

			return      m_rotate == RT_ACTOR2ENTITY
				?   NeedRotate(m_actor.get(), m_entity.get())
				:   NeedRotate(m_entity.get(), m_actor.get());
		}

	protected:

		Approach2EntityManager() {}

		void Init(BaseEntity* actor, BaseEntity* entity, rotate_type type)
		{
			m_actor.reset(actor);
			m_entity.reset(entity);
		}

	private:

		//должен ли первый повернуться ко второму
		bool NeedRotate(BaseEntity* first, BaseEntity* second) const
		{
			if(first->GetEID() == second->GetEID())
				return false; 

			point3 dir = second->GetGraph()->GetPos3() - first->GetGraph()->GetPos3();
			return first->GetGraph()->NeedRotate(dir);
		}

		//создать поворот первого ко второму
		Activity* CreateRotate(BaseEntity* first, BaseEntity* second)
		{
			point3 dir = second->GetGraph()->GetPos3() - first->GetGraph()->GetPos3();
			return ActivityFactory::CreateRotate(first, Dir2Angle(dir), ActivityFactory::CT_ROTATE_FOR_USE);
		}
	};
}

//==============================================================

#pragma warning(push)
#pragma warning(disable:4584)

namespace GameFormDetail
{

	//
	// класс для Use на существе
	//
	class EntityUseManager : public Approach2EntityManager, private EntityVisitor
	{
		HumanEntityRef m_actor;
		BaseEntityRef  m_ent4use;     

	public:
		static EntityUseManager* GetInst()
		{
			static EntityUseManager imp;
			return &imp;
		}

		void Init(HumanEntity* actor, BaseEntity* ent4use)
		{
			m_actor.reset(actor);
			m_ent4use.reset(ent4use);

			Approach2EntityManager::Init(actor, ent4use, RT_ENTITY2ACTOR);
		}

		void Shut()
		{
			if(m_actor.valid() && m_ent4use.valid())
			{
				m_ent4use->Accept(*this);
			}
		}

	private:
		EntityUseManager() {}

		void Visit(HumanEntity* human)
		{
			UseBaseEntity(human);
		}

		void Visit(TraderEntity* trader)
		{
			UseBaseEntity(trader);

			if(!PoL::ActMgr::isPlayingScene())
			{
				RadiusHeroScroller::GetInst()->Init(trader->GetGraph()->GetPos3());

				if(EnemyDetector::getInst()->isHeEnemy4Me(m_actor.get(), trader))
				{
					Forms::GetInst()->ShowTalkDialog(trader, phrase_s(DirtyLinks::GetStrRes("gf_cant_trade"), ""));
				}
				else
				{
					GetGameFormImp()->HandleShowShopReq(m_actor->Cast2Human(), trader,
						UsualGround::GetInst(),
						RadiusHeroScroller::GetInst(),
						GroundBin::GetInst());
				}
			}
		}

		void Visit(VehicleEntity* vehicle)
		{
			UseBaseEntity(vehicle);
		}

		void UseBaseEntity(BaseEntity* entity)
		{
			GameObserver::use_info info(m_actor.get(), entity);
			GameEvMessenger::GetInst()->Notify(GameObserver::EV_USE, &info);
			// gvozdoder:old_quests: QuestServer::GetInst()->HandleUseEntity(m_actor, entity);   
		}
	};
}

//==============================================================

namespace GameFormDetail
{

	//
	// положить предмет в ящик
	//
	class StoreBoxAcceptor : public ThingScriptParser::Acceptor
	{
	public:

		StoreBoxAcceptor(StoreBox* box) : m_box(box) {}

		bool AcceptThing(BaseThing* thing)
		{
			m_box->Insert(thing);
			return true;
		}

		//принять информцию об организации
		void AcceptOrgInfo(const rid_t rid) {}
		//принять информацию о существах
		void AcceptEntInfo(entity_type type, const rid_t& rid) {}
		//принять информацию о предметах
		void AcceptThingInfo(thing_type type, const rid_t& rid) {}

	private:

		StoreBox* m_box;
	};

	//
	// принять информацию с консоли
	//
	class HandbookAcceptor : public ThingScriptParser::Acceptor
	{
	public:

		bool AcceptThing(BaseThing* thing) { return false; }

		void AcceptOrgInfo(const rid_t rid)
		{
			ForceHandbook::GetInst()->Push(0, rid, ForceHandbook::F_NEW_REC);
		}

		void AcceptThingInfo(thing_type type, const rid_t& rid)
		{
			ThingHandbook::GetInst()->Push(type, rid, ThingHandbook::F_NEW_REC);
		}

		void AcceptEntInfo(entity_type type, const rid_t& rid)
		{
			EntityHandbook::GetInst()->Push(type, rid, EntityHandbook::F_NEW_REC);
		}
	};

	//
	// класс для use на объекте
	//
	class ObjectUseManager : public ActionManager, private BoxVisitor
	{
		mll::utils::table m_useTable;
		mll::utils::table::row_type m_header;
		mll::utils::table::col_type m_level;
		mll::utils::table::col_type m_objects;

		rid_t m_object;

		pnt_vec_t      m_obj_hexes;
		HumanEntityRef m_actor;

	public:
		static ObjectUseManager* GetInst()
		{
			static ObjectUseManager imp;
			return &imp;
		}

		void Init(HumanEntity* actor, const rid_t& rid)
		{
			m_object = rid;
			m_actor.reset(actor);
			m_obj_hexes.clear();
			DirtyLinks::CalcObjectHexes(m_object, &m_obj_hexes);
		}

		Activity* CreateAction()
		{
			unsigned int uc = ActivityFactory::CT_PLAYER_USE_SUCCEED;

			if(!m_actor.valid() || !DirtyLinks::IsGoodEntity(m_actor.get()))
			{
				m_object.clear();
				m_actor.reset();
				return 0;
			}
			else
			{
				if(IsSSceneObject(m_object))
				{
					uc = ActivityFactory::CT_PLAYER_USE_SSCENE;
				}
				else if(!CanUseObject(m_object))
				{
					uc = ActivityFactory::CT_PLAYER_USE_FAILED;
					m_object.clear();
				}
			}

			return ActivityFactory::CreateUse(m_actor.get(), m_object, uc);   
		}

		Activity* CreateMove()
		{
			if(m_actor.valid())
			{
				PathUtils::near_pnt_t near_pnt = PathUtils::GetNearPnt(m_obj_hexes);

				if(!near_pnt.IsDefPnt())
				{
					pnt_vec_t path;
					PathUtils::CalcPath(m_actor.get(), near_pnt.m_pnt, path);
					return ActivityFactory::CreateMove(m_actor.get(), path, ActivityFactory::CT_PLAYER_MOVE);
				}
			}

			return 0;
		}

		Activity* CreateRotate()
		{
			if(m_actor.valid())
			{
				point3 dir = DirtyLinks::GetObjCenter(m_object) - m_actor->GetGraph()->GetPos3();
				return ActivityFactory::CreateRotate(m_actor.get(), Dir2Angle(dir), ActivityFactory::CT_ROTATE_FOR_USE);
			}
			return 0;
		}

		bool NeedMove() const
		{
			if(m_actor.valid())
			{
				PathUtils::CalcPassField(m_actor.get());

				PathUtils::near_pnt_t near_pnt = PathUtils::GetNearPnt(m_obj_hexes);
				return near_pnt.m_cost != 0;
			}
			return false;
		}

		bool NeedRotate() const
		{
			if(m_actor.valid())
			{
				point3 dir = DirtyLinks::GetObjCenter(m_object) - m_actor->GetGraph()->GetPos3();
				return m_actor->GetGraph()->NeedRotate(dir);
			}
			return false;
		}

		void Shut()
		{
			//если объект есть
			if(m_object.size() && m_actor.valid())
			{
				// gvozdoder: these are redundant
				//GameObserver::use_info info(m_actor, m_object);
				//GameEvMessenger::GetInst()->Notify(GameObserver::EV_USE, &info);

				if(BaseBox* box = Bureau::GetInst()->Get(m_object))
				{
					box->Accept(*this);
				}

				//пометить как использованный
				GameObjectsMgr::GetInst()->MarkAsUsed(m_object);
			}
		}

	private:

		ObjectUseManager()
		{
			PoL::VFileWpr vfile("scripts/pol/sscene_obj.txt");
			if(vfile.get() && vfile.get()->Size())
			{
				PoL::VFileBuf buf(vfile.get());
				std::istream in(&buf);
				in >> m_useTable;

				m_header = m_useTable.row(0);
				m_level = m_useTable.col(std::find(m_header.begin(),m_header.end(),"level"));
				m_objects = m_useTable.col(std::find(m_header.begin(),m_header.end(),"object"));
			}
		}

		bool IsSSceneObject(const rid_t &obj) const
		{
			std::string lev;

			for(int i = 1; i != m_useTable.height(); ++i)
			{
				if(!m_level[i].empty()) lev = m_level[i];
				if(m_objects[i] == obj && lev == DirtyLinks::GetLevelRawSysName()) return true;
			}

			return false;
		}

		bool CanUseObject(const rid_t& obj_rid)
		{
			if(GameObjectsMgr::GetInst()->IsUsed(obj_rid))
			{
				return true;
			}

			HumanContext* context = m_actor->GetEntityContext();
			rid_t         key_rid = DirtyLinks::GetKey4UseObj(obj_rid);

			//найдем ключ у человека
			if(key_rid.size())
			{
				HumanContext::iterator itor = context->begin(HPK_ALL, TT_KEY);

				while(itor != context->end())
				{
					if(itor->GetInfo()->GetRID() == key_rid)
					{
						KeyThing* thing = static_cast<KeyThing*>(&*itor);

						m_actor->Notify(EntityObserver::EV_OPEN_SUCCEED);

						DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("gf_key_used").c_str(),
							m_actor->GetInfo()->GetName().c_str(),
							thing->GetInfo()->GetName().c_str()));

						DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("open_success").c_str(),
							m_actor->GetInfo()->GetName().c_str()));

						//проиграть звук на используемый ключ
						SndPlayer::GetInst()->Play(thing->GetInfo()->GetUseSound());

						context->RemoveThing(itor);
						delete thing;
						return true;
					}

					++itor;
				}
			}

			int wisdom = DirtyLinks::GetWisdom4UseObj(obj_rid);

			if(wisdom >= 0 && context->GetTraits()->GetWisdom() > wisdom)
			{
				if(wisdom)
				{
					m_actor->Notify(EntityObserver::EV_OPEN_SUCCEED);
					DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("open_success").c_str(),
						m_actor->GetInfo()->GetName().c_str()));
				}

				return true;
			}

			m_actor->Notify(EntityObserver::EV_OPEN_FAILED);

			if(key_rid.size())
			{
				DirtyLinks::Print( mlprintf(DirtyLinks::GetStrRes("gf_need_key").c_str(), 
					m_actor->GetInfo()->GetName().c_str()));
			}
			else if(wisdom >= 0)
			{
				DirtyLinks::Print( mlprintf(DirtyLinks::GetStrRes("gf_no_wisdom").c_str(),
					m_actor->GetInfo()->GetName().c_str())); 
			}

			DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("open_fail").c_str(), m_actor->GetInfo()->GetName().c_str()), DirtyLinks::MT_DENIAL);
			return false;            
		}

		void Visit(StoreBox* store)
		{
			//если ящик до этого не открывался заполним его предметами
			if(!GameObjectsMgr::GetInst()->IsUsed(store->GetRID()))
			{
				if(store->GetScriptRID().size())
				{
					StoreBoxAcceptor  acceptor(store);
					ThingScriptParser parser(&acceptor, ThingFactory::GetInst());

					int line;
					std::string script;

					if(!AIUtils::GetItemSetStr(store->GetScriptRID(), &script, &line))
						ThrowCantReadScript(store->GetRID(), store->GetScriptRID());

					if(!parser.Parse(script))
					{
						ThrowScriptParseError(store->GetRID(), store->GetScriptRID(), line, parser.GetLastPos());
					}
				}
			}

			if(!PoL::Bureau2::inst().hasBox(store->GetRID()))
			{
				AIUtils::AddExp4Hack(store->GetExperience(), m_actor.get());
			}

			PoL::HStoreBox2 box2 = PoL::Bureau2::inst().getBoxEx(store->GetRID());
			StoreBoxGround2::GetInst()->SetStoreBox(box2);

			SingleHumanScroller::GetInst()->Init(m_actor.get());

			GetGameFormImp()->HandleShowInventoryReq(
				GetGameFormImp()->GetCurEnt()->Cast2Human(), 
				StoreBoxGround2::GetInst(),
				SingleHumanScroller::GetInst(),
				GroundBin::GetInst());
		}

		void Visit(DataBox* console)
		{
			//если ящик еще не открывался
			if(!GameObjectsMgr::GetInst()->IsUsed(console->GetRID()))
			{
				HandbookAcceptor acceptor;
				ThingScriptParser parser(&acceptor);

				int line;
				std::string script;

				if(!AIUtils::GetItemSetStr(console->GetScriptRID(), &script, &line))
					ThrowCantReadScript(console->GetRID(), console->GetScriptRID());

				if(!parser.Parse(script))
					ThrowScriptParseError(console->GetRID(), console->GetScriptRID(), line, parser.GetLastPos());

				AIUtils::AddExp4Hack(console->GetExperience(), m_actor.get());
				return;
			}

			DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("no_data").c_str(), m_actor->GetInfo()->GetName().c_str()));                        
		}

		void ThrowCantReadScript(const rid_t& box_rid, const rid_t& script_rid)
		{
			std::ostringstream ostr;

			ostr << "GetItemSetStr: нет набора <" << script_rid << ">" << std::endl;
			ostr << "ящик:\t" << box_rid << std::endl;
			ostr << "уровень:\t" << DirtyLinks::GetLevelSysName() << std::endl;

			throw CASUS(ostr.str());
		}

		void ThrowScriptParseError(const rid_t& box_rid, const rid_t& script_rid, int line, int pos)
		{
			std::ostringstream ostr;

			ostr << "ScriptParser: ошибка разбора в наборе <" << script_rid << ">" << std::endl;
			ostr << "ящик:\t" << box_rid << std::endl;
			ostr << "уровень:\t " << DirtyLinks::GetLevelSysName() << std::endl;
			ostr << "item_sets.xls: line = " << line + 1 << " pos = " << pos << std::endl;

			throw CASUS(ostr.str());
		}
	};
};

//==============================================================

namespace GameFormDetail
{
	//
	// менеджер use'a для лечения
	//
	class TreatmentManager : public Approach2EntityManager
	{
		HumanEntityRef m_sick;
		HumanEntityRef m_doctor;
		MedikitThing *m_medikit;

	public:
		static TreatmentManager* GetInst()
		{
			static TreatmentManager imp;
			return &imp;
		}

		void Init(HumanEntity* doctor, HumanEntity* sick)
		{
			m_sick.reset(sick);
			m_doctor.reset(doctor);

			Approach2EntityManager::Init(doctor, sick, RT_ACTOR2ENTITY);

			m_medikit = static_cast<MedikitThing*>(doctor->GetEntityContext()->GetThingInHands(TT_MEDIKIT));
		}

		Activity* CreateAction()
		{
			if(m_doctor.valid() && m_sick.valid())
			{
				AIUtils::MakeTreatment(m_doctor.get(), m_sick.get(), m_medikit);
			}

			return 0;
		}

		void Shut()
		{
			if(m_medikit->GetCharge() == 0 && m_doctor.valid())
			{
				m_doctor->GetEntityContext()->RemoveThing(m_medikit);
				delete m_medikit;
			}
		}

	private:
		TreatmentManager() : m_medikit(0) {}
	};

//=====================================================================================//
//                  class SwapManager : public Approach2EntityManager                  //
//=====================================================================================//
	class SwapManager : public Approach2EntityManager
	{
		BaseEntityRef m_target;
		HumanEntityRef m_source;

	public:
		static SwapManager *GetInst()
		{
			static SwapManager imp;
			return &imp;
		}

		void Init(HumanEntity *source, BaseEntity *target)
		{
			m_target.reset(target);
			m_source.reset(source);

			Approach2EntityManager::Init(source, target, RT_ACTOR2ENTITY);
		}

		Activity* CreateAction()
		{
			if(m_source.valid() && m_target.valid())
			{
				AIUtils::MakeSwap(m_source.get(), m_target.get());
			}

			return 0;
		}

		void Shut()
		{
		}

	private:
		SwapManager()
		{
		}
	};
}

//==============================================================

namespace GameFormDetail
{

	//
	// менеджер взлома техники
	//
	class EntityHackManager : public Approach2EntityManager
	{
		float m_probability;

		HumanEntityRef   m_hacker;
		VehicleEntityRef m_vehicle;

	public:
		static EntityHackManager* GetInst()
		{
			static EntityHackManager imp;
			return &imp;
		}

		void Init(HumanEntity* hacker, VehicleEntity* vehicle, float probability)
		{
			m_hacker.reset(hacker);
			m_vehicle.reset(vehicle);

			m_probability = probability;

			Approach2EntityManager::Init(hacker, vehicle, RT_ACTOR2ENTITY);
		}

		void Shut()
		{
			if(m_vehicle.valid() && m_hacker.valid())
			{
				if(aiNormRand() < m_probability)
				{ 
					CommonScenario("gf_hack_succeed", EntityObserver::EV_OPEN_SUCCEED);

					AIUtils::ChangeEntityPlayer(m_vehicle.get(), PT_PLAYER);
					AIUtils::AddExp4Hack(m_vehicle->GetInfo()->GetExp4Hack(), m_hacker.get());

					GetGameFormImp()->HandleSelectReq(m_vehicle.get());
					return;
				}

				CommonScenario("gf_hack_failed", EntityObserver::EV_OPEN_FAILED);
			}
		}

	private:
		EntityHackManager() {}

		void CommonScenario(const rid_t& res_str, EntityObserver::event_t event)
		{
			m_hacker->Notify(event);
			m_hacker->GetEntityContext()->GetTraits()->PoLAddMovepnts(- MPS_FOR_HACK);

			DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes(res_str).c_str(),
				m_hacker->GetInfo()->GetName().c_str(),
				m_vehicle->GetInfo()->GetName().c_str()));
		}
	};
}

//==============================================================

namespace GameFormDetail
{

	//
	// менеджер для перемещения существа
	//
	class EntityMoveManager : public ActionManager
	{
		unsigned int m_flags;
		ipnt2_t m_hex;
		BaseEntityRef m_actor;

	public:
		enum flag_type
		{
			F_NEED_WALK       = 1 << 0, //перейти в режим дохождения если не достигли точки назначения
			F_CALC_PASS_FIELD = 1 << 1, //расчитать поле проходимости перед перемещением

			F_INS_2_WALKERS   = 1 << 2,

			F_USUAL_MOVE   = F_NEED_WALK,
			F_WALKERS_MOVE = F_NEED_WALK|F_CALC_PASS_FIELD,
		};

		virtual void Shut()
		{
			m_actor.reset();
			m_flags = 0;
		}

		static EntityMoveManager* GetInst()
		{
			static EntityMoveManager imp;
			return &imp;
		}

		void Init(BaseEntity* actor, const ipnt2_t& hex_pnt, unsigned flags = F_USUAL_MOVE)
		{
			m_hex = hex_pnt;
			m_actor.reset(actor);

			m_flags = flags;

			VehicleEntity* vehicle = actor->Cast2Vehicle();

			if(vehicle && !CanVehicleMove(vehicle))
			{
				m_actor.reset();

				if(IsNoDriver4Vehicle(vehicle))
				{
					EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
					while(itor != EntityPool::GetInst()->end())
					{
						VehicleEntity* crew = itor->Cast2Human()->GetEntityContext()->GetCrew();

						if(crew && crew->GetEID() == vehicle->GetEID())
						{
							itor->Notify(EntityObserver::EV_CAR_DRIVE_FAILED);
							break;
						}

						++itor;
					}

					DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("gf_no_driver").c_str(),
						vehicle->GetInfo()->GetName().c_str()));
				}
			}

			HumanEntity* human = actor->Cast2Human();
			if(human && !CanHumanMove(human))
			{
				m_actor.reset();

				//если не можем сделать ни шагу из-за перегрузки
				if(IsHumanOverloaded(human))
				{
					DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("weight_overload").c_str(),
						human->GetInfo()->GetName().c_str()));        
				}
			}

			if(m_actor.valid()) m_actor->Notify(EntityObserver::EV_MOVE_COMMAND);
		}

		bool NeedMove() const
		{
			if(m_actor.valid())
			{
				if(m_flags & F_INS_2_WALKERS && m_flags & F_NEED_WALK) GetGameFormImp()->GetWalkers()->Insert(m_actor.get(), m_hex);
			}

			return m_actor.valid();
		}

		Activity* CreateMove()
		{ 
			if(m_actor.valid())
			{
				if(
					Input::KeyState(DIK_A)
#ifdef _DEBUG
					|| CmdLine::IsKey("-am")
#endif
					)
				{
					std::vector<BaseEntity *> ents;
					std::vector<pnt_vec_t> paths;

					PoL::MultiMoveUtils::prepareMultiMove(ents,m_hex,paths);
					if(ents.size())
					{
						return ActivityFactory::CreateMultiMove(ents,paths,ActivityFactory::CT_PLAYER_MOVE);
					}
					else
					{
						return 0;
					}
				}

				if(m_flags & F_CALC_PASS_FIELD) PathUtils::CalcPassField(m_actor.get());

				pnt_vec_t   path;                            

				//вычислить путь и породить действие
				PathUtils::CalcPath(m_actor.get(), m_hex, path);

				if(m_flags & F_NEED_WALK)
				{
					//если путь длинный => занесемся в ходоки
					if((int)path.size() > m_actor->GetEntityContext()->GetStepsCount())
					{
						m_flags |= F_INS_2_WALKERS;
					}
					//if(HumanEntity* human = m_actor->Cast2Human())
					//{
					//	if((int)path.size() > human->GetEntityContext()->GetStepsCount())
					//	{
					//		m_flags |= F_INS_2_WALKERS;                  
					//	}
					//}
					//else if(VehicleEntity* vehicle = m_actor->Cast2Vehicle())
					//{
					//	if((int)path.size() > vehicle->GetEntityContext()->GetStepsCount())
					//	{
					//		m_flags |= F_INS_2_WALKERS;
					//	}
					//}
				}

				return ActivityFactory::CreateMove(m_actor.get(), path, ActivityFactory::CT_PLAYER_MOVE);
			}
			return 0;
		}


	private:
		EntityMoveManager(){}

		bool CanVehicleMove(VehicleEntity* vehicle) const
		{
			return      vehicle->GetInfo()->IsRobot()
				||  (       vehicle->GetInfo()->IsTech()
				&&  vehicle->GetEntityContext()->GetDriver());
		}

		bool CanHumanMove(HumanEntity* human) const { return true; }

		bool IsHumanOverloaded(HumanEntity* human) const
		{
			HumanContext* context = human->GetEntityContext();
			return context->GetTraits()->GetWeight() > context->GetLimits()->GetWeight();                  
		}

		bool IsNoDriver4Vehicle(VehicleEntity* vehicle) const
		{
			return      vehicle->GetInfo()->IsTech()
				&&  vehicle->GetEntityContext()->GetDriver() == 0;
		}
	};
}

//==============================================================

namespace GameFormDetail{

	class EntityRotateManager : public ActionManager
	{
		point3 m_dir;        
		BaseEntityRef m_actor;

	public:
		static EntityRotateManager* GetInst()
		{
			static EntityRotateManager imp;
			return &imp;
		}

		void Init(BaseEntity* actor, const point3& dir)
		{
			m_dir = dir;
			m_actor.reset(actor);
		}

		Activity* CreateRotate()
		{ 
			if(m_actor.valid())
			{
				return ActivityFactory::CreateRotate(
					m_actor.get(),
					Dir2Angle(m_dir),
					AIUtils::CalcMps4EntityRotate(m_actor.get(),m_dir),
					ActivityFactory::CT_PLAYER_ROTATE);
			}

			return 0;
		}

		bool NeedRotate() const { return m_actor.valid(); }

	private:
		EntityRotateManager() {}
	};
}

//==============================================================

namespace GameFormDetail{

	class EntityShootManager : public ActionManager
	{
		BaseEntityRef m_entity;
		BaseEntityRef m_victim;

		rid_t  m_object;

		point3 m_to;
		float  m_accuracy;

	public:
		static EntityShootManager* GetInst()
		{
			static EntityShootManager imp;
			return &imp;
		}

		void Init(BaseEntity* entity, float accuracy, const point3& to, eid_t victim, const rid_t& obj)
		{
			m_entity.reset(entity);
			m_victim.reset(victim ? EntityPool::GetInst()->Get(victim) : 0);

			m_to = to;
			m_accuracy = accuracy;
			m_object = obj;

			m_entity->Notify(EntityObserver::EV_ATTACK_COMMAND);
		}

		Activity* CreateAction()
		{
			if(m_entity.valid())
			{
				if(!m_object.empty())
				{
					return ActivityFactory::CreateShoot(m_entity.get(),
						//m_to,
						m_accuracy, 
						ActivityFactory::CT_PLAYER_SHOOT,
						ActivityFactory::shoot_info_s(m_object));
				}
				else if(m_victim.valid())
				{
					return ActivityFactory::CreateShoot(m_entity.get(),
						//m_to,
						m_accuracy,
						ActivityFactory::CT_PLAYER_SHOOT,
						ActivityFactory::shoot_info_s(m_victim->GetEID()));
				}
				else
				{
					return ActivityFactory::CreateShoot(m_entity.get(),
						//m_to,
						m_accuracy,
						ActivityFactory::CT_PLAYER_SHOOT,
						ActivityFactory::shoot_info_s(m_to));
				}
			}
			return 0;
		}

	private:
		EntityShootManager() {}
	};
}

//==============================================================

namespace GameFormDetail {

	class ThingThrowManager : public ActionManager
	{
		point3 m_to;
		HumanEntityRef m_human;

	public:
		static ThingThrowManager* GetInst()
		{
			static ThingThrowManager imp;
			return &imp;
		}

		void Init(HumanEntity* human, const point3& pnt)
		{
			m_to = pnt;
			m_human.reset(human);
		}

		Activity* CreateAction()
		{
			if(m_human.valid())
			{
				const int acc = AIUtils::CalcGrenadeAccuracy(m_human.get(), (m_human->GetGraph()->GetPos3() - m_to).Length());
				const float accuracy = static_cast<float>(acc) * 1e-2f;
				return ActivityFactory::CreateThrow(m_human.get(), ActivityFactory::CT_PLAYER_GRENADE_THROW,
					ActivityFactory::shoot_info_s(m_to),accuracy);
			}
			return 0;
		}

	private:
		ThingThrowManager(){}
	};
};

//==============================================================

namespace GameFormDetail{

	//
	// пустой обработчик ввода
	//
	class EmptyInputManager : public InputManager {
	public:

		void HandleInput(){}
	};
}

//==============================================================

namespace GameFormDetail{

	//
	// обычный обработчик ввода
	//
	class UsualInputManager : public InputManager
	{
	public:
		void HandleInput()
		{
			GameFormImp* game_form = GetGameFormImp();

			GraphEntity::ShowDecor(GraphEntity::DT_FOS, (GetGameFormImp()->CanShowFOS() || Input::KeyState(DIK_L)) != 0);
			GraphEntity::ShowDecor(GraphEntity::DT_BANNER, Input::KeyState(DIK_TAB) != 0);

			if((Input::KeyBack(DIK_RETURN) || Input::KeyBack(DIK_NUMPADENTER))
				&& Screens::Instance()->Game()->Child("endturn_hot")->IsEnable())
			{
				GetGameFormImp()->HandleEndOfTurnReq();    
				return;
			}

			if(Input::KeyBack(DIK_J)) game_form->HandleShowJournalReq();

			if(Input::KeyBack(DIK_HOME)) SwitchRoofs();

			if(Input::KeyBack(DIK_ADD))
				Screens::Instance()->Game()->MsgWindow()->IncMsgWindow();

			if(Input::KeyBack(DIK_SUBTRACT))
				Screens::Instance()->Game()->MsgWindow()->DecMsgWindow();

			if(Input::KeyBack(DIK_M)) game_form->HandleShowOptionsReq();

			if(Input::KeyBack(DIK_B)) game_form->HandleSwitchHandsMode();

			if(Input::KeyBack(DIK_LBRACKET)) game_form->HandlePrevFastAccessThing();

			if(Input::KeyBack(DIK_RBRACKET)) game_form->HandleNextFastAccessThing();

			if(Input::KeyBack(PoL::GetContinueKey()) && game_form->GetCurEnt())
			{
				if(ActionManager* action = game_form->GetWalkers()->CreateAction(game_form->GetCurEnt()))
					game_form->HandlePlayActionReq(action);
			}

			int index = GetPressedIndex();

			if(index >= 0)
			{
				if(Input::KeyState(DIK_LCONTROL) || Input::KeyState(DIK_RCONTROL))
				{
					if(BaseEntity* entity = game_form->GetEnemyIcons()->Icon2Entity(index))
					{
						Cameraman::GetInst()->FocusEntity(entity);
					}
				}
				else if(Input::KeyState(DIK_RALT) || Input::KeyState(DIK_LALT))
				{
					if(VehicleEntity* vehicle = game_form->GetRobotIcons()->Icon2Entity(index))
					{
						if(game_form->GetCurEnt() == vehicle)
						{
							Cameraman::GetInst()->FocusEntity(game_form->GetCurEnt());
						}
						else
						{
							game_form->HandleSelectReq(vehicle, GameFormImp::SEL_COMMAND);
						}
					}
				}
				else if(index < MAX_TEAMMATESTOTAL)
				{
					if(HumanEntity* human = game_form->GetHumanIcons()->Icon2Entity(index))
					{
						if(game_form->GetCurEnt() == human)
						{
							Cameraman::GetInst()->FocusEntity(game_form->GetCurEnt());
						}
						else
						{
							game_form->HandleSelectReq(human, GameFormImp::SEL_COMMAND);
						}
					}
				}
			}

			if(Input::KeyBack(DIK_ESCAPE))
			{
				if(game_form->CanMakeHack())
					game_form->HandleSwitchHackReq();
				else if(game_form->CanAttack())
					game_form->HandleSwitchAttackReq();
				else if(game_form->CanMakeUse())
					game_form->HandleSwitchUseReq();
				else if(game_form->GetCurEnt())
					game_form->HandleSelectReq(0);
				else
					game_form->HandleShowOptionsReq();
			}

			if(game_form->GetCurEnt() && Input::KeyBack(DIK_SPACE))
				Cameraman::GetInst()->FocusEntity(game_form->GetCurEnt());            

			if(game_form->GetCurEnt() && game_form->GetCurEnt()->Cast2Human() && Input::KeyBack(DIK_I))
				game_form->HandleShowInventoryReq(game_form->GetCurEnt()->Cast2Human(), UsualGround::GetInst(), GameFormHeroScroller::GetInst(), GroundBin::GetInst());

			if(game_form->GetCurEnt() && Input::KeyBack(PoL::GetAimShotKey()))
			{
				game_form->HandleSwitch2Aimshot();
			}

			if(game_form->GetCurEnt() && Input::KeyBack(PoL::GetSnapShotKey()))
			{
				game_form->HandleSwitch2Snapshot();
			}

			if(game_form->GetCurEnt() && Input::KeyBack(PoL::GetAutoShotKey()))
			{
				game_form->HandleSwitch2Autoshot();
			}

			if(game_form->GetCurEnt() && Input::KeyBack(PoL::GetCrouchKey()))
			{
				if(IsChecked(GameScreen::CB_SIT))
					game_form->HandleSwitch2Stand();    
				else
					game_form->HandleSwitch2Sit();
			}

			if(game_form->GetCurEnt() && Input::KeyBack(DIK_R))
			{
				if(IsChecked(GameScreen::CB_RUN))
					game_form->HandleSwitch2Stand();    
				else
					game_form->HandleSwitch2Run();
			}

			if(game_form->GetCurEnt() && Input::KeyBack(DIK_W))
				game_form->HandleSwitch2Stand();    

			if(Input::KeyBack(DIK_N))
			{
				BaseEntity *current = game_form->GetCurEnt();
				GameFormHeroScroller *scroller = GameFormHeroScroller::GetInst();

				//на все про все два шага
				int iterations = 2;

				while(iterations--)
				{
					//если никто не выделен, выделим человека
					if(current == 0)
					{                        
						scroller->First();                        

						if(!scroller->IsEnd())
						{
							current = scroller->Get();
							break;
						}
					}

					//если выделен человек, выделим след человека
					if(current && current->Cast2Human())
					{
						scroller->First(current->Cast2Human());                        
						scroller->Next();

						if(current = scroller->IsEnd() ? 0 : scroller->Get())
							break;
					}

					//если человек на выделяется выделим первую технику
					if(current == 0 && (current = FindNextEntity(game_form->GetTechIcons(),static_cast<VehicleEntity*>(0))))
						break;

					//если выделена технику выделим след. технику
					if(current && current->Cast2Vehicle() && (current = FindNextEntity(game_form->GetTechIcons(), current->Cast2Vehicle())))
						break;
				}

				if(current)
				{
					game_form->HandleSelectReq(current);
					Cameraman::GetInst()->FocusEntity(current);
					current->Notify(EntityObserver::EV_SELECT_COMMAND);
				}
			}

			if(Input::KeyBack(DIK_E)) game_form->HandleReloadWeapon();

			if(Input::KeyBack(DIK_F5)) DirtyLinks::MakeQuickSaveLoad(GenericStorage::SM_SAVE);
			if(Input::KeyBack(DIK_F9)) DirtyLinks::MakeQuickSaveLoad(GenericStorage::SM_LOAD);

			//перемотка оружия в колесиком мышки
			if(Input::MouseState().dz && Screens::Instance()->Game()->IsCursorOnWeaponIcon())
			{
				if(Input::MouseState().dz > 0)
					game_form->HandleNextFastAccessThing();
				else
					game_form->HandlePrevFastAccessThing();
			}

			//~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~

#ifdef _HOME_VERSION

			if(game_form->GetCurEnt() && Input::KeyBack(DIK_Z)){

				if(HumanEntity* human = game_form->GetCurEnt()->Cast2Human()){
					HumanContext::Traits* traits = human->GetEntityContext()->GetTraits();
					traits->AddMovepnts(traits->GetMovepnts() + 1);
				}else if(VehicleEntity* vehicle = game_form->GetCurEnt()->Cast2Vehicle()){
					VehicleContext* context = vehicle->GetEntityContext();
					context->SetMovepnts(context->GetMovepnts()*2+1);
				}
			}       

			if(game_form->GetCurEnt() && game_form->GetCurEnt()->Cast2Human() && Input::KeyBack(DIK_X)){

				EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_TRADER);

				if(itor != EntityPool::GetInst()->end()){  

					game_form->HandleShowShopReq(game_form->GetCurEnt()->Cast2Human(),
						itor->Cast2Trader(),
						UsualGround::GetInst(),
						GameFormHeroScroller::GetInst(),
						GroundBin::GetInst());                
					return;
				}            
			}

#endif            
			//~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~

			//обработка мышиного курсора
			game_form->GetCursor()->Update();       
		}
	};
}

//==============================================================

namespace GameFormDetail
{
	//
	// обработчик ввода для действия 
	//
	class ActivityInputManager : public InputManager
	{
	public:
		void HandleInput()
		{
			activity_command user_cmd = AC_TICK;

			if(IsLeftMouseButtonDoubleClick(GetGameFormImp()->GetCursor()->GetLastClickTime()))
			{
				user_cmd = AC_SPEED_UP;
			}
			else if(IsLeftMouseButtonClick())
			{
				user_cmd = AC_STOP;
			}

			GetGameFormImp()->RunActivity(user_cmd);
		}
	};
}

//==============================================================

namespace GameFormDetail
{
	//
	// класс установливющий иконку человека при выборе существа
	//
	class BodySelector : public EntityVisitor
	{
	public:

		void Visit(HumanEntity* human)
		{
			HumanContext* context = human->GetEntityContext();

			GameScreen::BodyIconTraits icon_traits;

			icon_traits.m_Damages = 0;
			if(context->HaveLongDamage(::DT_SHOCK)) icon_traits.m_Damages |= GameScreen::BodyIconTraits::DT_SHOCK;
			if(context->HaveLongDamage(::DT_FLAME)) icon_traits.m_Damages |= GameScreen::BodyIconTraits::DT_FLAME;
			if(context->HaveLongDamage(::DT_ELECTRIC)) icon_traits.m_Damages |= GameScreen::BodyIconTraits::DT_ELECTRIC;

			icon_traits.m_Body  = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_BODY));
			icon_traits.m_Hands = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_HANDS)); 
			icon_traits.m_Head  = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_HEAD));
			icon_traits.m_Legs  = Dmg2BodyPartState(context->GetBodyPartDamage(BPT_LEGS));

			Screens::Instance()->Game()->SetBodyIcon(icon_traits);
		}

	private:

		GameScreen::BodyIconTraits::BODY_PARTS_STATE Dmg2BodyPartState(int val)
		{
			const int m_low_dmg = 1;
			const int m_medium_dmg = 5;
			const int m_huge_dmg = 15;

			if( val <= m_low_dmg ) return GameScreen::BodyIconTraits::BPS_GREEN;
			if( val <= m_medium_dmg) return GameScreen::BodyIconTraits::BPS_BLUE;
			if( val <= m_huge_dmg) return GameScreen::BodyIconTraits::BPS_YELLOW;

			return GameScreen::BodyIconTraits::BPS_RED;
		}
	};

	//
	// класс устанавливающий иконку с оружием
	//
	class WeaponSelector : public EntityVisitor{
	private:

		class ShotTypeMpsAdapter : public GameScreen::ShootMovePoints{
		public:

			ShotTypeMpsAdapter() { Init(); }

			ShotTypeMpsAdapter(HumanEntity* human, WeaponThing* thing)
			{
				const bool unlim = UnlimitedMoves();
				int hum_mps = human->GetEntityContext()->GetTraits()->GetMovepnts();

				m_aim_count = Int2Str(thing->GetMovepnts(SHT_AIMSHOT));
				m_AimColor  = Bool2Color(hum_mps >= thing->GetMovepnts(SHT_AIMSHOT) || unlim);

				m_snap_count = Int2Str(thing->GetMovepnts(SHT_SNAPSHOT));
				m_SnapColor  = Bool2Color(hum_mps >= thing->GetMovepnts(SHT_SNAPSHOT) || unlim);

				if(thing->GetInfo()->IsAuto())
				{
					m_auto_count = Int2Str(thing->GetMovepnts(SHT_AUTOSHOT));
					m_AutoColor  = Bool2Color(hum_mps >= thing->GetMovepnts(SHT_AUTOSHOT) || unlim);
				}

				Init();
			}

			ShotTypeMpsAdapter(HumanEntity* human, BaseThing* thing)
			{
				const bool unlim = UnlimitedMoves();
				int thing_mps = AIUtils::CalcMps2Act(thing);
				int hum_mps   = human->GetEntityContext()->GetTraits()->GetMovepnts();

				m_aim_count = Int2Str(thing_mps);
				m_AimColor  = Bool2Color(hum_mps >= thing_mps || unlim);

				Init();
			}

			ShotTypeMpsAdapter(VehicleEntity* vehicle)
			{
				if(vehicle->GetInfo()->GetAmmoCount())
				{
					const bool unlim = UnlimitedMoves();
					int mps4sht = vehicle->GetInfo()->GetMp4Shot();
					int veh_mps = vehicle->GetEntityContext()->GetMovepnts();

					m_aim_count = Int2Str(mps4sht);
					m_AimColor  = Bool2Color(veh_mps >= mps4sht || unlim);                    
				}

				Init();
			}

		private:

			void Init()
			{
				m_AimMP = m_aim_count.c_str();
				m_SnapMP = m_snap_count.c_str();
				m_AutoMP = m_auto_count.c_str();
			}

			unsigned Bool2Color(bool flag) { return flag ? 0x00ff00 : 0xff0000; }

			std::string Int2Str(int value)
			{
				std::ostringstream ostr;
				ostr << value;
				return ostr.str();
			}

		private:

			std::string m_aim_count;
			std::string m_snap_count;
			std::string m_auto_count;
		};

		class WeaponIconAdapter : public GameScreen::WeaponIconTraits{
		public:

			WeaponIconAdapter(BaseThing* thing)
			{
				Init();
				SetThing(thing);
			}

			WeaponIconAdapter(WeaponThing* thing)
			{
				Init(); 
				SetThing(thing);

				if(AmmoThing* ammo = thing->GetAmmo())
				{
					SetCount(ammo->GetBulletCount());
					SetInitials(ammo->GetInfo()->GetInitials());
				}
			}

			WeaponIconAdapter(MedikitThing* med)
			{
				Init(); 
				SetThing(med);
				SetCount(med->GetCharge());
			}

			WeaponIconAdapter(VehicleEntity* vehicle)
			{
				Init();

				if(vehicle->GetInfo()->GetAmmoCount())
				{
					SetShader(vehicle->GetInfo()->GetWeaponIcon());
					SetInitials(vehicle->GetInfo()->GetAmmoInfo());
					SetCount(vehicle->GetEntityContext()->GetAmmoCount());
				}
			}

		private:

			void SetThing(BaseThing* thing)
			{
				SetShader(thing->GetInfo()->GetShader());
				SetInitials(thing->GetInfo()->GetInitials());                
			}

			void SetShader(const std::string& shader)
			{
				m_icon_name = shader;
				m_ImageName = m_icon_name.c_str();
			}

			void SetInitials(const std::string& initials)
			{
				m_ammo_info = initials;
				m_AmmoDesc = m_ammo_info.c_str();
			}

			void SetCount(int count)
			{
				std::ostringstream ostr;

				ostr << count;

				m_ammo_count = ostr.str();
				m_AmmoQuantity = m_ammo_count.c_str();
			}

			void Init()
			{
				m_AmmoDesc = m_ammo_info.c_str();
				m_ImageName = m_icon_name.c_str();
				m_AmmoQuantity = m_ammo_count.c_str();                
			}

		private:

			std::string m_icon_name;
			std::string m_ammo_info;
			std::string m_ammo_count;
		};

	public:

		void Visit(HumanEntity* human)
		{
			//точность стрельбы
			if(BaseThing* thing = human->GetEntityContext()->GetThingInHands()){

				//установим картинку оружия
				if(thing->GetInfo()->IsWeapon())
					SetThing(human, static_cast<WeaponThing*>(thing));
				else if(thing->GetInfo()->IsMedikit())
					SetThing(human, static_cast<MedikitThing*>(thing));
				else if(thing->GetInfo()->IsGrenade())
					SetThing(human, static_cast<GrenadeThing*>(thing));
				else if(thing->GetInfo()->IsCamera())
					SetThing(human, static_cast<CameraThing*>(thing));
				else if(thing->GetInfo()->IsShield())
					SetThing(human, static_cast<ShieldThing*>(thing));
				else if(thing->GetInfo()->IsScanner())
					SetThing(human, static_cast<ScannerThing*>(thing));
				else
					SetNullThing();

				return;
			}

			SetNullThing();
		}

		void Visit(VehicleEntity* vehicle)
		{
			GameScreen* screen = Screens::Instance()->Game();

			DisableShotTypeButtons();

			WeaponIconAdapter traits(vehicle);
			screen->SetWeaponIcon(&traits);

			ShotTypeMpsAdapter shoot_traits(vehicle);
			screen->SetShootMovePoints(shoot_traits);
		}

	private:

		void SetThing(HumanEntity* human, WeaponThing* weapon)
		{
			GameScreen* screen = Screens::Instance()->Game();
			shot_type type = human->GetEntityContext()->GetShotType();

			//разрешить кнопки управления оружием
			screen->EnableCBState(GameScreen::CB_AIMSHOT, true);
			screen->EnableCBState(GameScreen::CB_SNAPSHOT, true);
			screen->EnableCBState(GameScreen::CB_AUTOSHOT, weapon->GetInfo()->IsAuto());

			//установить кнопки в соотв сост       
			screen->SetCBState(GameScreen::CB_AIMSHOT, type == SHT_AIMSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
			screen->SetCBState(GameScreen::CB_AUTOSHOT, type == SHT_AUTOSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
			screen->SetCBState(GameScreen::CB_SNAPSHOT, type == SHT_SNAPSHOT ? GameScreen::CBS_ON : GameScreen::CBS_OFF);

			WeaponIconAdapter thing_traits(weapon);
			screen->SetWeaponIcon(&thing_traits);

			ShotTypeMpsAdapter shoot_traits(human, weapon);
			screen->SetShootMovePoints(shoot_traits);
		}

		void SetThing(HumanEntity* human, BaseThing* thing)
		{
			DisableShotTypeButtons();

			GameScreen* screen = Screens::Instance()->Game();

			WeaponIconAdapter thing_traits(thing);
			screen->SetWeaponIcon(&thing_traits);

			ShotTypeMpsAdapter shoot_traits(human, thing);
			screen->SetShootMovePoints(shoot_traits);
		}

		void SetThing(HumanEntity* human, MedikitThing* medikit)
		{
			DisableShotTypeButtons();

			GameScreen* screen = Screens::Instance()->Game();

			WeaponIconAdapter thing_traits(medikit);
			screen->SetWeaponIcon(&thing_traits);

			ShotTypeMpsAdapter shoot_traits(human, medikit);
			screen->SetShootMovePoints(shoot_traits);
		}

		void SetNullThing()
		{
			DisableShotTypeButtons();

			GameScreen* screen = Screens::Instance()->Game();

			screen->SetShootMovePoints(ShotTypeMpsAdapter());
			screen->SetWeaponIcon(static_cast<GameScreen::WeaponIconTraits*>(0));
		}

		void DisableShotTypeButtons()
		{
			GameScreen* screen = Screens::Instance()->Game();

			screen->EnableCBState(GameScreen::CB_AIMSHOT, false);
			screen->EnableCBState(GameScreen::CB_SNAPSHOT, false);            
			screen->EnableCBState(GameScreen::CB_AUTOSHOT, false);
		}
	};

	//
	// класс для установки совтояния QuickSlot'a
	//
	class QuickSlotSelector : public EntityVisitor{
	public:

		void Visit(HumanEntity* human)
		{
			HumanContext* context = human->GetEntityContext();

			if(!context->GetFastAccessStrategy()->CanMakeFastAccess()){
				DisableQuickSlotButtons();                
				return;
			}

			GameScreen* screen  = Screens::Instance()->Game();

			HumanContext::hands_mode mode = context->GetHandsMode();

			//установить установить кнопки быстрого доступа к предмету
			screen->EnableCBState(GameScreen::CB_BACKPACK, true);
			screen->EnableButtonState(GameScreen::B_UP_ARROW, mode == HumanContext::HM_FAST_ACCESS);
			screen->EnableButtonState(GameScreen::B_DOWN_ARROW, mode == HumanContext::HM_FAST_ACCESS);
			screen->SetCBState(GameScreen::CB_BACKPACK, mode == HumanContext::HM_HANDS ? GameScreen::CBS_OFF : GameScreen::CBS_ON);
		}

		void Visit(TraderEntity* trader)
		{ DisableQuickSlotButtons(); }

		void Visit(VehicleEntity* vehicle)
		{ DisableQuickSlotButtons(); }

	private:

		void DisableQuickSlotButtons()
		{
			GameScreen* screen = Screens::Instance()->Game();

			screen->EnableCBState(GameScreen::CB_BACKPACK, false);
			screen->EnableButtonState(GameScreen::B_UP_ARROW, false);
			screen->EnableButtonState(GameScreen::B_DOWN_ARROW, false);
		}
	};

	//
	// класс для установки состояния кнопки перезарядки
	//
	class ReloadButtonSelector : public EntityVisitor{
	public:

		void Visit(HumanEntity* human)
		{
			Screens::Instance()->Game()->EnableButtonState(GameScreen::B_RELOAD, GetGameFormImp()->GetReloader()->CanReload(human));
		}

		void Visit(VehicleEntity* vehicle)
		{
			Screens::Instance()->Game()->EnableButtonState(GameScreen::B_RELOAD, false);
		}
	};

	//
	// класс для работы с movepnts при выборе существа
	//
	class MovepntsSelector : public EntityVisitor
	{
	public:

		void Visit(HumanEntity* human)
		{
			if(VehicleEntity* vehicle = human->GetEntityContext()->GetCrew())
			{
				AIUtils::CalcAndShowLandField(vehicle);
				return;
			}

			AIUtils::CalcAndShowPassField(human);
		}

		void Visit(VehicleEntity* vehicle)
		{
			AIUtils::CalcAndShowPassField(vehicle);
		}
	};

	//
	// кнопки для человека
	//
	class ButtonStateSelector : public EntityVisitor{
	public:

		void Visit(HumanEntity* human)
		{
			GameScreen* game_scr = Screens::Instance()->Game();

			HumanContext* context = human->GetEntityContext();
			GraphHuman*   graph   = human->GetGraph()->Cast2Human();

			game_scr->EnableCBState(GameScreen::CB_USE, true);
			game_scr->EnableCBState(GameScreen::CB_SPECIAL, true);
			game_scr->EnableCBState(GameScreen::CB_SIT, context->CanSit());
			game_scr->EnableCBState(GameScreen::CB_RUN, context->CanRun());            

			//отрисовать правильные кнопки
			game_scr->SetCBState(GameScreen::CB_SIT, graph->IsSitPose() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
			game_scr->SetCBState(GameScreen::CB_RUN, graph->IsRunMove() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
			game_scr->SetCBState(GameScreen::CB_USE, GetGameFormImp()->CanMakeUse() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);            
			game_scr->SetCBState(GameScreen::CB_SPECIAL, GetGameFormImp()->CanMakeHack() ? GameScreen::CBS_ON : GameScreen::CBS_OFF);

			/*Grom*/
			static bool rec=false; //защита от повторного входа
			if(!rec)
			{
				//if(!context->CanSit())
				//{
				//	rec=true;
				//	context->SetHandsMode(HumanContext::HM_HANDS);
				//	game_scr->EnableCBState(GameScreen::CB_BACKPACK, false);
				//	//game_scr->SetCBState(GameScreen::CB_BACKPACK, GameScreen::CBS_OFF);
				//	rec=false;
				//}
				//else
				{
					game_scr->EnableCBState(GameScreen::CB_BACKPACK, true);
					if(context->GetHandsMode()==HumanContext::HM_HANDS)
						game_scr->SetCBState(GameScreen::CB_BACKPACK, GameScreen::CBS_OFF);
					else
						game_scr->SetCBState(GameScreen::CB_BACKPACK, GameScreen::CBS_ON);
				}
			}


		}

		void Visit(VehicleEntity* vehicle)
		{
			GameScreen* game_scr = Screens::Instance()->Game();

			game_scr->EnableCBState(GameScreen::CB_SIT, false);
			game_scr->EnableCBState(GameScreen::CB_RUN, false);
			game_scr->EnableCBState(GameScreen::CB_USE, false);
			game_scr->EnableCBState(GameScreen::CB_SPECIAL, false);
		}
	};

	//
	// класс для выделение существа
	//
	class Selector : public EntityVisitor
	{
	public:
		void Visit(TraderEntity* trader)
		{
			throw CASUS("Selecter: попытка выделения торговца!!!");
		}

		void Visit(HumanEntity* human)
		{   
			HumanContext* context = human->GetEntityContext(); 

			context->Select(true);

			human->Accept(m_body_selector);
			human->Accept(m_button_selector);
			human->Accept(m_reload_selector);
			human->Accept(m_movepnts_selector);
			human->Accept(m_quick_slot_selector);

			GetGameFormImp()->GetEnemyIcons()->HandleSelectReq(human);

			//человек в технике?
			if(context->GetCrew())
			{
				context->Select(true);
				context->GetCrew()->GetEntityContext()->Select(true);
				return;            
			}

			human->Accept(m_weapon_selector); 
		}

		void Visit(VehicleEntity* vehicle)
		{
			VehicleContext* context = vehicle->GetEntityContext();

			vehicle->Accept(m_body_selector);
			vehicle->Accept(m_button_selector);
			vehicle->Accept(m_weapon_selector);           
			vehicle->Accept(m_reload_selector);
			vehicle->Accept(m_movepnts_selector);
			vehicle->Accept(m_quick_slot_selector);

			GetGameFormImp()->GetEnemyIcons()->HandleSelectReq(vehicle);

			context->Select(true);
		}

	private:

		BodySelector      m_body_selector;
		WeaponSelector    m_weapon_selector;
		MovepntsSelector  m_movepnts_selector;
		QuickSlotSelector m_quick_slot_selector;

		ReloadButtonSelector m_reload_selector;
		ButtonStateSelector  m_button_selector;
	};
}

//==============================================================

namespace GameFormDetail{

	class Deselector : public EntityVisitor{
	public:

		void Visit(HumanEntity* human)
		{
			HumanContext* contect = human->GetEntityContext();

			//развыделить существо
			if(contect->GetCrew()) contect->GetCrew()->GetEntityContext()->Select(false);

			contect->Select(false);

			//очистить человека
			GameScreen::BodyIconTraits icon_traits;

			icon_traits.m_Damages = 0;

			icon_traits.m_Body  = GameScreen::BodyIconTraits::BPS_NONE;
			icon_traits.m_Hands = GameScreen::BodyIconTraits::BPS_NONE;
			icon_traits.m_Head  = GameScreen::BodyIconTraits::BPS_NONE;
			icon_traits.m_Legs  = GameScreen::BodyIconTraits::BPS_NONE;

			Screens::Instance()->Game()->SetBodyIcon(icon_traits);

			CommonPart();
		}

		void Visit(VehicleEntity* vehicle)
		{
			vehicle->GetEntityContext()->Select(false);
			CommonPart();
		}

		void Visit(TraderEntity* trader)
		{
			throw CASUS("Deselecter: попытка развыделить торговца!!!");
		}

	private:

		void CommonPart()
		{
			AIUtils::HideLandField();
			AIUtils::HidePassField();

			GameScreen* game_scr = Screens::Instance()->Game();

			game_scr->SetWeaponIcon(static_cast<GameScreen::WeaponIconTraits*>(0));

			game_scr->EnableCBState(GameScreen::CB_BACKPACK, false);
			game_scr->EnableCBState(GameScreen::CB_AIMSHOT, false);
			game_scr->EnableCBState(GameScreen::CB_SNAPSHOT, false);
			game_scr->EnableCBState(GameScreen::CB_AUTOSHOT, false);
			game_scr->EnableCBState(GameScreen::CB_SPECIAL, false);
			game_scr->EnableCBState(GameScreen::CB_SIT,false);
			game_scr->EnableCBState(GameScreen::CB_RUN, false);
			game_scr->EnableCBState(GameScreen::CB_USE, false);

			game_scr->EnableButtonState(GameScreen::B_RELOAD, false);
			game_scr->EnableButtonState(GameScreen::B_UP_ARROW, false);
			game_scr->EnableButtonState(GameScreen::B_DOWN_ARROW, false);

			GameScreen::ShootMovePoints shoot_traits;

			shoot_traits.m_AimMP = shoot_traits.m_AutoMP = shoot_traits.m_SnapMP = "";
			game_scr->SetShootMovePoints(shoot_traits);

			GetGameFormImp()->GetEnemyIcons()->HandleSelectReq(0);
		}        
	};
}

//==============================================================

namespace GameFormDetail
{
	class HumanLandingManager : public ActionManager
	{
		ipnt2_t    m_hex;

		HumanEntityRef   m_human;
		//VehicleEntityRef m_vehicle;

	public:
		static HumanLandingManager* GetInst()
		{
			static HumanLandingManager imp;
			return &imp;
		}

		void Init(HumanEntity* hum, const ipnt2_t& hex_pnt)
		{
			m_hex = hex_pnt;
			m_human.reset(hum);
			//m_vehicle.reset(m_human->GetEntityContext()->GetCrew());
		}

		Activity* CreateAction()
		{ 
			if(m_human.valid())
			{
				Deselector desel;
				m_human->Accept(desel);

				return ActivityFactory::CreateLanding(m_human.get(), m_hex, ActivityFactory::CT_PLAYER_LANDING); 
			}
			return 0;
		}

	private:
		HumanLandingManager() {}
	};
}

//==============================================================

namespace GameFormDetail{

	class HumanShipmentManager : public Approach2EntityManager
	{
		HumanEntityRef m_human;
		VehicleEntityRef m_vehicle;

	public:
		static HumanShipmentManager* GetInst() 
		{
			static HumanShipmentManager imp;
			return &imp;
		}

		void Init(HumanEntity* hum, VehicleEntity* veh)
		{
			m_human.reset(hum);
			m_vehicle.reset(veh);

			Approach2EntityManager::Init(hum, veh, RT_ACTOR2ENTITY);
		}

		Activity* CreateAction()
		{
			if(m_human.valid() && m_vehicle.valid())
			{
				return ActivityFactory::CreateShipment(m_human.get(), m_vehicle.get(), ActivityFactory::CT_PLAYER_SHIPMENT);
			}
			else
			{
				return 0;
			}
		}

		void Shut()
		{   
			if(m_vehicle.valid())
			{
				if(m_vehicle->GetPlayer() == PT_PLAYER)
				{
					GetGameFormImp()->HandleSelectReq(m_vehicle.get());
					GetGameFormImp()->GetTechIcons()->Insert(m_vehicle.get());
				}
				else
				{
					GetGameFormImp()->HandleSelectReq(0);
				}
			}
		}

	private:
		HumanShipmentManager() {}
	};
}

#pragma warning(pop)

//==============================================================

namespace GameFormDetail{

	class NullCursorMgr : public CursorManager{
	public:

		void Execute() {}

		bool CanExecute(CursorDevice* device)
		{ 
			//SetCursorText("null cursor");
			return false;
		}
	};

	class MenuCursorMgr : public CursorManager{
	public:

		void Execute() {}

		bool CanExecute(CursorDevice* device)
		{ 
			CursorDevice::menu_part menu_part = CursorDevice::MP_NONE;

			HumanEntity* human = GetGameFormImp()->GetCurEnt() ? GetGameFormImp()->GetCurEnt()->Cast2Human() : 0;

			if(     human == 0 
				||  !GetGameFormImp()->CanAttack() 
				||  !device->GetMenuPart(&menu_part)
				||  menu_part != CursorDevice::MP_WEAPON)
				return false;

			//нужен курсор на лечение?
			if(human->GetEntityContext()->GetThingInHands(TT_MEDIKIT))
			{
				device->SetCursor(CursorDevice::CT_MEDIC);
				return true;
			}

			device->SetCursor(CursorDevice::CT_ATTACK_ENABLE);
			return true;
		}
	};

	class UseCursorMgr : public CursorManager
	{
	public:

		bool CanExecute(CursorDevice* device)
		{ 
			m_human = 0;
			m_ent4use = 0;
			m_object.clear();

			if(GetGameFormImp()->GetCurEnt()) m_human = GetGameFormImp()->GetCurEnt()->Cast2Human();

			if(m_human == 0)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(device->GetObject(&m_object))
			{
				return CanUseObject(m_object, device) && CanAccessObject(m_object, device);
			}

			if(m_ent4use = device->GetEntity()) return CanUseEntity(m_ent4use, device);

			device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
			return false;
		}

		void Execute()
		{
			GameFormImp* game_form = GetGameFormImp();
			if(game_form->CanMakeUse()) game_form->HandleSwitchUseReq();

			if(m_ent4use)
			{
				EntityUseManager::GetInst()->Init(m_human, m_ent4use);
				GetGameFormImp()->HandlePlayActionReq(EntityUseManager::GetInst());            
				return;
			}

			if(m_object.size())
			{
				ObjectUseManager::GetInst()->Init(m_human, m_object);
				GetGameFormImp()->HandlePlayActionReq(ObjectUseManager::GetInst());
				return;
			}
		}

	private:
		bool CanComeToObject(const PathUtils::near_pnt_t &pts) const
		{
			if(pts.IsDefPnt())
			{
				return false;
			}
			else if(!UnlimitedMoves())
			{
				const int needmps = pts.m_cost * m_human->GetEntityContext()->GetHexCost();
				return needmps <= m_human->GetEntityContext()->GetTraits()->GetMovepnts();
			}
			else
			{
				return true;
			}
		}

		bool HaveMovepointsToUse(const PathUtils::near_pnt_t &pts) const
		{
			if(!UnlimitedMoves())
			{
				const int needmps = pts.m_cost * m_human->GetEntityContext()->GetHexCost()
										   + PoL::Inv::MovepointsForUse;
				return needmps <= m_human->GetEntityContext()->GetTraits()->GetMovepnts();
			}
			return true;
		}

		bool CanAccessObject(const rid_t& obj, CursorDevice* device)
		{
			pnt_vec_t   obj_hexes;

			DirtyLinks::CalcObjectHexes(m_object, &obj_hexes);
			PathUtils::near_pnt_t near_pnt = PathUtils::GetNearPnt(obj_hexes);

			if(!CanComeToObject(near_pnt))
			{
				device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!HaveMovepointsToUse(near_pnt))
			{
				device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_mps"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			return true;
		}

		bool CanUseObject(const rid_t& obj_rid, CursorDevice* device)
		{
			std::string hint;

			if(DirtyLinks::CanUseObject(obj_rid, &hint))
			{
				device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes(hint)));
				return true;
			}

			if(BaseBox* box = Bureau::GetInst()->Get(obj_rid))
			{
				if(box->Cast2Info())
					device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_console")));
				else if(box->Cast2Store())
					device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_store")));

				return true;
			}

			return false;
		}

		bool CanUseEntity(BaseEntity* ent4use, CursorDevice* device)
		{
			if(m_human->GetEID() == ent4use->GetEID())
			{
				return false;
			}

			if(ent4use->Cast2Vehicle() && !ent4use->Cast2Vehicle()->GetInfo()->CanUse()) return false;

			if(!AIUtils::IsAccessibleEntity(m_human, ent4use))
			{
				device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			device->SetCursor(CursorDevice::CT_USE, CursorDevice::text(ent4use->GetInfo()->GetName()));
			return true;
		}

	private:

		std::string  m_object;
		HumanEntity* m_human;
		BaseEntity*  m_ent4use;
	};

	class MoveCursorMgr : public CursorManager, private EntityVisitor
	{
	public:

		bool CanExecute(CursorDevice* device)
		{ 
			m_device = device;
			m_entity = GetGameFormImp()->GetCurEnt();

			//предположим худшее
			m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE);

			//если нет текущего существа
			if(m_entity == 0)
			{
				m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			//если есть выход за пределы поля
			if(!m_device->GetLevelHex(&m_pnt))
			{
				m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_lvl"), CursorDevice::DENIAL_COLOR));
				return false;            
			}

			//если не проходимая зона
			if(HexGrid::GetInst()->Get(m_pnt).IsDefPnt())
			{
				m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
				return false;
			}

			//проверка специфичная для существа
			m_cost = 0;
			m_entity->Accept(*this);

			if(m_cost)
			{
				m_device->SetCursor(CursorDevice::CT_MOVE_ENABLE, CursorDevice::text(m_cost, m_cost > 0 ? CursorDevice::DEFAULT_COLOR : CursorDevice::DENIAL_COLOR));
				return true;
			}

			return false;
		}

		void Execute()
		{
			EntityMoveManager::GetInst()->Init(GetGameFormImp()->GetCurEnt(), m_pnt);
			GetGameFormImp()->HandlePlayActionReq(EntityMoveManager::GetInst());
		}

	private:

		void Visit(HumanEntity* human)
		{
			HumanContext* context = human->GetEntityContext();

			if(context->GetTraits()->GetWeight() > context->GetLimits()->GetWeight())
			{
				m_device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_weight_overload"),  CursorDevice::DENIAL_COLOR));
				return;
			}

			m_cost = HexGrid::GetInst()->Get(m_pnt).GetCost() * context->GetHexCost();            
			if(m_cost > context->GetTraits()->GetMovepnts() && !UnlimitedMoves())
			{
				m_cost = context->GetTraits()->GetMovepnts() - m_cost;
			}
		}

		void Visit(VehicleEntity* vehicle)
		{
			VehicleContext* context = vehicle->GetEntityContext();

			m_cost = HexGrid::GetInst()->Get(m_pnt).GetCost() * context->GetHexCost();
			if(m_cost > context->GetMovepnts() && !UnlimitedMoves())
			{
				m_cost = context->GetMovepnts() - m_cost;
			}
		}

	private:

		ipnt2_t m_pnt;
		int     m_cost;     

		BaseEntity*   m_entity;
		CursorDevice* m_device;
	};

	class SelectCursorMgr : public CursorManager
	{
	public:

		bool CanExecute(CursorDevice* device)
		{ 
			if((m_entity = device->GetEntity()) && m_entity->GetPlayer() == PT_PLAYER)
			{      
				device->SetCursor(CursorDevice::CT_SELECT, CursorDevice::text(m_entity->GetInfo()->GetName()));
				return true;
			}

			return false;
		}

		void Execute()
		{
			GetGameFormImp()->HandleSelectReq(m_entity, GameFormImp::SEL_COMMAND);
		}

	private:

		BaseEntity* m_entity;
	};

	class RotateCursorMgr : public CursorManager, private EntityVisitor
	{
	public:

		bool CanExecute(CursorDevice* device)
		{ 
			m_entity = GetGameFormImp()->GetCurEnt();

			if(m_entity == 0){      
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			point3 pnt;

			if(!device->GetTracePnt(&pnt))
				return false;

			m_dir    = pnt - m_entity->GetGraph()->GetPos3(); 
			int cost = AIUtils::CalcMps4EntityRotate(m_entity, m_dir);

			if(m_entity->Cast2Human() && m_entity->Cast2Human()->GetEntityContext()->GetCrew())
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_in_crew"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			//узнаем кол-во ходов существа
			m_entity->Accept(*this);

			//если не хватает на поворот
			if(cost > m_moves && !UnlimitedMoves())
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_mps"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			device->SetCursor(CursorDevice::CT_MOVE_ENABLE, CursorDevice::text(cost));
			return true;
		}

		void Execute()
		{
			EntityRotateManager::GetInst()->Init(m_entity, m_dir);
			GetGameFormImp()->HandlePlayActionReq(EntityRotateManager::GetInst());
		}

	private:

		void Visit(HumanEntity* human)
		{
			m_moves = human->GetEntityContext()->GetTraits()->GetMovepnts();
		}

		void Visit(VehicleEntity* vehicle)
		{
			m_moves = vehicle->GetEntityContext()->GetMovepnts();
		}

	private:

		int     m_moves;   
		point3  m_dir;

		BaseEntity*   m_entity;
	};

	class LandingCursorMgr : public CursorManager
	{
	public:
		bool CanExecute(CursorDevice* device)
		{ 
			m_human = 0;

			if(GetGameFormImp()->GetCurEnt()) m_human = GetGameFormImp()->GetCurEnt()->Cast2Human();

			if(m_human == 0)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(m_human->GetEntityContext()->GetCrew() == 0)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_not_in_crew"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!device->GetLevelHex(&m_pnt))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_lvl"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!HexGrid::GetInst()->Get(m_pnt).IsLandPnt())
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_land_zone"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!UnlimitedMoves() && m_human->GetEntityContext()->GetTraits()->GetMovepnts() < PoL::Inv::MovepointsForLanding)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_mps"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			device->SetCursor(CursorDevice::CT_UNLOAD);
			return true;
		}

		void Execute()
		{
			HumanLandingManager::GetInst()->Init(m_human, m_pnt);
			GetGameFormImp()->HandlePlayActionReq(HumanLandingManager::GetInst());
		}

	private:

		ipnt2_t      m_pnt;
		HumanEntity* m_human;
	};

	class ShipmentCursorMgr : public CursorManager
	{
	public:

		bool CanExecute(CursorDevice* device)
		{ 
			m_human = 0;
			m_vehicle = 0;

			if(GetGameFormImp()->GetCurEnt()) m_human = GetGameFormImp()->GetCurEnt()->Cast2Human();

			if(m_human == 0)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!(m_vehicle = device->GetEntity() ? device->GetEntity()->Cast2Vehicle() : 0 ))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_veh"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!AIUtils::IsAccessibleEntity(m_human, m_vehicle))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!m_vehicle->GetEntityContext()->CanJoinCrew(m_human))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_full_veh"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!UnlimitedMoves() && m_human->GetEntityContext()->GetTraits()->GetMovepnts() < PoL::Inv::MovepointsForShipment)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_mps"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			device->SetCursor(CursorDevice::CT_SHIPMENT, CursorDevice::text(m_vehicle->GetInfo()->GetName()));
			return true;
		}

		void Execute()
		{
			HumanShipmentManager::GetInst()->Init(m_human, m_vehicle);
			GetGameFormImp()->HandlePlayActionReq(HumanShipmentManager::GetInst());
		}

	private:

		HumanEntity*   m_human;
		VehicleEntity* m_vehicle;
	};

//=====================================================================================//
//                   class TreatmentCursorMgr : public CursorManager                   //
//=====================================================================================//
	class TreatmentCursorMgr : public CursorManager
	{
	public:

		bool CanExecute(CursorDevice* device)
		{ 
			m_sick = 0;
			m_doctor = 0;

			if(GetGameFormImp()->GetCurEnt()) m_doctor = GetGameFormImp()->GetCurEnt()->Cast2Human();

			if(m_doctor == 0)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			MedikitThing* medikit = static_cast<MedikitThing*>(m_doctor->GetEntityContext()->GetThingInHands(TT_MEDIKIT));
			if(medikit == 0)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_medikit"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(!(m_sick = device->GetEntity() ? device->GetEntity()->Cast2Human() : 0))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent_4_treat"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			if(m_sick->GetEID() != m_doctor->GetEID() && !AIUtils::IsAccessibleEntity(m_doctor, m_sick))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			HumanContext* sick_context   = m_sick->GetEntityContext();
			HumanContext* doctor_context = m_doctor->GetEntityContext();

			if(sick_context->GetTraits()->GetHealth() >= sick_context->GetLimits()->GetHealth()){
				std::string msg = mlprintf( DirtyLinks::GetStrRes("aiu_not_ill").c_str(), m_sick->GetInfo()->GetName().c_str());
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(msg, CursorDevice::DENIAL_COLOR));
				return false;
			}

			//int mps4treatment = PathUtils::GetNearPnt(m_sick).m_cost*doctor_context->GetHexCost() + medikit->GetInfo()->GetMp2Act();
			//Grom
			int mps4treatment =  medikit->GetInfo()->GetMp2Act();
			if(sick_context!=doctor_context)
				mps4treatment += (PathUtils::GetNearPnt(m_sick).m_cost-1)*doctor_context->GetHexCost();
			const bool unlim = UnlimitedMoves();


			if(doctor_context->GetTraits()->GetMovepnts() < mps4treatment && !unlim)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("aiu_no_mps_for_treat"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			device->SetCursor(CursorDevice::CT_MEDIC,
				CursorDevice::text(m_sick->GetInfo()->GetName()),
				CursorDevice::text(),
				CursorDevice::text(mps4treatment));

			return true;
		}

		void Execute()
		{
			TreatmentManager::GetInst()->Init(m_doctor, m_sick);
			GetGameFormImp()->HandlePlayActionReq(TreatmentManager::GetInst());
		}

	private:

		HumanEntity* m_sick;
		HumanEntity* m_doctor;
	};

//=====================================================================================//
//                     class SwapCursorMgr : public CursorManager                      //
//=====================================================================================//
	class SwapCursorMgr : public CursorManager
	{
		BaseEntity *m_target;
		HumanEntity *m_source;

	public:
		bool CanExecute(CursorDevice *device)
		{
			m_target = 0;
			m_source = 0;

			if(GetGameFormImp()->GetCurEnt())
			{
				m_source = GetGameFormImp()->GetCurEnt()->Cast2Human();
			}

			if(m_source == 0)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
				return false;
			}

			m_target = device->GetEntity();

			if(m_target == 0 || (m_target->Cast2Human() == 0 && m_target->Cast2Vehicle() == 0))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
				return false;
			}

			if(VehicleEntity *vehent = m_target->Cast2Vehicle())
			{
				if(!vehent->GetInfo()->IsTinySize())
				{
					device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
					return false;
				}

				if(!vehent->GetInfo()->CanSwap())
				{
					device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
					return false;
				}
			}
			else if(HumanEntity *hument = m_target->Cast2Human())
			{
				if(!hument->GetInfo()->CanSwap())
				{
					device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
					return false;
				}
			}

			if(m_target->GetEID() != m_source->GetEID() && !AIUtils::IsAccessibleEntity(m_source, m_target))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
				return false;
			}

			if(m_target->GetPlayer() == PT_PLAYER)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
				return false;
			}

			if(EnemyDetector::getInst()->isHeEnemy(m_target))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
				return false;
			}

			EntityContext *target_context = m_target->GetEntityContext();
			HumanContext *source_context = m_source->GetEntityContext();

			int mps4swap = PoL::Inv::MovepointsForSwap;

			if(target_context != source_context)
			{
				mps4swap += (PathUtils::GetNearPnt(m_target).m_cost-1) * source_context->GetHexCost();
			}

			const bool unlim = UnlimitedMoves();

			if(source_context->GetTraits()->GetMovepnts() < mps4swap && !unlim)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
				return false;
			}

			device->SetCursor(CursorDevice::CT_SWAP,
				CursorDevice::text(m_target->GetInfo()->GetName()),
				CursorDevice::text(),
				CursorDevice::text(mps4swap));

			return true;
		}

		void Execute()
		{
			SwapManager::GetInst()->Init(m_source, m_target);
			GetGameFormImp()->HandlePlayActionReq(SwapManager::GetInst());
		}
	};

	static std::ostringstream &init(std::ostringstream &s)
	{
		s.str(std::string());
		s.clear();
		return s;
	}

	static std::string relationToText(RelationType rt)
	{
		switch(rt)
		{
		case RT_NEUTRAL: return "neutral";
		case RT_ENEMY: return "enemy";
		case RT_FRIEND: return "friend";
		}

		return "";
	}

//=====================================================================================//
//                     class DebugCursorMgr : public CursorManager                     //
//=====================================================================================//
	class DebugCursorMgr : public CursorManager
	{
		BaseEntity *m_entity;

	public:
		bool CanExecute(CursorDevice *device)
		{
			int y = 120;
			int x = 150;

			m_entity = 0;

			BaseEntity *curent = GetGameFormImp()->GetCurEnt();
			m_entity = device->GetEntity();
			std::ostringstream sstr;

			if(m_entity)
			{
				sstr << "Entity: eid(" << m_entity->GetEID() << "); sysname("
					 << m_entity->GetInfo()->GetRID() << "); ";
				if(m_entity->Cast2Human())
				{
					sstr << "Human";
				}
				else if(m_entity->Cast2Vehicle())
				{
					sstr << "Vehicle";
				}
				else if(m_entity->Cast2Trader())
				{
					sstr << "Trader";
				}

				DebugInfo::Add(x,y,const_cast<char*>(sstr.str().c_str()));
				y += 20;

				init(sstr) << "team(";

				if(m_entity->GetPlayer() == PT_PLAYER)
				{
					sstr << "PLAYER)";
				}
				else if(m_entity->GetPlayer() == PT_ENEMY)
				{
					sstr << "ENEMY)";
					if(m_entity->GetAIContext())
					{
						sstr << "; subteam(" << m_entity->GetAIContext()->getSubteam() << ")";
					}
				}
				else if(m_entity->GetPlayer() == PT_CIVILIAN)
				{
					sstr << "CIVILIAN)";
					if(m_entity->GetAIContext())
					{
						sstr << "; subteam(" << m_entity->GetAIContext()->getSubteam() << ")";
					}
				}

				DebugInfo::Add(x,y,const_cast<char*>(sstr.str().c_str()));
				y += 20;

				init(sstr) << "Player relation is "
					<< relationToText(EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(m_entity));

				DebugInfo::Add(x,y,const_cast<char*>(sstr.str().c_str()));
				y += 20;
			}


			if(curent && m_entity)
			{
				init(sstr) << "Entity relation to current entity is ";
				if(EnemyDetector::getInst()->isHeEnemy4Me(m_entity,curent))
				{
					sstr << "enemy";
				}
				else
				{
					sstr << "not enemy";
				}
				DebugInfo::Add(x,y,const_cast<char*>(sstr.str().c_str()));
				y += 20;
			}

			device->SetCursor(CursorDevice::CT_MOVE_DISABLE);
			return false;
		}

		void Execute()
		{
		}
	};

//=====================================================================================//
//         class AttackCursorMgr : public CursorManager, private EntityVisitor         //
//=====================================================================================//
	class AttackCursorMgr : public CursorManager, private EntityVisitor
	{
	public:

		bool CanExecute(CursorDevice* device)
		{ 
			m_victim = 0;
			m_entity = 0;
			m_accuracy = 0;
			m_fexecute = false;
			m_object.clear();
			m_device = device;

			//поставить запрещающий курсор
			m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE);

			m_entity = GetGameFormImp()->GetCurEnt();
			if(m_entity == 0)
			{
				m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			device->GetTracePnt(&m_to);
			device->GetObject(&m_object);

			if(m_victim = device->GetEntity())
			{
				if(m_victim->GetEID() == m_entity->GetEID())
					return false;

				m_to = m_victim->GetGraph()->GetShotPoint(m_to);
			}

			//проверки специфич. для существа
			m_entity->Accept(*this);
			return m_fexecute;
		}

		void Execute()
		{
			GameFormImp* game_form = GetGameFormImp();

			if(game_form->CanAttack()) game_form->HandleSwitchAttackReq();

			if(HumanEntity* human = m_entity->Cast2Human())
			{
				//проверка: это бросок предмета?
				if(human->GetEntityContext()->GetThingInHands(TT_THROWABLE))
				{
					ThingThrowManager::GetInst()->Init(human, m_to);
					GetGameFormImp()->HandlePlayActionReq(ThingThrowManager::GetInst());        
					return;
				}
			}

			EntityShootManager::GetInst()->Init(m_entity, m_accuracy, m_to, m_victim ? m_victim->GetEID() : 0, m_object);
			GetGameFormImp()->HandlePlayActionReq(EntityShootManager::GetInst());        
		}

	private:

		void Visit(HumanEntity* human)
		{
			HumanContext* context = human->GetEntityContext();
			AIUtils::reason_type reason = AIUtils::RT_NONE;

			//проверка на оружие
			if(WeaponThing* weapon = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON)))
			{
				if(AIUtils::CanShoot(human, weapon, m_to, &reason))
				{
					m_fexecute = true;

					int accuracy = AIUtils::CalcShootAccuracy(human, weapon, m_to);                    
					accuracy = saturate(accuracy,1,99);

					SetShootCursor(accuracy, weapon->GetMovepnts(context->GetShotType()));

					m_accuracy = AIUtils::NormalizeAccuracy(m_entity, accuracy);
					return;
				}

				m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(AIUtils::Reason2Str(reason), CursorDevice::DENIAL_COLOR));
				return;
			}

			//проверка на бросаемый предмет
			if(BaseThing* thing = context->GetThingInHands(TT_THROWABLE))
			{
				if(AIUtils::CanThrow(human, thing, m_to, &reason))
				{
					m_fexecute = true;
					m_accuracy = AIUtils::CalcGrenadeAccuracy(human,(human->GetGraph()->GetPos3()-m_to).Length());
					SetShootCursor(m_accuracy, AIUtils::CalcMps2Act(thing));
					return;
				}

				m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(AIUtils::Reason2Str(reason), CursorDevice::DENIAL_COLOR));
				return;
			}

			//если в руках есть предмет вывести надпись что им нельзя атаковать
			if(BaseThing* thing = context->GetThingInHands())
			{
				std::string text = mlprintf(DirtyLinks::GetStrRes("gf_not_for_attack").c_str(), thing->GetInfo()->GetName().c_str());
				m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(text, CursorDevice::DENIAL_COLOR));
				return;
			}

			m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_weap"), CursorDevice::DENIAL_COLOR));
		}

		void Visit(VehicleEntity* vehicle)
		{
			AIUtils::reason_type reason = AIUtils::RT_NONE;

			if(AIUtils::CanShoot(vehicle, m_to, &reason))
			{
				m_fexecute = true;

				int accuracy = vehicle->GetInfo()->GetAccuracy();
				SetShootCursor(accuracy, vehicle->GetInfo()->GetMp4Shot());

				m_accuracy = AIUtils::NormalizeAccuracy(m_entity, accuracy);
				return;
			}

			m_device->SetCursor(CursorDevice::CT_ATTACK_DISABLE, CursorDevice::text(AIUtils::Reason2Str(reason), CursorDevice::DENIAL_COLOR));
		}

		void SetShootCursor(int accuracy, int movepnts)
		{
			m_device->SetCursor(CursorDevice::CT_ATTACK_ENABLE,
				CursorDevice::text(m_victim ? m_victim->GetInfo()->GetName() : ""),
				CursorDevice::text(accuracy),
				CursorDevice::text(movepnts),
				CursorDevice::text(AIUtils::CalcEfficiency(m_entity,m_to)));
		}

	private:

		point3 m_to;
		rid_t  m_object;
		float  m_accuracy;        

		bool   m_fexecute;

		BaseEntity* m_victim;
		BaseEntity* m_entity;

		CursorDevice* m_device;
	};

	class HackCursorMgr : public CursorManager{
	public:

		bool CanExecute(CursorDevice* device)
		{
			m_human = 0;
			m_vehicle = 0;
			m_probability = 0;

			BaseEntity* cur_ent = GetGameFormImp()->GetCurEnt();

			m_human = cur_ent ? cur_ent->Cast2Human() : 0;

			device->SetCursor(CursorDevice::CT_MOVE_DISABLE);

			if(m_human == 0){
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_hum"), CursorDevice::DENIAL_COLOR));                
				return false;
			}

			m_vehicle = device->GetEntity() ? device->GetEntity()->Cast2Vehicle() : 0;
			if(m_vehicle == 0 || !m_vehicle->GetInfo()->IsRobot())
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_ent_4_hack"), CursorDevice::DENIAL_COLOR));                
				return false;
			}

			if(m_vehicle->GetPlayer() == m_human->GetPlayer())
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_can_not_hack_teammate"), CursorDevice::DENIAL_COLOR));                
				return false;
			}

			if(!AIUtils::IsAccessibleEntity(m_human, m_vehicle))
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_out_of_rng"), CursorDevice::DENIAL_COLOR));
				return false;
			}

			HumanContext* human_context = m_human->GetEntityContext();
			int mps4hack = (PathUtils::GetNearPnt(m_vehicle).m_cost-1)*human_context->GetHexCost() + MPS_FOR_HACK;
			const bool unlim = UnlimitedMoves();

			if(human_context->GetTraits()->GetMovepnts() < mps4hack && !unlim)
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_mps"), CursorDevice::DENIAL_COLOR));                
				return false;
			}

			if(human_context->GetTraits()->GetWisdom() < m_vehicle->GetInfo()->GetWisdom())
			{
				device->SetCursor(CursorDevice::CT_MOVE_DISABLE, CursorDevice::text(DirtyLinks::GetStrRes("gf_no_wisdom_4_hack"), CursorDevice::DENIAL_COLOR));                
				return false;
			}

			m_probability = AIUtils::CalcHackProbability(m_human, m_vehicle);

			device->SetCursor(CursorDevice::CT_SPECIAL,
				CursorDevice::text(m_vehicle->GetInfo()->GetName()),
				CursorDevice::text(100.0f * m_probability),
				CursorDevice::text(mps4hack));

			return true;
		}

		void Execute()
		{
			GameFormImp* game_form = GetGameFormImp();
			if(game_form->CanMakeHack()) game_form->HandleSwitchHackReq();

			EntityHackManager::GetInst()->Init(m_human, m_vehicle, m_probability);
			GetGameFormImp()->HandlePlayActionReq(EntityHackManager::GetInst());        
		}

	private:

		float  m_probability;

		HumanEntity*   m_human;
		VehicleEntity* m_vehicle;
	};
}

//==============================================================

namespace GameFormDetail
{

//=====================================================================================//
//                   class UsualCursorAdviser : public CursorAdviser                   //
//=====================================================================================//
	class UsualCursorAdviser : public CursorAdviser
	{
	public:
		bool NeedUse()
		{ 
			return Input::KeyState(DIK_LALT)
				|| Input::KeyState(DIK_RALT)
				|| GetGameFormImp()->CanMakeUse();
		}

		bool NeedHack() 
		{
			return Input::KeyState(DIK_H) || GetGameFormImp()->CanMakeHack();
		}

		bool NeedAttack()
		{ 
			return Input::KeyState(DIK_LCONTROL)
				|| Input::KeyState(DIK_RCONTROL)
				|| GetGameFormImp()->CanAttack();
		}

		bool NeedRotate()
		{ 
			return Input::KeyState(DIK_LSHIFT) || Input::KeyState(DIK_RSHIFT);
		}

		bool NeedSwap()
		{
			return Input::KeyState(DIK_G) != 0;
		}

		bool NeedDebug()
		{
			return Input::KeyState(DIK_T) && Input::KeyState(DIK_LCONTROL);
		}
	};
}

//==============================================================

namespace GameFormDetail
{
//=====================================================================================//
//      class UsualCursorSelector : public CursorSelector, private EntityVisitor       //
//=====================================================================================//
	class UsualCursorSelector : public CursorSelector, private EntityVisitor
	{
	public:
		cursor_type Select(CursorDevice *device, CursorAdviser *adviser)
		{  
			//если курсор над интерфейсом
			if(device->GetMenuPart(0)) return CUR_MENU;

#ifdef _HOME_VERSION
			if(adviser->NeedDebug())
			{
				return CUR_DEBUG;
			}

			if(Input::KeyState(DIK_F) && Input::KeyState(DIK_LCONTROL))
			{
				EnemyDetector::getInst()->debugOutRelations();
			}
#endif

			BaseEntity* entity = GetGameFormImp()->GetCurEnt();

			//если в интерфейсе никто не выделен
			if(entity == 0) return CUR_SELECT;

			//итак существо есть => анализ для существа
			if(adviser->NeedRotate()) return CUR_ROTATE;

			m_result  = CUR_NULL;
			m_device  = device;
			m_adviser = adviser;

			entity->Accept(*this);
			return m_result;
		}

	private:

		void Visit(HumanEntity* human)
		{
			HumanContext* context = human->GetEntityContext();

			//если человек в технике курсор на высадку
			if(context->GetCrew())
			{
				BaseEntity* entity = m_device->GetEntity();

				if(entity && entity->GetPlayer() == PT_PLAYER)
				{
					m_result = CUR_SELECT;
				}
				else
				{
					m_result = CUR_LANDING;
				}

				return;
			}

			//обработка курсора атаки
			if(m_adviser->NeedAttack())
			{
				m_result = context->GetThingInHands(TT_MEDIKIT) 
					?   CUR_TREATMENT  //курсор лечения если в руках аптечка
					:   CUR_ATTACK;    //курсор атаки в любом другом случае            
				return;
			}

			if(m_adviser->NeedSwap())
			{
				m_result = CUR_SWAP;
				return;
			}

			//если нужен use их есть у нас
			if(m_adviser->NeedUse())
			{
				m_result = CUR_USE;
				return;
			}

			//если есть курсор взлома
			if(m_adviser->NeedHack())
			{
				m_result = CUR_HACK;
				return;
			}
			//если курсор указывает на существо
			if(BaseEntity* entity = m_device->GetEntity())
			{
				//если это мы и есть => выход
				if(entity->GetEID() == human->GetEID())
					return;

				//если враг - атакуем
				if(EnemyDetector::getInst()->isHeEnemy4Me(human, entity))
				{
					m_result = CUR_ATTACK;
					return;
				}

				if(VehicleEntity* vehicle = entity->Cast2Vehicle())
				{
					//если наш робот, мы можем его выделить
					if(vehicle->GetInfo()->IsRobot() && vehicle->GetPlayer() == PT_PLAYER)
					{
						m_result = CUR_SELECT;
						return;
					}

					//если наша или ничейная техника попробуем в нее залезть
					if(vehicle->GetInfo()->IsTech() && vehicle->GetPlayer() & (PT_NONE|PT_PLAYER|PT_CIVILIAN))
					{                        
						m_result = CUR_SHIPMENT;
						return;
					}
				}

				//если наш => выделение
				if(entity->GetPlayer() == PT_PLAYER)
				{
					m_result = CUR_SELECT;
					return;
				}

				//на всех остальныx USE
				m_result = CUR_USE;
				return;
			}

			//если курсор указывает на HexGrid
			if(m_device->GetLevelHex(0))
			{
				m_result = CUR_MOVE;
				return;
			}

			//если курсор указывает на объект
			std::string object;
			if(m_device->GetObject(&object))
			{
				m_result = DirtyLinks::IsElevatorObj(object) ? CUR_MOVE : CUR_USE;
				return;
			}
		}

		void Visit(VehicleEntity* vehicle)
		{
			if(m_adviser->NeedAttack())
			{
				m_result = CUR_ATTACK;
				return;
			}

			//если курсор указывет на сущетсво
			if(BaseEntity* entity = m_device->GetEntity())
			{

				//если это мы и есть выход
				if(entity->GetEID() == vehicle->GetEID())
					return;

				//если враг - атакуем
				if(EnemyDetector::getInst()->isHeEnemy4Me(vehicle, entity))
				{
					m_result = CUR_ATTACK;
					return;
				}

				//eсли наш - выделяем
				if(entity->GetPlayer() == PT_PLAYER)
				{
					m_result = CUR_SELECT;
					return;
				}

				return;
			}

			//если курсор указывает на поле
			if(m_device->GetLevelHex(0))
			{
				m_result = CUR_MOVE;
				return;
			}
		}

		void Visit(TraderEntity* trader)
		{
		}

	private:

		cursor_type    m_result;
		CursorDevice*  m_device;
		CursorAdviser* m_adviser;
	};
}

//==============================================================

namespace GameFormDetail
{

	//
	// реализация мышиного устройства
	//
	class CursorDeviceImp : public CursorDevice
	{
		static bool CheckWidget(Widget *w)
		{
			return w && w->IsVisible();
		}

	public:
		CursorDeviceImp() : m_flags(0), m_cursor(CT_NORMAL), m_entity(0), m_menu_part(MP_NONE)
		{
			//если курсор над интерфейсом выход
			if(Screens::Instance()->Game()->FromPoint(Input::MouseState().x, Input::MouseState().y)
				|| CheckWidget(Screens::Instance()->HasInventory())
				|| CheckWidget(Screens::Instance()->HasTrade())
				|| CheckWidget(Screens::Instance()->HasDialog())
				|| CheckWidget(Screens::Instance()->HasHelper())
				|| CheckWidget(Screens::Instance()->HasHeroInfo())
				|| CheckWidget(Screens::Instance()->HasJournal())
				|| CheckWidget(Screens::Instance()->HasOptions())
				|| CheckWidget(Screens::Instance()->HasDescription())
				|| CheckWidget(Screens::Instance()->HasFlagDesc())
				)
			{
				m_flags |= F_MENU_PART;
				if(Screens::Instance()->Game()->IsCursorOnWeaponIcon()) m_menu_part = MP_WEAPON;
				return;            
			}

			unsigned trace_flags =  ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_INVISIBLE_ENTS
				|ShotTracer::F_SKIP_SHIELDS//|ShotTracer::F_ENTITIES_AS_BOX
				|ShotTracer::F_SKIP_ENTITIES_WITH_CAMERA
				|ShotTracer::F_CURSOR;

			ShotTracer tracer(0, DirtyLinks::GetCamPos(), DirtyLinks::PickCam(Input::MouseState().x, Input::MouseState().y), 0, trace_flags);

			m_flags |= F_TRACE_PNT;
			m_trace_pnt = tracer.GetEnd();

			switch(tracer.GetMaterial())
			{
				case ShotTracer::MT_AIR:
				case ShotTracer::MT_SHIELD:
					break;

				case ShotTracer::MT_WALL:
					if(CalcLevelHex(m_trace_pnt, &m_hex_pnt))
						m_flags |= F_LEVEL_HEX;
					break;

				case ShotTracer::MT_ENTITY:
					m_flags |= F_ENTITY;
					m_entity = EntityPool::GetInst()->Get(tracer.GetEntity());
					break;

				case ShotTracer::MT_OBJECT:
					m_flags |= F_OBJECT;
					m_object = tracer.GetObject();

					if(     DirtyLinks::IsElevatorObj(m_object)
						&&  CalcLevelHex(m_trace_pnt, &m_hex_pnt))
						m_flags |= F_LEVEL_HEX;
					break;
			}
		}

		~CursorDeviceImp()
		{
			MouseCursor::CURSOR_TYPE type = MouseCursor::CT_NORMAL;

			switch(m_cursor)
			{
				case CT_USE: type = MouseCursor::CT_CANUSE; break; 
				case CT_MEDIC: type = MouseCursor::CT_MEDIC; break; 
				case CT_SELECT: type = MouseCursor::CT_SELECT; break;
				case CT_UNLOAD: type = MouseCursor::CT_UNLOAD; break;
				case CT_NORMAL: type = MouseCursor::CT_NORMAL; break;
				case CT_PICKUP: type = MouseCursor::CT_PICKUP; break;
				case CT_SPECIAL: type = MouseCursor::CT_SPECIAL; break;
				case CT_SHIPMENT: type = MouseCursor::CT_SHIPMENT; break;
				case CT_MOVE_ENABLE: type = MouseCursor::CT_CANMOVE; break;
				case CT_MOVE_DISABLE: type = MouseCursor::CT_CANNOTMOVE; break;
				case CT_ATTACK_ENABLE: type = MouseCursor::CT_CANATTACK; break;
				case CT_ATTACK_DISABLE: type = MouseCursor::CT_CANNOTATTACK; break;
				case CT_SWAP: type = MouseCursor::CT_SWAP; break;
			}

			TxtFldAdapter txt[4] = {m_text[0],m_text[1],m_text[2],m_text[3]};

			if(UnlimitedMoves())
			{
				switch(m_cursor)
				{
				case CT_SWAP: txt[2].m_Text = "--"; break;
				case CT_MOVE_ENABLE: txt[0].m_Text = "--"; txt[1].m_Text = "--"; txt[2].m_Text = "--"; break;
				case CT_MOVE_DISABLE: txt[0].m_Text = "--"; txt[1].m_Text = "--"; txt[2].m_Text = "--"; break;
				case CT_ATTACK_ENABLE: txt[2].m_Text = "--"; break;
				case CT_ATTACK_DISABLE: txt[2].m_Text = "--"; break;
				}
			}

			MouseCursor::SetCursor(type,
				m_text[0].IsEmpty() ? 0 : &txt[0],
				m_text[1].IsEmpty() ? 0 : &txt[1],
				m_text[2].IsEmpty() ? 0 : &txt[2],
				m_text[3].IsEmpty() ? 0 : &txt[3]
				);
		}

		BaseEntity* GetEntity() const
		{ 
			return m_flags & F_ENTITY ? m_entity : 0; 
		}

		bool GetObject(rid_t* str) const
		{
			if(m_flags & F_OBJECT)
			{
				if(str) *str = m_object;
				return true;
			}

			return false;
		}

		bool GetTracePnt(point3* pnt) const 
		{
			if(m_flags & F_TRACE_PNT)
			{
				if(pnt) *pnt = m_trace_pnt;
				return true;
			}

			return false;
		}

		bool GetLevelHex(ipnt2_t* pnt) const
		{
			if(m_flags & F_LEVEL_HEX)
			{
				if(pnt) *pnt = m_hex_pnt;
				return true;
			}

			return false;
		}

		bool GetMenuPart(menu_part* part) const
		{
			if(m_flags & F_MENU_PART)
			{
				if(part) *part = m_menu_part;
				return true;
			}

			return false;
		}

		void SetCursor(cursor_type type, const text &t1, const text &t2, const text &t3, const text &t4)
		{
			m_cursor = type;

			m_text[0].Adapt(t1);
			m_text[1].Adapt(t2);
			m_text[2].Adapt(t3);
			m_text[3].Adapt(t4);
		}

	private:

		bool CalcLevelHex(const point3& pnt3, ipnt2_t* pnt2)
		{
			ipnt2_t ret;

			if(!HexGrid::GetInst()->IsOutOfRange(ret = HexUtils::scr2hex(pnt3))){
				*pnt2 = ret;
				return true;
			}

			return false;
		}

	private:

		struct TxtFldAdapter : public MouseCursor::TextField
		{
			bool IsEmpty() const { return m_text.empty(); }

			void Adapt(const text& text)
			{
				m_Color = text.m_color;
				m_text = text.m_text;
				m_Text = m_text.c_str();
			}

		private:

			std::string m_text;
		};

		TxtFldAdapter m_text[4];   
		cursor_type   m_cursor;

		rid_t       m_object;
		BaseEntity* m_entity;
		ipnt2_t     m_hex_pnt;
		menu_part   m_menu_part;
		point3      m_trace_pnt;

		enum flag_type
		{
			F_OBJECT    = 1 << 0,
			F_ENTITY    = 1 << 1,
			F_LEVEL_HEX = 1 << 2,
			F_MENU_PART = 1 << 3,
			F_TRACE_PNT = 1 << 4,
		};

		unsigned m_flags;
	};
}

//==============================================================

//=====================================================================================//
//                             SmartCursor::SmartCursor()                              //
//=====================================================================================//
SmartCursor::SmartCursor()
:	m_requests(0),
	m_current(0),
	m_upd_time(0),
	m_click_time(0),
	m_adviser(new UsualCursorAdviser()),
	m_selector(new UsualCursorSelector())
{
	m_cursors[CursorSelector::CUR_USE].reset(new UseCursorMgr());
	m_cursors[CursorSelector::CUR_HACK].reset(new HackCursorMgr());
	m_cursors[CursorSelector::CUR_NULL].reset(new NullCursorMgr());
	m_cursors[CursorSelector::CUR_MOVE].reset(new MoveCursorMgr());
	m_cursors[CursorSelector::CUR_MENU].reset(new MenuCursorMgr());
	m_cursors[CursorSelector::CUR_SELECT].reset(new SelectCursorMgr());
	m_cursors[CursorSelector::CUR_ROTATE].reset(new RotateCursorMgr());
	m_cursors[CursorSelector::CUR_ATTACK].reset(new AttackCursorMgr());
	m_cursors[CursorSelector::CUR_LANDING].reset(new LandingCursorMgr());
	m_cursors[CursorSelector::CUR_SHIPMENT].reset(new ShipmentCursorMgr());
	m_cursors[CursorSelector::CUR_TREATMENT].reset(new TreatmentCursorMgr());
	m_cursors[CursorSelector::CUR_SWAP].reset(new SwapCursorMgr());
	m_cursors[CursorSelector::CUR_DEBUG].reset(new DebugCursorMgr());
}

//=====================================================================================//
//                             SmartCursor::~SmartCursor()                             //
//=====================================================================================//
SmartCursor::~SmartCursor()
{
	delete m_adviser;
	delete m_selector;
}

//=====================================================================================//
//                            void SmartCursor::SetUsual()                             //
//=====================================================================================//
void SmartCursor::SetUsual()
{
	MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
}

//=====================================================================================//
//                             void SmartCursor::Update()                              //
//=====================================================================================//
void SmartCursor::Update()
{    
	//    if(m_requests/* || m_upd_time < Timer::GetSeconds()*/)
	{
		m_current  = 0;
		m_requests = 0;
		//        m_upd_time = Timer::GetSeconds() + DirtyLinks::GetFloatOpt(DirtyLinks::OT_MOUSE_UPDATE_TIME);

		CursorDeviceImp cursor_device;

		m_current = m_cursors[m_selector->Select(&cursor_device, m_adviser)].get();
		if(!m_current->CanExecute(&cursor_device)) m_current = 0;
	}

	//нажатие левой кнопки мыши
	if(m_current && IsLeftMouseButtonClick())
	{
		//запомним время последнего click'а
		m_click_time = static_cast<unsigned int>(Timer::GetSeconds() * 1e+3f);
		//выполним комманду курсора
		m_current->Execute();
	}
}

//==============================================================

//=====================================================================================//
//                        void Walkers::info_s::MakeSaveLoad()                         //
//=====================================================================================//
void Walkers::info_s::MakeSaveLoad(SavSlot& slot)
{
	if(slot.IsSaving())
	{
		if(m_entity)
		{
			slot << m_entity->GetEID();
			slot << m_point.x << m_point.y;
		}
		else
		{
			slot << static_cast<eid_t>(0);
		}
	}
	else
	{
		eid_t eid;
		slot >> eid;

		if(eid)
		{
			m_entity = EntityPool::GetInst()->Get(eid);
			slot >> m_point.x >> m_point.y;
		}
	}
}

//=====================================================================================//
//                        bool Walkers::info_s::IsSame() const                         //
//=====================================================================================//
bool Walkers::info_s::IsSame(BaseEntity* entity) const
{
	return m_entity && m_entity->GetEID() == entity->GetEID();
}

//=====================================================================================//
//                       bool Walkers::info_s::NeedMove() const                        //
//=====================================================================================//
bool Walkers::info_s::NeedMove() const
{
	return m_entity && m_point != m_entity->GetGraph()->GetPos2(); 
}

//=====================================================================================//
//                                 Walkers::Walkers()                                  //
//=====================================================================================//
Walkers::Walkers()
{
	Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
}

//=====================================================================================//
//                                 Walkers::~Walkers()                                 //
//=====================================================================================//
Walkers::~Walkers()
{
	Spawner::GetInst()->Detach(this);
	for(int k = 0; k < MAX_TEAMMATESTOTAL; Remove(m_infos[k++]));
}

//=====================================================================================//
//                           int Walkers::Ent2Index() const                            //
//=====================================================================================//
int Walkers::Ent2Index(BaseEntity* entity) const
{
	for(int k = 0; k < MAX_TEAMMATESTOTAL; k++)
	{
		if(m_infos[k].IsSame(entity))
			return k;
	}

	return -1;
}

//=====================================================================================//
//                            bool Walkers::IsGoing() const                            //
//=====================================================================================//
bool Walkers::IsGoing(BaseEntity* ent) const
{
	return Ent2Index(ent) >= 0;
}

//=====================================================================================//
//                               void Walkers::Insert()                                //
//=====================================================================================//
void Walkers::Insert(BaseEntity* entity, const ipnt2_t& to)
{
	int index = Ent2Index(entity);

	//попробуем найти такое же существо и подправить точку
	if(index >= 0)
	{
		m_infos[index].m_point = to;
		return;
	}

	//вставим это сущетво в списки
	for(int k = 0; k < MAX_TEAMMATESTOTAL; k++)
	{
		if(m_infos[k].IsEmpty())
		{
			m_infos[k].m_point  = to;
			m_infos[k].m_entity = entity;

			//поставить наблюдателя
			LinkObserver(entity);

			entity->Notify(EntityObserver::EV_INGOING_CHANGE);
			return;
		}
	}
}

//=====================================================================================//
//                            void Walkers::LinkObserver()                             //
//=====================================================================================//
void Walkers::LinkObserver(BaseEntity* entity)
{
	if(entity == 0) return;

	entity->Attach(this, EV_DESTROY);
	entity->Attach(this, EV_ACTIVITY_INFO);
}

//=====================================================================================//
//                               void Walkers::Update()                                //
//=====================================================================================//
void Walkers::Update(Spawner* spawn, SpawnObserver::event_t event, SpawnObserver::info_t info)
{
	for(int k = 0; k < MAX_TEAMMATESTOTAL; Remove(m_infos[k++]));
}

//=====================================================================================//
//                               void Walkers::Update()                                //
//=====================================================================================//
void Walkers::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
	//сообщения о конце действия на не интересны
	if(event == EV_ACTIVITY_INFO && static_cast<activity_info_s*>(info)->IsEnd())
		return;

	int index = Ent2Index(entity);

	if(index < 0) throw CASUS("Walkers: работа с существом за которым не наблюдаем!");

	Remove(m_infos[index]);    
}

//=====================================================================================//
//                               void Walkers::Remove()                                //
//=====================================================================================//
void Walkers::Remove(info_s& info)
{
	if(BaseEntity* entity = info.m_entity)
	{
		info.m_entity = 0;

		entity->Detach(this);    
		entity->Notify(EntityObserver::EV_INGOING_CHANGE);
	}
}

//=====================================================================================//
//                            void Walkers::MakeSaveLoad()                             //
//=====================================================================================//
void Walkers::MakeSaveLoad(SavSlot& slot)
{
	int old_limit = 6;

#ifdef _HOME_VERSION
	if(CmdLine::IsKey("-save206")) old_limit = 5;
#endif

	if(slot.GetStore()->GetVersion() < 10)
	{
		for(int k = 0; k < old_limit; m_infos[k++].MakeSaveLoad(slot));
		for(int k = old_limit; k < MAX_TEAMMATESTOTAL; ++k) m_infos[k].m_entity = 0;
	}
	else
	{
		for(int k = 0; k < MAX_TEAMMATESTOTAL; m_infos[k++].MakeSaveLoad(slot));
	}

	if(!slot.IsSaving())
	{
		//восстановить наблюдателей
		for(int k = 0; k < MAX_TEAMMATESTOTAL; LinkObserver(m_infos[k++].m_entity));
	}  
}

//=====================================================================================//
//                       ActionManager* Walkers::CreateAction()                        //
//=====================================================================================//
ActionManager* Walkers::CreateAction(BaseEntity* entity)
{
	if(entity)
	{
		int index = Ent2Index(entity);
		return index >= 0 ? CreateAction(m_infos[index]) : 0;
	}

	for(int k = 0; k < MAX_TEAMMATESTOTAL; k++)
	{
		if(m_infos[k].IsEmpty())
			continue;

		if(ActionManager* action = CreateAction(m_infos[k]))
			return action;        
	}

	return 0;
}

//=====================================================================================//
//                       ActionManager* Walkers::CreateAction()                        //
//=====================================================================================//
ActionManager* Walkers::CreateAction(info_s& info)
{
	if(HumanEntity* human = info.m_entity->Cast2Human())
	{
		if(human->GetEntityContext()->GetStepsCount() == 0)
			return 0;    
	}

	if(VehicleEntity* vehicle = info.m_entity->Cast2Vehicle())
	{
		if(vehicle->GetEntityContext()->GetStepsCount() == 0)
			return 0;    
	}

	//инициализируем менеджер перемещения
	EntityMoveManager::GetInst()->Init(info.m_entity, info.m_point, EntityMoveManager::F_WALKERS_MOVE);

	//удаляемся из списков
	Remove(info);

	//идем
	return EntityMoveManager::GetInst();
}

//==============================================================

namespace GameFormDetail
{

	GameScreen::PersonIconMgr* GetPersonIcons()
	{
		return Screens::Instance()->Game()->GetPersonIconMgr();
	}
}

typedef GameScreen::PersonIconMgr::Icon human_icon_t;
typedef GameScreen::PersonIconMgr::MonitorTraits hi_prefix;

HumanIcons::HumanIcons()
{
	GetPersonIcons()->RemoveAll();

	Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
	Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
}

HumanIcons::~HumanIcons()
{
	Spawner::GetInst()->Detach(this);

	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);

	while(itor != EntityPool::GetInst()->end()){
		itor->Detach(this);
		++itor;
	}
}

HumanEntity* HumanIcons::Icon2Entity(int index)
{
	human_icon_t* icon = GetPersonIcons()->IconAt(index);
	return icon ? icon->GetHumanEntity() : 0;
}

void HumanIcons::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
	switch(type){
	case ET_PREPARE_SPAWN:
		GetPersonIcons()->RemoveAll();
		break;

	case ET_ENTITY_SPAWN:
		{

			HumanEntity* human =    static_cast<spawn_info_s*>(info)->m_entity
				?   static_cast<spawn_info_s*>(info)->m_entity->Cast2Human()
				:   0;

			if(human && human->GetPlayer() == PT_PLAYER) Insert(human, human->GetInfo()->IsHacker());
		}
		break;
	}
}

void HumanIcons::Insert(HumanEntity* human, bool insert2front)
{
	//если вставка в начало списка
	if(insert2front)
	{
		typedef std::list<HumanEntity*> human_list_t;
		human_list_t hlst;

		hlst.push_back(human);

		for(int k = 0; k < MAX_TEAMMATESTOTAL; k++)
		{
			if(HumanEntity* hum = Icon2Entity(k))
				hlst.push_back(hum);
		}

		//чистим список
		GetPersonIcons()->RemoveAll();

		//вставляем в список все по новой
		human_list_t::iterator itor = hlst.begin();
		while(itor != hlst.end()) Insert(*(itor++), false);     

		return;
	}

	human_icon_t  icon(human);
	HumanContext* context = human->GetEntityContext();

	//зарегистировать наблюдателя
	LinkObserver(human);

	//установим свойства иконки
	icon.SetStateFlags(GetHumanFlags(human));
	icon.SetHealth(context->GetTraits()->GetHealth());
	icon.SetMovePoints(context->GetTraits()->GetMovepnts());
	icon.SetMaxHealth(context->GetLimits()->GetHealth());
	icon.SetMaxMovePoints(context->GetLimits()->GetMovepnts());
	icon.SetHeartbeat(static_cast<hi_prefix::HEARTBEAT>(GetHumanHeartbeat(human)));
	icon.SetPortrait(human->GetInfo()->GetRID().c_str(), context->IsSelected() ? hi_prefix::ST_NORMAL : hi_prefix::ST_NONE);

	//вставим иконку
	GetPersonIcons()->InsertIcon(icon);
}

int HumanIcons::GetHumanHeartbeat(HumanEntity* human)
{
	HumanContext* context = human->GetEntityContext();

	if(context->GetTraits()->GetHealth() < m_dead_lvl)
		return hi_prefix::HB_DEAD;

	if(static_cast<float>(context->GetTraits()->GetHealth())/static_cast<float>(context->GetLimits()->GetHealth()) < m_wound_lvl)
		return hi_prefix::HB_WOUND;

	return hi_prefix::HB_CALM;
}

unsigned HumanIcons::GetHumanFlags(HumanEntity* human)
{
	HumanContext* context = human->GetEntityContext();

	unsigned flags = 0;

	if(context->GetCrew()) flags |= hi_prefix::IF_INVEHICLE;
	if(context->GetPanicType() != HPT_NONE) flags |= hi_prefix::IF_INPANIC;
	if(context->GetTraits()->GetLevelupPoints()) flags |= hi_prefix::IF_LEVELUP;
	//if(GetGameFormImp()->GetWalkers()->IsGoing(human) && !UnlimitedMoves()) flags |= hi_prefix::IF_INMOVEMENT;

	return flags;
}

void HumanIcons::LinkObserver(HumanEntity* human)
{
	human->Attach(this, EV_QUIT_TEAM);
	human->Attach(this, EV_CREW_CHANGE);
	human->Attach(this, EV_PANIC_CHANGE);
	human->Attach(this, EV_PREPARE_DEATH);
	human->Attach(this, EV_HEALTH_CHANGE);
	human->Attach(this, EV_MAXHEALTH_CHANGE);
	human->Attach(this, EV_MAXMOVEPNTS_CHANGE);
	human->Attach(this, EV_INGOING_CHANGE);
	human->Attach(this, EV_LEVELUP_CHANGE);
	human->Attach(this, EV_MOVEPNTS_CHANGE);
	human->Attach(this, EV_SELECTION_CHANGE);
}

void HumanIcons::Remove(HumanEntity* human, remove_type type)
{
	human->Detach(this);

	GameScreen::PersonIconMgr::REMOVE_TYPE rt =     type == RT_DEATH
		?   GameScreen::PersonIconMgr::RT_SLOW
		:   GameScreen::PersonIconMgr::RT_QUICK;

	human_icon_t* icon = GetPersonIcons()->IconAt(human);
	if(icon && type == RT_DEATH) icon->SetPortrait(human->GetInfo()->GetRID().c_str(), hi_prefix::ST_DEAD);

	GetPersonIcons()->RemoveIcon(human, rt);
}

void HumanIcons::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
	HumanEntity* human = entity->Cast2Human();

	switch(event){
	case EV_QUIT_TEAM:
		if(human->GetPlayer() != PT_PLAYER) Remove(human);
		break;

	case EV_CREW_CHANGE:
	case EV_PANIC_CHANGE:
	case EV_INGOING_CHANGE:
	case EV_LEVELUP_CHANGE:
		if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
			icon->SetStateFlags(GetHumanFlags(human));
		break;

	case EV_PREPARE_DEATH:
		Remove(human, RT_DEATH);
		break;

	case EV_HEALTH_CHANGE:
		if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
		{
			icon->SetHealth(human->GetEntityContext()->GetTraits()->GetHealth());        
			icon->SetHeartbeat(static_cast<hi_prefix::HEARTBEAT>(GetHumanHeartbeat(human)));
		}
		break;

	case EV_MAXHEALTH_CHANGE:
		if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
		{
			icon->SetMaxHealth(human->GetEntityContext()->GetLimits()->GetHealth());
		}
		break;

	case EV_MOVEPNTS_CHANGE:
		if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
			icon->SetMovePoints(human->GetEntityContext()->GetTraits()->GetMovepnts());
		break;

	case EV_MAXMOVEPNTS_CHANGE:
		if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
			icon->SetMaxMovePoints(human->GetEntityContext()->GetLimits()->GetMovepnts());
		break;

	case EV_SELECTION_CHANGE:
		if(human_icon_t* icon = GetPersonIcons()->IconAt(human))
		{
			hi_prefix::SEL_TYPE selection =     human->GetEntityContext()->IsSelected()
				?   hi_prefix::ST_NORMAL : hi_prefix::ST_NONE;        

			icon->SetPortrait(human->GetInfo()->GetRID().c_str(), selection);
		}
		break;
	}
}

void HumanIcons::MakeSaveLoad(SavSlot& slot)
{
	if(slot.IsSaving()) return;

	GetPersonIcons()->RemoveAll();

	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);

	while(itor != EntityPool::GetInst()->end()){
		Insert(itor->Cast2Human(), itor->Cast2Human()->GetInfo()->IsHacker());
		++itor;
	}   
}

int HumanIcons::Entity2Icon(HumanEntity* human)
{
	HumanIcons* icons = GetGameFormImp()->GetHumanIcons();

	for(int k = 0; k < MAX_TEAMMATESTOTAL && human; k++)
	{
		if(human == icons->Icon2Entity(k))
			return k;
	}

	return -1;
}

#undef hi_prefix

//==============================================================

TechIcons::TechIcons()
{
	Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
	Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
}

TechIcons::~TechIcons()
{
	Spawner::GetInst()->Detach(this);

	for(int k = 0; k < MAX_TECHINTEAM; k++)
	{
		if(m_icons[k].m_vehicle) m_icons[k].m_vehicle->Detach(this);
	}    
}

void TechIcons::Update()
{
	for(int k = 0; k < MAX_TECHINTEAM; k++)
	{
		if(m_icons[k].m_requests)
		{
			DrawIcon(k, m_icons[k].m_vehicle);
			m_icons[k].m_requests = 0;
		}
	}
}

void TechIcons::Insert(VehicleEntity* vehicle)
{
	//найдем первый свободный слот и вставим технику
	for(int k = 0; k < MAX_TECHINTEAM; k++)
	{
		if(!m_icons[k].m_vehicle)
		{
			//пропишемся в массив иконок
			m_icons[k].m_vehicle  = vehicle;
			m_icons[k].m_requests++;

			//установим наблюдателя
			LinkObserver(vehicle);
			break;
		}

		if(m_icons[k].m_vehicle->GetEID() == vehicle->GetEID())
		{
			m_icons[k].m_requests++;
			return;
		}
	}
}

void TechIcons::LinkObserver(BaseEntity* vehicle)
{
	vehicle->Attach(this, EV_QUIT_TEAM);
	vehicle->Attach(this, EV_CREW_CHANGE);
	vehicle->Attach(this, EV_DEATH_PLAYED);
	vehicle->Attach(this, EV_HEALTH_CHANGE);
	vehicle->Attach(this, EV_INGOING_CHANGE);
	vehicle->Attach(this, EV_MOVEPNTS_CHANGE);
	vehicle->Attach(this, EV_SELECTION_CHANGE);   
}

void TechIcons::Remove(VehicleEntity* vehicle)
{
	//удалим технику
	for(int k = 0; k < MAX_TECHINTEAM; k++)
	{
		if(m_icons[k].m_vehicle && m_icons[k].m_vehicle->GetEID() == vehicle->GetEID())
		{
			//снимем набюдателя
			vehicle->Detach(this);

			//передвинем все вверх
			for(int z = k+1; z < MAX_TECHINTEAM; z++)
			{
				m_icons[z-1] = m_icons[z];
				m_icons[z-1].m_requests++;
			}

			m_icons[MAX_TECHINTEAM - 1].m_vehicle  = 0;
			m_icons[MAX_TECHINTEAM - 1].m_requests ++;
			break;
		}
	}
}

VehicleEntity* TechIcons::Icon2Entity(unsigned icon)
{
	if(icon >= MAX_TECHINTEAM) return 0;
	return m_icons[icon].m_vehicle;
}

int TechIcons::Entity2Icon(VehicleEntity* vehicle)
{
	for(int k = 0; k < MAX_TECHINTEAM; k++)
	{
		if(     m_icons[k].m_vehicle
			&&  m_icons[k].m_vehicle->GetEID() == vehicle->GetEID())
			return k;
	}

	return -1;
}

void TechIcons::DrawIcon(int icon_id, VehicleEntity* vehicle)
{
	//если все нормально - выход
	GameScreen::VehicleIconTraits icon_traits;

	icon_traits.m_Visible = vehicle != 0;

	if(vehicle){

		VehicleContext* context  = vehicle->GetEntityContext();

		icon_traits.m_HitPoints  = context->GetHealth();
		icon_traits.m_MaxCrew    = vehicle->GetInfo()->GetCapacity();
		icon_traits.m_NumCrew    = context->GetCrewSize();
		icon_traits.m_MovePoints = context->GetMovepnts();
		icon_traits.m_Selection  = context->IsSelected() ? GameScreen::VehicleIconTraits::ST_NORMAL : GameScreen::VehicleIconTraits::ST_NONE;

		icon_traits.m_Flags      = 0; 
		if(GetGameFormImp()->GetWalkers()->IsGoing(vehicle)) icon_traits.m_Flags |= GameScreen::VehicleIconTraits::VIF_INMOVEMENT;
	}

	Screens::Instance()->Game()->SetVehicleIcon(icon_id, icon_traits);
}

void TechIcons::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
	switch(type)
	{
	case ET_PREPARE_SPAWN:
		{
			for(int k = 0; k < MAX_TECHINTEAM; k++)
			{
				m_icons[k].m_vehicle  = 0;
				m_icons[k].m_requests ++;
			}
		}
		break;

	case ET_ENTITY_SPAWN:
		{
			VehicleEntity* vehicle =    static_cast<spawn_info_s*>(info)->m_entity
				?  static_cast<spawn_info_s*>(info)->m_entity->Cast2Vehicle()
				:  0;

			if(     vehicle
				&&  vehicle->GetInfo()->IsTech()
				&&  vehicle->GetPlayer() == PT_PLAYER)
				Insert(vehicle);
		}
		break;
	}

}

void TechIcons::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
	switch(event)
	{
	case EV_QUIT_TEAM:
		if(entity->GetPlayer() != PT_PLAYER)
			Remove(entity->Cast2Vehicle());
		break;

	case EV_DEATH_PLAYED:
		Remove(entity->Cast2Vehicle());
		break;

	default:
		{
			int icon = Entity2Icon(entity->Cast2Vehicle());
			if(icon >= 0) m_icons[icon].m_requests++;
		}
	};
}

void TechIcons::MakeSaveLoad(SavSlot& slot)
{
	if(slot.IsSaving())
	{
		for(int k = 0; k < MAX_TECHINTEAM; k++)
		{
			slot << (m_icons[k].m_vehicle ? m_icons[k].m_vehicle->GetEID() : 0);
		}
	}
	else
	{
		eid_t eid;
		int old_limit = 6;
#ifdef _HOME_VERSION
		if(CmdLine::IsKey("-save206")) old_limit = 5;
#endif

		for(int k = 0; k < MAX_TECHINTEAM; k++)
		{
			eid = 0;
			if(slot.GetStore()->GetVersion() >= 10 || k < old_limit)
			{
				slot >> eid;
			}

			if(eid)
			{
				m_icons[k].m_vehicle = EntityPool::GetInst()->Get(eid)->Cast2Vehicle();
				LinkObserver(m_icons[k].m_vehicle);
			}

			m_icons[k].m_requests++;
		}
	}
}

//==============================================================

//=====================================================================================//
//                              EnemyIcons::EnemyIcons()                               //
//=====================================================================================//
EnemyIcons::EnemyIcons()
{
	AttachObservers();

	//заполним массив иконок с врагами
	Screens::Instance()->Game()->GetEnemyIconMgr()->Clear();

	VisMap::PlainMarkIterator_t i = VisMap::GetInst()->PlainMarkBegin();
	for(; i != VisMap::GetInst()->PlainMarkEnd(); ++i)
	{
		if((*i)->IsVisibleForPlayer(PT_PLAYER1)) Insert(*i);
	}
}

//=====================================================================================//
//                              EnemyIcons::~EnemyIcons()                              //
//=====================================================================================//
EnemyIcons::~EnemyIcons()
{
	DetachObservers();
}

//=====================================================================================//
//                         void EnemyIcons::AttachObservers()                          //
//=====================================================================================//
void EnemyIcons::AttachObservers()
{
	//предотвращаем двойное подключение
	DetachObservers();

	Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);

	//присоеденимся к наблюдателю за командой
	Spectator* spectator = PlayerSpectator::GetInst()->GetSpectator();

	spectator->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
	spectator->AttachSpectatorObserver(this, EV_INVISIBLE_MARKER);    

	EnemyDetector::getInst()->attach(this, ET_RELATION2PLAYER_CHANGED);
}

//=====================================================================================//
//                         void EnemyIcons::DetachObservers()                          //
//=====================================================================================//
void EnemyIcons::DetachObservers()
{
	Spawner::GetInst()->Detach(this);
	EnemyDetector::getInst()->detach(this);
	PlayerSpectator::GetInst()->GetSpectator()->DetachSpectatorObserver(this);

	if(m_current.valid()) m_current->GetEntityContext()->GetSpectator()->DetachSpectatorObserver(this);
}

//=====================================================================================//
//                         void EnemyIcons::HandleSelectReq()                          //
//=====================================================================================//
void EnemyIcons::HandleSelectReq(BaseEntity* entity)
{
	if(m_current.valid()) m_current->GetEntityContext()->GetSpectator()->DetachSpectatorObserver(this);

	//сбросим старые флаги
	Screens::Instance()->Game()->GetEnemyIconMgr()->SetFlags(0);

	m_current.reset(entity);
	if(m_current.valid())
	{
		Spectator* spectator = m_current->GetEntityContext()->GetSpectator();

		spectator->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
		spectator->AttachSpectatorObserver(this, EV_INVISIBLE_MARKER);

		//установим новые
		VisMap::MarkIterator_t itor = VisMap::GetInst()->MarkBegin(entity->GetEntityContext()->GetSpectator(), VisParams::vpVisible);            

		for(; itor != VisMap::GetInst()->MarkEnd(); ++itor)
		{
			SetFlags(m_current.get(), &*itor);
		}            
	}
}

//=====================================================================================//
//                              void EnemyIcons::Update()                              //
//=====================================================================================//
void EnemyIcons::Update(Spawner* spawner, SpawnObserver::event_t event, SpawnObserver::info_t info)
{
	AttachObservers();
	Screens::Instance()->Game()->GetEnemyIconMgr()->Clear();
}

//=====================================================================================//
//                              void EnemyIcons::Update()                              //
//=====================================================================================//
void EnemyIcons::Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
{
	BaseEntity* entity = spectator->GetEntity();
	Marker* marker = static_cast<marker_info*>(info)->m_marker;

	if(entity && m_current.valid() && entity->GetEID() == m_current->GetEID())
	{
		SetFlags(entity, marker);        
	}
	else
	{
		if(event == EV_VISIBLE_MARKER)
		{
			Insert(marker);
		}
		else if(event == EV_INVISIBLE_MARKER)
		{
			Remove(marker);
		}
	}
}

//=====================================================================================//
//                             void EnemyIcons::changed()                              //
//=====================================================================================//
void EnemyIcons::changed(RelationsObserver::event_type et, void* info)
{
	BaseEntity* entity = EntityPool::GetInst()->Get(static_cast<EntityDesc*>(info)->m_id);

	VisParams flags = VisMap::GetInst()->GetVisFlags(
		PlayerSpectator::GetInst()->GetSpectator(),
		entity->GetEntityContext()->GetMarker());

	if(flags.IsVisible())
	{
		if(IsEnemy4Player(entity))
			Insert(entity->GetEntityContext()->GetMarker());
		else
			Remove(entity->GetEntityContext()->GetMarker());
	}
}

//=====================================================================================//
//                              void EnemyIcons::Insert()                              //
//=====================================================================================//
void EnemyIcons::Insert(Marker* marker)
{
	if(Screens::Instance()->HasGame())
	{
		BaseEntity* entity = marker->GetEntity();
		GameScreen::EnemyIconMgr* icon_mgr = Screens::Instance()->Game()->GetEnemyIconMgr();

		if(entity && icon_mgr->ItemAt(marker) == 0 && IsEnemy4Player(entity))
		{
			icon_mgr->Insert(GameScreen::EnemyIconMgr::EnemyIcon(entity->GetInfo()->GetName(), marker));
			SetFlags(m_current.get(), marker);
		}
	}
}

//=====================================================================================//
//                              void EnemyIcons::Remove()                              //
//=====================================================================================//
void EnemyIcons::Remove(Marker* marker)
{
	BaseEntity *entity = marker->GetEntity();
	Screens::Instance()->Game()->GetEnemyIconMgr()->Remove(marker);
}

//=====================================================================================//
//                             void EnemyIcons::SetFlags()                             //
//=====================================================================================//
void EnemyIcons::SetFlags(BaseEntity* entity, Marker* marker)
{
	VisParams visibility;
	if(entity) visibility = VisMap::GetInst()->GetVisFlags(entity->GetEntityContext()->GetSpectator(), marker) ;

	unsigned int flags = 0;
	if(visibility.IsVisible())
	{
		flags |= GameScreen::EnemyIconMgr::EnemyIcon::F_SELECTED;
	}

	GameScreen::EnemyIconMgr::EnemyIcon* icon = 0; 
	GameScreen::EnemyIconMgr* icon_mgr = Screens::Instance()->Game()->GetEnemyIconMgr();

	if(icon = icon_mgr->ItemAt(marker)) icon->SetFlags(flags);
}

//=====================================================================================//
//                        BaseEntity* EnemyIcons::Icon2Entity()                        //
//=====================================================================================//
BaseEntity* EnemyIcons::Icon2Entity(unsigned index)
{
	GameScreen::EnemyIconMgr::EnemyIcon* icon = 0;

	if(icon = Screens::Instance()->Game()->GetEnemyIconMgr()->ItemAt(index))
		if(icon->GetMarker()) return icon->GetMarker()->GetEntity();           

	return 0;
}

//=====================================================================================//
//                           void EnemyIcons::MakeSaveLoad()                           //
//=====================================================================================//
void EnemyIcons::MakeSaveLoad(SavSlot& slot)
{
	GameScreen::EnemyIconMgr* mgr = Screens::Instance()->Game()->GetEnemyIconMgr();

	if(slot.IsSaving()){

		size_t index = 0;
		GameScreen::EnemyIconMgr::EnemyIcon* icon = 0;

		while(icon = mgr->ItemAt(index++)){
			slot << icon->GetMarker()->GetEntity()->GetEID();
		}

		slot << static_cast<eid_t>(0);

	}else{

		eid_t eid;
		slot >> eid;

		mgr->Clear();

		while(eid)
		{
			if(EntityPool::GetInst()->Get(eid))
			{
				Insert(EntityPool::GetInst()->Get(eid)->GetEntityContext()->GetMarker());
			}

			slot >> eid;
		}

		AttachObservers();
	}
}

//==============================================================

//=====================================================================================//
//                              RobotIcons::RobotIcons()                               //
//=====================================================================================//
RobotIcons::RobotIcons()
{
	Spawner::Attach(this, ET_PREPARE_SPAWN);
	Spawner::Attach(this, ET_ENTITY_SPAWN);

	Screens::Instance()->Game()->GetHackVehicleMgr()->Clear();
}

//=====================================================================================//
//                              RobotIcons::~RobotIcons()                              //
//=====================================================================================//
RobotIcons::~RobotIcons()
{
	Spawner::Detach(this);
}

//=====================================================================================//
//                              void RobotIcons::Insert()                              //
//=====================================================================================//
void RobotIcons::Insert(VehicleEntity* robot)
{
	Screens::Instance()->Game()->GetHackVehicleMgr()->Insert(GameScreen::HackVehicleMgr::Icon(robot->GetInfo()->GetName(), robot));        

	robot->Attach(this, EV_PREPARE_DEATH);
	robot->Attach(this, EV_SELECTION_CHANGE);
	robot->Attach(this, EV_DESTROY);

	SetRobotState(robot);
}

//=====================================================================================//
//                      VehicleEntity* RobotIcons::Icon2Entity()                       //
//=====================================================================================//
VehicleEntity* RobotIcons::Icon2Entity(unsigned index)
{
	GameScreen::HackVehicleMgr::Icon* icon = 0;

	if(icon = Screens::Instance()->Game()->GetHackVehicleMgr()->ItemAt(index))
		if(icon->GetVehicleEntity()) return icon->GetVehicleEntity()->Cast2Vehicle();           

	return 0;
}

//=====================================================================================//
//                              void RobotIcons::Update()                              //
//=====================================================================================//
void RobotIcons::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
	switch(type)
	{
	case ET_PREPARE_SPAWN:
		Screens::Instance()->Game()->GetHackVehicleMgr()->Clear();
		break;

	case ET_ENTITY_SPAWN:
		{
			VehicleEntity* vehicle =    static_cast<spawn_info_s*>(info)->m_entity
				?  static_cast<spawn_info_s*>(info)->m_entity->Cast2Vehicle()
				:  0;
			if(     vehicle
				&&  vehicle->GetInfo()->IsRobot()
				&&  vehicle->GetPlayer() == PT_PLAYER)
				Insert(vehicle);
		}
		break;
	}

}

//=====================================================================================//
//                              void RobotIcons::Update()                              //
//=====================================================================================//
void RobotIcons::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
	switch(event)
	{
	case EV_PREPARE_DEATH:
	case EV_DESTROY:
		Remove(entity->Cast2Vehicle());
		break;
	case EV_SELECTION_CHANGE:
		SetRobotState(entity->Cast2Vehicle());
		break;
	}
}

//=====================================================================================//
//                          void RobotIcons::SetRobotState()                           //
//=====================================================================================//
void RobotIcons::SetRobotState(VehicleEntity *robot)
{    
	unsigned flags = 0;
	if(robot->GetEntityContext()->IsSelected()) flags |= GameScreen::HackVehicleMgr::Icon::F_SELECTED;

	GameScreen::HackVehicleMgr::Icon* icon = 0; 

	if(icon = Screens::Instance()->Game()->GetHackVehicleMgr()->ItemAt(robot))
		icon->SetFlags(flags);
}

//=====================================================================================//
//                              void RobotIcons::Remove()                              //
//=====================================================================================//
void RobotIcons::Remove(VehicleEntity *robot)
{
	robot->Detach(this);
	Screens::Instance()->Game()->GetHackVehicleMgr()->Remove(robot);    
}

//=====================================================================================//
//                           void RobotIcons::MakeSaveLoad()                           //
//=====================================================================================//
void RobotIcons::MakeSaveLoad(SavSlot &slot)
{
	GameScreen::HackVehicleMgr* mgr = Screens::Instance()->Game()->GetHackVehicleMgr();

	if(slot.IsSaving())
	{
		size_t index = 0;
		GameScreen::HackVehicleMgr::Icon* icon = 0;

		while(icon = mgr->ItemAt(index++))
		{
			slot << icon->GetVehicleEntity()->GetEID();
		}

		//признак конца
		slot << static_cast<eid_t>(0);
	}
	else
	{
		eid_t eid;
		slot >> eid;

		mgr->Clear();

		while(eid)
		{
			Insert(EntityPool::GetInst()->Get(eid)->Cast2Vehicle());
			slot >> eid;
		}
	}
}

//==============================================================

Reloader::Reloader()
{
}

Reloader::~Reloader()
{
}

//=====================================================================================//
//                              void Reloader::DoReload()                              //
//=====================================================================================//
void Reloader::DoReload(HumanEntity* human)
{       
	HumanContext* context = human->GetEntityContext();
	WeaponThing*  weapon  = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON));

	const bool unlim = UnlimitedMoves();

	int mps=context->GetTraits()->GetMovepnts();
	int mps4reload = weapon->GetInfo()->GetMps2Reload();
	if(mps<mps4reload && !unlim)
	{
		DirtyLinks::Print(DirtyLinks::GetStrRes("aui_no_movepnts"));
		return;
	}

	context->GetTraits()->PoLAddMovepnts(-mps4reload);

	HumanContext::reload_notifier notifier(human, weapon);

	HumanContext::iterator itor = context->begin(HPK_ALL, TT_AMMO);

	//ищем подходящие патроны
	while(itor != context->end())
	{
		AmmoThing* ammo4load = static_cast<AmmoThing*>(&(*itor++));

		//если патроны не подходят перейти к следущим
		if(!ThingDelivery::IsSuitable(weapon, ammo4load))
			continue;

		if(AmmoThing* weapon_ammo = weapon->GetAmmo())
		{
			if(weapon_ammo->GetInfo()->GetRID() == ammo4load->GetInfo()->GetRID())
			{
				const int prev_ammo = weapon_ammo->GetBulletCount();
				//пополним обойму
				weapon_ammo->SetBulletCount(std::min(ammo4load->GetBulletCount() + weapon_ammo->GetBulletCount(),weapon->GetInfo()->GetQuantity()));

				const int cur_ammo = weapon_ammo->GetBulletCount();
				const int ammo_diff = cur_ammo - prev_ammo;
				ammo4load->SetBulletCount(ammo4load->GetBulletCount() - ammo_diff);

				//если обойма пуста удалить ее
				if(ammo4load->GetBulletCount() == 0)
				{
					context->RemoveThing(ammo4load);
					delete ammo4load;
				}
			}
		}
		else
		{
			//оружие пустое заряжаем первые попавшиеся патроны
			if(ammo4load->GetBulletCount() > weapon->GetInfo()->GetQuantity())
			{
				AmmoThing* new_ammo = static_cast<AmmoThing*>(ThingFactory::GetInst()->CreateAmmo(
					ammo4load->GetInfo()->GetRID(),
					weapon->GetInfo()->GetQuantity()));
				//создаим новые патроны в оружии
				weapon->Load(new_ammo);

				//уберем заряженное из обоймы
				ammo4load->SetBulletCount(ammo4load->GetBulletCount() - weapon->GetInfo()->GetQuantity());
			}
			else
			{
				context->RemoveThing(ammo4load);
				weapon->Load(ammo4load);
			}
		} 

		//если оружие больше не заряжается выход
		if(!CanLoadAmmo(weapon)) break;
	}
}

//=====================================================================================//
//                          bool Reloader::CanReload() const                           //
//=====================================================================================//
bool Reloader::CanReload(HumanEntity* human) const
{
	HumanContext* context = human->GetEntityContext();

	//если у человека нет оружия  
	if(WeaponThing* weapon = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON)))
	{
		//если оружие не заряжается
		if(!CanLoadAmmo(weapon)) return false;

		//получим ссылку на патроны в оружии
		AmmoThing* weapon_ammo = weapon->GetAmmo();

		HumanContext::iterator itor = context->begin(HPK_ALL, TT_AMMO);

		//ищем подходящие патроны
		while(itor != context->end())
		{
			if(ThingDelivery::IsSuitable(weapon, &(*itor)))
			{
				//если в оружие нет патронов, то подойдут любые
				if(weapon_ammo == 0)
				{
					return true;
				}

				//если в оружии есть патроны то нужны такие же
				if(weapon_ammo->GetInfo()->GetRID() == itor->GetInfo()->GetRID())
					return true;
			}

			++itor;
		}
	}

	return false;
}

bool Reloader::CanLoadAmmo(const WeaponThing* weapon) const
{
	return      weapon->GetAmmo() == 0
		||  weapon->GetAmmo()->GetBulletCount() < weapon->GetInfo()->GetQuantity();
}

//==============================================================

//=====================================================================================//
//                             GameFormImp::GameFormImp()                              //
//=====================================================================================//
GameFormImp::GameFormImp()
:	m_last_move(0),
	m_activity(0),
	m_auto_turn(0.0f),
	m_action_mgr(0),
	m_update_delta(0.35f),
	m_cur_ent(0),
	m_flags(0),
	m_turn_state(TS_START_TURN),
	m_activity_type(AT_NONE),
	m_update_time(0)
{
	Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL);
	Spawner::GetInst()->Attach(this, ET_FINISH_SPAWN);    
	Spawner::GetInst()->Attach(this, ET_DESTROY_ACTIVITY);

	//проиниц массив драйверов ввода
	m_input_mgr[IT_EMPTY]     = new EmptyInputManager();
	m_input_mgr[IT_USUAL]     = new UsualInputManager();
	m_input_mgr[IT_ACTIVITY]  = new ActivityInputManager();

	Init();
}

//=====================================================================================//
//                             GameFormImp::~GameFormImp()                             //
//=====================================================================================//
GameFormImp::~GameFormImp()
{
	Spawner::GetInst()->Detach(this);

	if(m_cur_ent) m_cur_ent->Detach(this);

	for(int k = 0; k < MAX_INPUT_MGRS; delete m_input_mgr[k++]);

	delete m_activity;
}

void GameFormImp::Show()
{
	GameScreen* screen = Screens::Instance()->Game();

	screen->ShowControls(true);
	screen->SetController(this);
	screen->SetCBState(GameScreen::CB_MENU, GameScreen::CBS_OFF);

	m_auto_turn = 0.0f;
	m_last_move = Timer::GetSeconds();

	std::string str;

#ifdef _HOME_VERSION
	if(DirtyLinks::GetCmdLineArg("-autoturn", &str))
	{
		std::istringstream istr(str);
		istr >> m_auto_turn;
	}
#endif

	Screens::Instance()->Activate(Screens::SID_GAME);
}

//=====================================================================================//
//                           void GameFormImp::HandleInput()                           //
//=====================================================================================//
void GameFormImp::HandleInput(state_type* st)
{
	if(!st) return;
	*st = ST_INCOMPLETE;

	std::string lev;
	const bool endTurnEnabled = !UnlimitedMoves() || CanExitLevel(&lev);

	Screens::Instance()->Game()->EnableCBState(GameScreen::CB_ENDTURN,endTurnEnabled);
	m_vehicles.Update();

	//установить кнопку журнала квестов
	if(m_update_time < Timer::GetSeconds())
	{        
		m_update_time = Timer::GetSeconds() + m_update_delta;

		bool fupdated = MessageCounter::HasNewMessages();
		bool blink = Forms::GetInst()->GetJournalForm()->IsUpdated();

		if(fupdated && blink)
		{
			MessageCounter::UpdateNewMessages();
			DirtyLinks::Print(DirtyLinks::GetStrRes("gf_journal_updated"));
			SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_JOURNAL_WAS_UPDATED));
		}

		Screens::Instance()->Game()->SetJournalButtonState( 
			blink
			?   GameScreen::JBS_BLINK
			:   GameScreen::JBS_NORMAL);     
	}

	//проигрывание действия
	if(m_activity_type != AT_NONE)
	{
		if(m_turn_state == TS_INCOMPLETE)
			m_input_mgr[IT_ACTIVITY]->HandleInput();
		else
			RunActivity(AC_TICK);

		return;
	}    

	if(Input::MouseState().dx || Input::MouseState().dy || Input::MouseState().dz)
		m_last_move = Timer::GetSeconds();

#ifdef _HOME_VERSION
	if(m_auto_turn && m_auto_turn < Timer::GetSeconds() - m_last_move)
	{
		m_auto_turn = 0.0f;
		HandleEndOfTurnReq();
	}
#endif

	switch(m_turn_state)
	{
	case TS_INCOMPLETE:
		m_input_mgr[IT_USUAL]->HandleInput();
		break;

	case TS_START_TURN:
		m_turn_state = TS_INCOMPLETE;
		if(m_cur_ent)
		{
			HandleSelectReq(m_cur_ent);
			Cameraman::GetInst()->FocusEntity(m_cur_ent);
		}
		break;

	case TS_FINISHED:
		*st = ST_FINISHED;
		m_turn_state = TS_START_TURN;
		break;

	case TS_RUN_WALKERS:
		if(ActionManager* action = m_walkers.CreateAction())
			HandlePlayActionReq(action);
		else
			m_turn_state = TS_FINISHED;
		break;

	case TS_TEST_LEVEL_EXIT:
		{
			Screens::Instance()->Game()->ShowControls(false);
			Cameraman::GetInst()->Configure(Cameraman::F_SHOW_ALL);

			ShowUsualCursor();

			AIUtils::HideLandField();
			AIUtils::HidePassField();

			m_turn_state = TS_RUN_WALKERS;

			std::string full_name;

			if(!CanExitLevel(&m_targetLevel))
				break;

			full_name = DirtyLinks::GetLevelFullName(m_targetLevel);

			if(Forms::GetInst()->ShowMessageBox( mlprintf( DirtyLinks::GetStrRes("gf_can_exit_level").c_str(), full_name.c_str())))
			{
				m_turn_state = TS_EXIT_LEVEL;

				//сохрание по выходе с уровня
				//Screens::Instance()->Game()->MakeSpecialSaveLoad(GameScreen::SST_AUTOSAVE, GameScreen::SSM_SAVE);

				GameObserver::exit_level_info info(m_targetLevel);
				GameEvMessenger::GetInst()->Notify(GameObserver::EV_INIT_LEVEL_EXIT, &info);
			}            
		}
		break;

	case TS_EXIT_LEVEL:
		if(!PoL::ActMgr::isPlayingScene())
		{
			m_turn_state = TS_WAIT_LEVEL;

			//уничтожить ссылку на текущего перца
			if(m_cur_ent) HandleSelectReq(0);

			//выход с уровня
			Spawner::GetInst()->ExitLevel(m_targetLevel);
		}
		break;

	case TS_WAIT_LEVEL:
		break;
	}
}

void GameFormImp::MakeSaveLoad(SavSlot& slot)
{
	if(slot.GetStore()->GetVersion() >= 12)
	{
		MessageCounter::MakeSaveLoad(slot);
	}

	m_robots.MakeSaveLoad(slot);
	m_humans.MakeSaveLoad(slot);
	m_enemies.MakeSaveLoad(slot);
	m_walkers.MakeSaveLoad(slot);
	m_vehicles.MakeSaveLoad(slot);

	//	Naughty: переставляю эту строчку только для загрузки
	//  Screens::Instance()->Game()->MsgWindow()->Clear();

	if(slot.IsSaving())
	{
		slot << m_flags;
		slot << (m_cur_ent != 0);

		if(m_cur_ent) slot << m_cur_ent->GetEID();
	}
	else
	{
		slot >> m_flags;

		bool flag;
		slot >> flag;

		if(flag)
		{
			eid_t eid; slot >> eid;
			HandleSelectReq(EntityPool::GetInst()->Get(eid));
		}

		RestoreSelection();
		ResetCursorModeButtons();

		m_turn_state = TS_INCOMPLETE;

		//	Naughty: очищаем консоль
		Screens::Instance()->Game()->MsgWindow()->Clear();
	}
}

void GameFormImp::HandleSwitch2Run()
{
	Switch2Run switcher;
	m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Sit()
{
	Switch2Sit switcher;
	m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Stand()
{
	Switch2Stand switcher;
	m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Aimshot()
{
	Switch2Aimshot switcher;
	m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Autoshot()
{
	Switch2Autoshot switcher;
	m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSwitch2Snapshot()
{
	Switch2Snapshot switcher;
	m_cur_ent->Accept(switcher);
}

void GameFormImp::HandleSelectReq(BaseEntity* entity, selection_type type)
{
	if(m_cur_ent)
	{
		//снять наблюдателя
		m_cur_ent->Detach(this);
		m_cur_ent->Accept(Deselector());
		//RunActivity(AC_STOP);//Grom
	}

	//уберем кнопки принудительной атаки, use, hack
	if(     entity == 0 
		||  (m_cur_ent && m_cur_ent->GetEID() != entity->GetEID()))
		ResetCursorModeButtons();

	if(m_cur_ent = entity)
	{
		//поставить наблюдателя
		m_cur_ent->Attach(this, EV_DESTROY);
		m_cur_ent->Attach(this, EV_QUIT_TEAM);
		m_cur_ent->Attach(this, EV_INSERT_THING);
		m_cur_ent->Attach(this, EV_REMOVE_THING);
		m_cur_ent->Attach(this, EV_PREPARE_DEATH);
		m_cur_ent->Attach(this, EV_HEX_COST_CHANGE);
		m_cur_ent->Attach(this, EV_MOVEPNTS_CHANGE);
		m_cur_ent->Attach(this, EV_BODY_PACK_CHANGE);
		m_cur_ent->Attach(this, EV_BODY_PART_CHANGE);
		m_cur_ent->Attach(this, EV_HANDS_PACK_CHANGE);
		m_cur_ent->Attach(this, EV_LONG_DAMAGE_CHANGE);
		m_cur_ent->Attach(this, EV_WEAPON_STATE_CHANGE);
		m_cur_ent->Attach(this, EV_CAN_MAKE_FAST_ACCESS);

		if(type == SEL_COMMAND) m_cur_ent->Notify(EntityObserver::EV_SELECT_COMMAND);

		Selector sel;
		m_cur_ent->Accept(sel);
	}

	m_cursor.Invalidate();
}

void GameFormImp::HandlePlayActionReq(ActionManager* mgr)
{
	ShowUsualCursor();

	AIUtils::HidePassField();
	AIUtils::HideLandField();

	m_action_mgr = mgr;     
	Screens::Instance()->Game()->SetController(0);

	m_activity_type = AT_ACTION_MOVE;
	if(m_action_mgr->NeedMove()) m_activity = m_action_mgr->CreateMove();       
}

void GameFormImp::Init()
{
	GameScreen* screen = Screens::Instance()->Game();

	screen->SetJournalButtonState(GameScreen::JBS_NORMAL);

	screen->SetCBState(GameScreen::CB_RUN, GameScreen::CBS_OFF);
	screen->SetCBState(GameScreen::CB_SIT, GameScreen::CBS_OFF); 
	screen->SetCBState(GameScreen::CB_USE, GameScreen::CBS_OFF);
	screen->SetCBState(GameScreen::CB_SIT, GameScreen::CBS_OFF);
	screen->SetCBState(GameScreen::CB_RUN, GameScreen::CBS_OFF);
	screen->SetCBState(GameScreen::CB_MENU, GameScreen::CBS_OFF);
	screen->SetCBState(GameScreen::CB_ROOF, GameScreen::CBS_OFF);
	screen->SetCBState(GameScreen::CB_ENDTURN, GameScreen::CBS_OFF);

	screen->SetCBState(GameScreen::CB_FOS, (m_flags & FT_SHOW_FOS) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);

	screen->EnableCBState(GameScreen::CB_USE, false);
	screen->EnableCBState(GameScreen::CB_SIT, false);
	screen->EnableCBState(GameScreen::CB_RUN, false);
	screen->EnableCBState(GameScreen::CB_AIMSHOT, false);
	screen->EnableCBState(GameScreen::CB_SNAPSHOT, false);
	screen->EnableCBState(GameScreen::CB_AUTOSHOT, false);

	GameScreen::BodyIconTraits icon_traits;

	icon_traits.m_Body  = GameScreen::BodyIconTraits::BPS_NONE;
	icon_traits.m_Hands = GameScreen::BodyIconTraits::BPS_NONE;
	icon_traits.m_Head  = GameScreen::BodyIconTraits::BPS_NONE;
	icon_traits.m_Legs  = GameScreen::BodyIconTraits::BPS_NONE;
	icon_traits.m_Damages = 0;

	screen->SetBodyIcon(icon_traits);

	SwitchRoofs();
}

void GameFormImp::RunActivity(int command)
{
	if(m_activity && m_activity->Run(static_cast<activity_command>(command)))
	{
		return;
	}

	delete m_activity;
	m_activity = 0;

	switch(m_activity_type)
	{
		case AT_ACTION_MOVE:
			if(TrapActionMoveEnd()) return;
			break;

		case AT_ACTION_ROTATE:
			if(TrapActionRotateEnd()) return;
			break;

		case AT_ACTION:
			if(TrapActionEnd()) return;
			break;
	}
	if(m_cur_ent)
	{
		Selector sel;
		m_cur_ent->Accept(sel);
	}

	m_activity_type = AT_NONE;
	Screens::Instance()->Game()->SetController(this);
}

void GameFormImp::HandleSwitchFOSReq()
{
	m_flags ^= FT_SHOW_FOS;
	Screens::Instance()->Game()->SetCBState(GameScreen::CB_FOS, (m_flags & FT_SHOW_FOS) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
}

void GameFormImp::HandleEndOfTurnReq()
{
	m_turn_state = TS_TEST_LEVEL_EXIT; 
}

void GameFormImp::HandleShowJournalReq()
{
	ShowUsualCursor();
	Forms::GetInst()->ShowJournalForm();
	Screens::Instance()->Game()->SetJournalButtonState(GameScreen::JBS_SELECTED);
}

void GameFormImp::HandleShowInventoryReq(HumanEntity* human, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin)
{
	ShowUsualCursor();
	Forms::GetInst()->ShowInventoryForm(human, ground, scroller, bin);
}

void GameFormImp::Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info)
{
	assert(!IsBadWritePtr(entity,sizeof(BaseEntity)));

	switch(event){
	case EV_DESTROY:
		m_cur_ent = NULL;
	case EV_PREPARE_DEATH:
		//if(m_action_mgr) m_action_mgr->Shut();//Grom
		//Внимание! сквозной switch
	case EV_QUIT_TEAM:
		GetGameFormImp()->HandleSelectReq(0);
		break;

	case EV_HEX_COST_CHANGE:
	case EV_MOVEPNTS_CHANGE: 

		m_cur_ent->Accept(WeaponSelector());

		if(m_activity_type == AT_NONE && m_turn_state != TS_RUN_WALKERS)
			m_cur_ent->Accept(MovepntsSelector());
		break;

	case EV_BODY_PART_CHANGE:
	case EV_LONG_DAMAGE_CHANGE:
		m_cur_ent->Accept(BodySelector());
		break;

	case EV_BODY_PACK_CHANGE:
		m_cur_ent->Accept(ButtonStateSelector());
		break;

	case EV_CAN_MAKE_FAST_ACCESS:
		m_cur_ent->Accept(QuickSlotSelector());
		break;

	case EV_REMOVE_THING:
	case EV_INSERT_THING:
		if(static_cast<thing_info_s*>(info)->m_thing->GetInfo()->IsAmmo())
			m_cur_ent->Accept(ReloadButtonSelector());
		break;

	case EV_HANDS_PACK_CHANGE:
	case EV_WEAPON_STATE_CHANGE:
		m_cur_ent->Accept(WeaponSelector());
		m_cur_ent->Accept(QuickSlotSelector());
		m_cur_ent->Accept(ButtonStateSelector());
		m_cur_ent->Accept(ReloadButtonSelector());
		if(CanAttack()) HandleSwitchAttackReq();
		break;
	}
}

bool GameFormImp::TrapActionMoveEnd()
{
	if(!m_cur_ent && (m_activity_type==AT_ACTION_MOVE||m_activity_type==AT_ACTION_ROTATE) )
	{
		Screens::Instance()->Game()->SetController(this);
	}
	else
	{
		if(m_action_mgr->NeedMove())
		{
			return false;
		}
		//m_activity_type = AT_ACTION_ROTATE;

		if(m_action_mgr->NeedRotate())
		{
			m_activity = m_action_mgr->CreateRotate();
		}
	}

	m_activity_type = AT_ACTION_ROTATE;

	return true;
}

bool GameFormImp::TrapActionRotateEnd()
{ 
	if(!m_cur_ent && (m_activity_type==AT_ACTION_MOVE||m_activity_type==AT_ACTION_ROTATE) )
	{
	}
	else
	{
		if(m_action_mgr->NeedRotate())
			return false;
	}

	m_activity_type = AT_ACTION;
	m_activity = m_action_mgr->CreateAction();

	if(!m_activity)
	{
		Screens::Instance()->Game()->SetController(this);
	}

	return true;
}

bool GameFormImp::TrapActionEnd()
{
	/*if(!m_cur_ent && (m_activity_type==AT_ACTION_MOVE||m_activity_type==AT_ACTION_ROTATE) )
	return false;
	*/
	m_action_mgr->Shut();
	return false;
}

bool GameFormImp::CanExitLevel(std::string* level)
{
	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_PLAYER);

	//если наших нет - выход
	if(itor == EntityPool::GetInst()->end())
		return false;

	itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER, EA_INCREW);
	while(itor != EntityPool::GetInst()->end()){
		HumanEntity *ent = itor->Cast2Human();
		if(ent)
		{
			VehicleEntity *v=ent->GetEntityContext()->GetCrew();
			if(v->GetPlayer()!=PT_PLAYER) return false;
		}
		++itor;
	}


	const unsigned none_joints = 0xffffffff;

	unsigned joints = none_joints;

	//проверка для людей
	itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER, EA_NOT_INCREW);
	while(itor != EntityPool::GetInst()->end()){
		joints &= HexGrid::GetInst()->GetProp(itor->GetGraph()->GetPos2()).m_joints;
		++ itor;
	}

	//проверка техники
	itor = EntityPool::GetInst()->begin(ET_VEHICLE, PT_PLAYER);
	while(itor != EntityPool::GetInst()->end()){

		if(itor->Cast2Vehicle()->GetInfo()->IsTech()){
			joints &= HexGrid::GetInst()->GetProp(itor->GetGraph()->GetPos2()).m_joints;
		}

		++itor;
	}

	return joints && joints != none_joints && Spawner::GetInst()->CanExit(joints, level);
}

//=====================================================================================//
//                             void GameFormImp::Update()                              //
//=====================================================================================//
void GameFormImp::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
	assert(!IsBadWritePtr(spawner,sizeof(Spawner)));

	switch(type)
	{
	case ET_ENTRY_LEVEL:
		if(static_cast<entry_info_s*>(info)->m_entry.empty())
		{
			ResetCursorModeButtons();
		}
		if(m_activity)
		{
			delete m_activity;
			m_activity = 0;
		}
		break;
	case ET_FINISH_SPAWN:
		RestoreSelection();
		m_turn_state = TS_START_TURN;
		Screens::Instance()->Game()->MsgWindow()->Clear();
		break;
	case ET_DESTROY_ACTIVITY:
		if(m_activity)
		{
			delete m_activity;
			m_activity = 0;
		}
		break;
	}
}

void GameFormImp::HandleShowShopReq(HumanEntity* hero, TraderEntity* trader, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin)
{
	Forms::GetInst()->ShowShopForm(hero, trader, ground, scroller, bin);
	ShowUsualCursor();
}

void GameFormImp::HandleShowOptionsReq()
{
	ShowUsualCursor();

	Forms::GetInst()->ShowOptionsDialog(Forms::ODM_GAMEMENU);
	Screens::Instance()->Game()->SetCBState(GameScreen::CB_MENU, GameScreen::CBS_ON);
}

void GameFormImp::HandleSwitchUseReq()
{
	if(m_cur_ent && m_cur_ent->Cast2Human())
	{
		//переключить состояние флага
		if((m_flags ^= FT_CAN_USE) & FT_CAN_USE)
		{
			if(CanAttack()) HandleSwitchAttackReq();
			if(CanMakeHack()) HandleSwitchHackReq();
		}

		//обновить курсор немедленно
		m_cursor.Invalidate();

		//изменить вид кнопки
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_USE, (m_flags & FT_CAN_USE) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
	}
}

void GameFormImp::HandleSwitchAttackReq()
{
	if(m_cur_ent && m_cur_ent->Cast2Human())
	{
		HumanEntity *he = m_cur_ent->Cast2Human();
		ScannerThing *scanner = static_cast<ScannerThing*>(he->GetEntityContext()->GetThingInHands(TT_SCANNER));

		if(scanner)
		{
			if(!scanner->IsInUse())
			{
				AIUtils::UseScanner(he,scanner);
			}
		}
		else
		{
			//переключим флаг
			if((m_flags ^= FT_CAN_ATTACK) & FT_CAN_ATTACK)
			{
				if(CanMakeUse()) HandleSwitchUseReq();
				if(CanMakeHack()) HandleSwitchHackReq();
			}

			//обновим курсор
			m_cursor.Invalidate();
		}
	}
}

bool GameFormImp::CanMakeUse() const
{
	return m_cur_ent && (m_flags & FT_CAN_USE) && m_cur_ent->Cast2Human();
}

bool GameFormImp::CanAttack() const
{
	return m_cur_ent && (m_flags & FT_CAN_ATTACK) && m_cur_ent->Cast2Human();
}

void GameFormImp::RestoreSelection()
{
	EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
	while(itor != EntityPool::GetInst()->end()){

		if(itor->Cast2Human()->GetEntityContext()->IsSelected()){
			Cameraman::GetInst()->FocusEntity(&*itor, 0);
			HandleSelectReq(&*itor);
			return;
		}

		++itor;
	}

	//просмотрим технику
	itor = EntityPool::GetInst()->begin(ET_VEHICLE);
	while(itor != EntityPool::GetInst()->end()){

		if(itor->Cast2Vehicle()->GetEntityContext()->IsSelected()){
			Cameraman::GetInst()->FocusEntity(&*itor, 0);
			HandleSelectReq(&*itor);
			return;
		}

		++itor;
	}

	//выделим первого в иконках
	if(BaseEntity* ent4sel = m_humans.Icon2Entity(0)){
		Cameraman::GetInst()->FocusEntity(ent4sel, 0);
		HandleSelectReq(ent4sel);
	}
}

void GameFormImp::ShowControls(bool flag)
{
	Screens::Instance()->Game()->ShowControls(flag);
}

bool GameFormImp::CanMakeHack() const
{
	return m_cur_ent && (m_flags & FT_CAN_HACK) && m_cur_ent->Cast2Human();
}

void GameFormImp::HandleSwitchHackReq()
{
	if(m_cur_ent && m_cur_ent->Cast2Human()){

		//переключить состояние флага
		if((m_flags ^= FT_CAN_HACK) & FT_CAN_HACK){         
			if(CanMakeUse()) HandleSwitchUseReq();
			if(CanAttack()) HandleSwitchAttackReq();
		}        

		//обновить курсор немедленно
		m_cursor.Invalidate();
		//изменить вид кнопки
		Screens::Instance()->Game()->SetCBState(GameScreen::CB_SPECIAL, (m_flags & FT_CAN_HACK) ? GameScreen::CBS_ON : GameScreen::CBS_OFF);
	}
}

void GameFormImp::ShowUsualCursor()
{
	m_cursor.SetUsual();
}

void GameFormImp::OnCheckBoxClick(GameScreen::CHECKBOX id)
{
	switch(id){
	case GameScreen::CB_USE:
		HandleSwitchUseReq();
		break;

	case GameScreen::CB_FOS:
		HandleSwitchFOSReq();
		break;

	case GameScreen::CB_SPECIAL:
		HandleSwitchHackReq();
		break;

	case GameScreen::CB_EYES:
		SwitchEyes();
		break;

	case GameScreen::CB_ROOF:
		SwitchRoofs();
		break;   

	case GameScreen::CB_ENDTURN:
		HandleEndOfTurnReq();
		break;

	case GameScreen::CB_MENU:
		HandleShowOptionsReq();
		break;

	case GameScreen::CB_JOURNAL:
		HandleShowJournalReq();
		break;             
	}

	if(m_cur_ent == 0) return;

	switch(id){
	case GameScreen::CB_AIMSHOT:
		HandleSwitch2Aimshot();
		break;

	case GameScreen::CB_AUTOSHOT:
		HandleSwitch2Autoshot();
		break;

	case GameScreen::CB_SNAPSHOT:
		HandleSwitch2Snapshot();
		break;

	case GameScreen::CB_SIT:
		if(IsChecked(GameScreen::CB_SIT))
			HandleSwitch2Stand();
		else
			HandleSwitch2Sit();
		break;

	case GameScreen::CB_RUN:
		if(IsChecked(GameScreen::CB_RUN))
			HandleSwitch2Stand();
		else
			HandleSwitch2Run();
		break;   

	case GameScreen::CB_BACKPACK:
		HandleSwitchHandsMode();
		break;
	}
}

void GameFormImp::OnIconClick(unsigned int id,GameScreen::BN_CLICK click_type)
{
	HumanEntity* human = m_humans.Icon2Entity(id);

	if(!human) return;

	switch(click_type)
	{
	case GameScreen::BNC_LCLICK:
		HandleSelectReq(human, SEL_COMMAND);
		break;

	case GameScreen::BNC_RCLICK:        
		HandleShowInventoryReq(human, UsualGround::GetInst(), GameFormHeroScroller::GetInst(), GroundBin::GetInst());
		break;

	case GameScreen::BNC_LDBLCLICK:
		Cameraman::GetInst()->FocusEntity(human);
		break;
	}
}

//=====================================================================================//
//                        void GameFormImp::OnWeaponIconClick()                        //
//=====================================================================================//
void GameFormImp::OnWeaponIconClick() 
{  
	if(Input::MouseState().LButtonFront)
	{
		HandleSwitchAttackReq();
		return;
	}

	if(Input::MouseState().RButtonFront)
	{
		if(m_cur_ent == 0 || m_cur_ent->Cast2Human() == 0)
		{
			return;
		}

		BaseThing* thing = m_cur_ent->Cast2Human()->GetEntityContext()->GetThingInHands();
		if(thing) Forms::GetInst()->ShowDescDialog(thing);                
	}
}

void GameFormImp::OnVehicleIconClick(unsigned int icon_number, GameScreen::BN_CLICK click_type)
{
	VehicleEntity* vehicle = GetGameFormImp()->GetTechIcons()->Icon2Entity(icon_number);

	if(!vehicle) return;

	switch(click_type){
	case GameScreen::BNC_LCLICK:
		HandleSelectReq(vehicle);
		break;

	case GameScreen::BNC_LDBLCLICK:
		Cameraman::GetInst()->FocusEntity(vehicle);
		break;
	}
}

void GameFormImp::OnEnemyIconClick(Marker* marker)
{   
	if(BaseEntity* entity = marker->GetEntity()) Cameraman::GetInst()->FocusEntity(entity);
}

void GameFormImp::OnHackVehicleClick(VehicleEntity* vehicle, GameScreen::BN_CLICK click_type)
{
	switch(click_type){
	case GameScreen::BNC_LCLICK:
		HandleSelectReq(vehicle);
		break;

	case GameScreen::BNC_LDBLCLICK:
		Cameraman::GetInst()->FocusEntity(vehicle);
		break;
	}
}

void GameFormImp::OnButtonClick(GameScreen::BUTTON button)
{
	switch(button){
	case GameScreen::B_RELOAD:
		HandleReloadWeapon();
		break;  

	case GameScreen::B_UP_ARROW:
		HandleNextFastAccessThing();
		break;

	case GameScreen::B_DOWN_ARROW:
		HandlePrevFastAccessThing();
		break;
	}
}

void GameFormImp::Init(const ini_s& ini)
{
}

void GameFormImp::HandleReloadWeapon()
{
	HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0;

	if(human && m_reloader.CanReload(human))
	{
		m_reloader.DoReload(human);
	}
}

void GameFormImp::HandleSwitchHandsMode()
{
	if(HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0){

		HumanContext* context = human->GetEntityContext();

		context->SetHandsMode(      context->GetHandsMode() == HumanContext::HM_HANDS
			?   HumanContext::HM_FAST_ACCESS
			:   HumanContext::HM_HANDS);
	}
}

void GameFormImp::HandleNextFastAccessThing()
{
	HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0;

	if(human && human->GetEntityContext()->GetHandsMode() == HumanContext::HM_FAST_ACCESS)
		SetNextFastThing(human, FastAccessStrategy::IT_FORWARD);    
}

void GameFormImp::HandlePrevFastAccessThing()
{
	HumanEntity* human = m_cur_ent ? m_cur_ent->Cast2Human() : 0;

	if(human && human->GetEntityContext()->GetHandsMode() == HumanContext::HM_FAST_ACCESS)
		SetNextFastThing(human, FastAccessStrategy::IT_BACKWARD);
}

void GameFormImp::ResetCursorModeButtons()
{
	if(CanMakeUse()) HandleSwitchUseReq();
	if(CanAttack()) HandleSwitchAttackReq();
	if(CanMakeHack()) HandleSwitchHackReq();
}


