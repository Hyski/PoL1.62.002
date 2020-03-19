#pragma once

class Marker;
class Spectator;

//=====================================================================================//
//                   class MarkerObserver : public Observer<Marker*>                   //
//=====================================================================================//
// ����������� �� ��������
class MarkerObserver : public Observer<Marker*>
{
public:
    enum event_type
	{
        EV_NONE,
        EV_VISIBLE_FOR_SPECTATOR,			// ��� ������� �����������
        EV_INVISIBLE_FOR_SPECTATOR,			// �� ����� �� ���� ������ �����������
		EV_VISIBILITY_FOR_PLAYER_CHANGED	// �� �������/��������� �� ������� �������� ������
    };

    struct spectator_info
	{
        Spectator* m_spectator;
        spectator_info(Spectator* sp) : m_spectator(sp) {}
    };
};
