#pragma once

class Marker;
class Spectator;

//=====================================================================================//
//                   class MarkerObserver : public Observer<Marker*>                   //
//=====================================================================================//
// наблюдатель за маркером
class MarkerObserver : public Observer<Marker*>
{
public:
    enum event_type
	{
        EV_NONE,
        EV_VISIBLE_FOR_SPECTATOR,			// нас заметил наблюдатель
        EV_INVISIBLE_FOR_SPECTATOR,			// мы вышли из поля зрения наблюдателя
		EV_VISIBILITY_FOR_PLAYER_CHANGED	// мы пропали/появились из области действия игрока
    };

    struct spectator_info
	{
        Spectator* m_spectator;
        spectator_info(Spectator* sp) : m_spectator(sp) {}
    };
};
