#pragma once

class Marker;
class Spectator;

//=====================================================================================//
//                class SpectatorObserver : public Observer<Spectator*>                //
//=====================================================================================//
// ����������� �� Spectator'��
class SpectatorObserver : public Observer<Spectator*>
{
public:
    enum event_type
	{
        EV_NONE,
        EV_VISIBLE_MARKER,   //������ ����� � ���� ���������
        EV_INVISIBLE_MARKER, //������ ����� �� ���� ���������
    };

    //���������� � ��������/�������� ��������
    struct marker_info
	{
        Marker* m_marker;
        marker_info(Marker* mk) : m_marker(mk) {}
    };
};
