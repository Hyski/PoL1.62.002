#pragma once

class Marker;
class Spectator;

//=====================================================================================//
//                class SpectatorObserver : public Observer<Spectator*>                //
//=====================================================================================//
// наблюдатель за Spectator'ом
class SpectatorObserver : public Observer<Spectator*>
{
public:
    enum event_type
	{
        EV_NONE,
        EV_VISIBLE_MARKER,   //маркер вошел в поле видимости
        EV_INVISIBLE_MARKER, //маркер вышел из поля видимости
    };

    //информация о вошедшем/вышедшем маркерах
    struct marker_info
	{
        Marker* m_marker;
        marker_info(Marker* mk) : m_marker(mk) {}
    };
};
