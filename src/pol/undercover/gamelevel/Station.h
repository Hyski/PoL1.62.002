#pragma once

//=====================================================================================//
//                                    class Station                                    //
//=====================================================================================//
struct Station
{
	bool HasStation;//������ ��������������� 
	float StationAt;//��������� � ���� ����� ��������
	int  TurnsToGo; //����� ������� ����� ������. ����������� ��������
	int  StopTurns; //������� ����� ������� ���������
	int  OutTurns; //�� ������� ����� ��������
	int SwitchJoint;

	Station()
	:	HasStation(false),
		StationAt(0.0f),
		TurnsToGo(0),
		StopTurns(0),
		OutTurns(0),
		SwitchJoint(0)
	{
	}
};
