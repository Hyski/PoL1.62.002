#pragma once

//=====================================================================================//
//                                    class Station                                    //
//=====================================================================================//
struct Station
{
	bool HasStation;//объект останавливается 
	float StationAt;//остановка в этой точке анимации
	int  TurnsToGo; //Через сколько туров поедем. динамически меняется
	int  StopTurns; //сколько туров длиться остановка
	int  OutTurns; //на сколько туров изчезает
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
