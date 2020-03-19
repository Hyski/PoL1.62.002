//
// –еализаци€ методов игроков
//
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "form.h"
#include "Player.h"
#include "AIBase.h"
#include "Entity.h"
#include "AIUtils.h"
#include "DirtyLinks.h"
#include "PanicPlayer.h"
#include "GameObserver.h"

//#ifdef _HOME_VERSION
//#	define USING_VTUNE
//#endif

extern bool g_noSleep;

//=====================================================

//
// базовый класс дл€ все игроков
//
class BasePlayer
{
public:

    BasePlayer(player_type type = PT_NONE) : m_ptype(type) {}
    virtual ~BasePlayer(){}

    DCTOR_ABS_DEF(BasePlayer)

    //определение типа игрока
    player_type GetType() const {return m_ptype;}

    //в этой функции игрок думает
    virtual void Think(state_type* st) = 0;

    //обработчик сохранени€
    virtual void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving())
		{
            slot << static_cast<int>(m_ptype);
        }
		else
		{
            int tmp; slot >> tmp;
            m_ptype = static_cast<player_type>(tmp);
        }
    }

private:

    //необходимо дл€ выполнени€ операций сохранени€ / загрузки
    player_type  m_ptype;     //тип команды кот "рулит" игрок     
};

//=====================================================

namespace
{

//=====================================================

//
// общие элементы дл€ всех игроков
//
class CommonPlayer : public BasePlayer, protected GameObserver
{
public:

    CommonPlayer(player_type type) : BasePlayer(type)
    {
        GameEvMessenger::GetInst()->Attach(this, EV_TURN);
    }

    ~CommonPlayer()
    {
        GameEvMessenger::GetInst()->Detach(this);
    }

protected:

    void Update(subject_t subj, event_t event, info_t info)
    {
        turn_info* ptr = static_cast<turn_info*>(info);

        if(ptr->m_player == GetType() && ptr->IsBegining())
		{
            EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_ALL_ENTS, GetType());

            while(itor != EntityPool::GetInst()->end())
			{
                AIUtils::CalcMovepnts4Turn(&*itor);
                ++itor;
            }        

			if(GetType() == PT_PLAYER)
			{
				EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_VEHICLE, PT_ALL_PLAYERS);
				while(itor != EntityPool::GetInst()->end())
				{
					if(VehicleEntity *vehe = itor->Cast2Vehicle())
					{
						if(vehe->GetPlayer() != PT_PLAYER && vehe->GetInfo()->GetCapacity())
						{
							AIUtils::CalcMovepnts4Turn(&*itor);
						}
					}
					++itor;
				}
			}
		}
    }
};

//=====================================================

//
// "обычный" игрок
//
class HumanPlayer: public CommonPlayer
{
public:

    HumanPlayer() : CommonPlayer(PT_PLAYER), m_current(EntityPool::GetInst()->end()) { }

    DCTOR_DEF(HumanPlayer)

    void Think(state_type* st)
    {
        while(m_current != EntityPool::GetInst()->end() && !PanicPlayer::Execute())
		{
            InitEntity(++m_current);
        }

        Forms::GetInst()->HandleInput(st);
    }
    
private:

    void Update(subject_t subj, event_t event, info_t info)
    {
        CommonPlayer::Update(subj, event, info);

        turn_info* ptr = static_cast<turn_info*>(info);

        if(ptr->m_player != GetType())
            return;

        if(ptr->IsBegining())
		{
			g_noSleep = false;
#ifdef USING_VTUNE
			g_vtune.pause();
#endif
            CameraPool::GetInst()->HandleNextTurn();
            ShieldPool::GetInst()->HandleNextTurn();
            
            InitEntity(m_current = EntityPool::GetInst()->begin(ET_ALL_ENTS, GetType()));
        }

		if(ptr->IsEnd())
		{
			g_noSleep = true;
#ifdef USING_VTUNE
			g_vtune.resume();
#endif
		}
    }

    void InitEntity(EntityPool::iterator& itor)
    {
        if(m_current == EntityPool::GetInst()->end())
		{
            Forms::GetInst()->ShowGameForm(true);
            return;
        }

        PanicPlayer::Init(&*m_current);

        if(PanicPlayer::CanExecute())
		{
			Forms::GetInst()->ShowPanicForm((&*m_current)->Cast2Human());        
		}

		PanicPlayer::Execute();
   }

private:

    EntityPool::iterator m_current;
};

DCTOR_IMP(HumanPlayer)

//=====================================================

//
// компьютерный игрок
//
class ComputerPlayer: public CommonPlayer
{
public:

	ComputerPlayer(player_type type, std::auto_ptr<AINode> ai) : 
      CommonPlayer(type), m_ai(ai) {}

    void Think(state_type* st)
    {
		float progress = m_ai->Think(st);
		if(st) Forms::GetInst()->UpdateHMProress(progress/m_ai->getComplexity());
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        BasePlayer::MakeSaveLoad(slot);
        m_ai->MakeSaveLoad(slot);        
    }

protected:
    void Update(subject_t subj, event_t event, info_t info)
    {
        CommonPlayer::Update(subj, event, info);

        turn_info* ptr = static_cast<turn_info*>(info);

        if(ptr->m_player == GetType() && ptr->IsBegining())
            Forms::GetInst()->ShowHMForm(ptr->m_player);        
    }

private:
	std::auto_ptr<AINode> m_ai;
};

//
// игрок от Civilian
//
class CivilianPlayer : public ComputerPlayer{
public:

	CivilianPlayer() : ComputerPlayer(PT_CIVILIAN, std::auto_ptr<AINode>(new CivilRoot)) {}

    DCTOR_DEF(CivilianPlayer)
};

DCTOR_IMP(CivilianPlayer)

//
// игрок от Enemy
//
class EnemyPlayer : public ComputerPlayer{
public:

    EnemyPlayer() : ComputerPlayer(PT_ENEMY, std::auto_ptr<AINode>(new EnemyRoot)) {}

    DCTOR_DEF(EnemyPlayer)
};

DCTOR_IMP(EnemyPlayer)

//=====================================================

} //namespace

//=====================================================

BasePlayer* PlayerFactory::CreateHumanPlayer()
{
    return new HumanPlayer;
}

BasePlayer* PlayerFactory::CreateEnemyPlayer()
{
    return new EnemyPlayer;
}

BasePlayer* PlayerFactory::CreateCivilianPlayer()
{
    return new CivilianPlayer;
}

PlayerFactory* PlayerFactory::GetInst()
{
    static PlayerFactory imp;
    return &imp;
}

//=====================================================

//=====================================================================================//
//                              PlayerPool::PlayerPool()                               //
//=====================================================================================//
PlayerPool::PlayerPool()
:	m_current(MAX_TEAMS)
{
    memset(m_players, 0, sizeof(m_players));
}

//=====================================================================================//
//                              PlayerPool::~PlayerPool()                              //
//=====================================================================================//
PlayerPool::~PlayerPool()
{
    Reset();
}

//=====================================================================================//
//                              void PlayerPool::Insert()                              //
//=====================================================================================//
void PlayerPool::Insert(BasePlayer* player)
{
    for(int k = 0; k < MAX_TEAMS; k++)
	{
        if(m_players[k] == 0)
		{
            m_players[k] = player;
            return;
        }
    }

    throw CASUS("PlayerPool: слишком много игроков");
}

//=====================================================================================//
//                              void PlayerPool::Reset()                               //
//=====================================================================================//
void PlayerPool::Reset()
{
    for(int k = 0; k < MAX_TEAMS; delete m_players[k++]);

    memset(m_players, 0, sizeof(m_players));

    m_current = MAX_TEAMS;
}

//=====================================================================================//
//                              void PlayerPool::Think()                               //
//=====================================================================================//
void PlayerPool::Think()
{
	if(m_players[0] == 0) return;

	if(m_current == MAX_TEAMS)
	{
		//уведомить о начале игры    
		GameObserver::turn_info info(m_players[m_current = 0]->GetType(), GameObserver::TEV_BEG);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_TURN, &info);         
		return;
	}

	for(size_t k = 0; k < MAX_TEAMS; k++)
	{
		if(m_players[k] && k != m_current)
			m_players[k]->Think(0);
	}

	//дать текущему игроку возможность подумать
	state_type st = ST_INCOMPLETE;
	m_players[m_current]->Think(&st);

	//игрок передал ход другому?
	if(st == ST_FINISHED)
	{
		GameObserver::turn_info info(m_players[m_current]->GetType(), GameObserver::TEV_END);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_TURN, &info);

		if(!UnlimitedMoves() || m_players[m_current]->GetType() != PT_PLAYER)
		{
			do
			{
				m_current = (m_current + 1)%MAX_TEAMS;
			}
			while(!m_players[m_current]);

			switch(m_players[m_current]->GetType())
			{
				case PT_PLAYER:
					DirtyLinks::Print(DirtyLinks::GetStrRes("game_new_turn"));
					break;
				case PT_ENEMY:
					DirtyLinks::Print(DirtyLinks::GetStrRes("game_new_enemy_turn"));
					break;
				case PT_CIVILIAN:
					DirtyLinks::Print(DirtyLinks::GetStrRes("game_new_neutral_turn"));
					break;
			}
		}

		info.m_event  = GameObserver::TEV_BEG;
		info.m_player = m_players[m_current]->GetType();

		GameEvMessenger::GetInst()->Notify(GameObserver::EV_TURN, &info);
	}
}

//=====================================================================================//
//                           void PlayerPool::MakeSaveLoad()                           //
//=====================================================================================//
void PlayerPool::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving())
	{
        slot << m_current;        

        for(int k = 0; k < MAX_TEAMS; k++)
		{
            slot << (m_players[k] != 0);
            
            if(m_players[k])
			{
                DynUtils::SaveObj(slot, m_players[k]);
                m_players[k]->MakeSaveLoad(slot);
            }
        }
    }
	else
	{
		const unsigned int playercount = MAX_TEAMS;

        slot >> m_current;

        bool flag;

        for(int k = 0; k < playercount; ++k)
		{
            slot >> flag;

			if(flag)
			{
                DynUtils::LoadObj(slot, m_players[k]);
                m_players[k]->MakeSaveLoad(slot);
            }
        }
    }
}

//=====================================================================================//
//                          PlayerPool* PlayerPool::GetInst()                          //
//=====================================================================================//
PlayerPool* PlayerPool::GetInst()
{
    static PlayerPool imp;
    return &imp;
}
