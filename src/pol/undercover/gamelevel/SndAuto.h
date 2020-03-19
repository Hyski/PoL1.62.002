#pragma once

#include <Muffle/ISound.h>

class PositionesSnd;

//=====================================================================================//
//                                    class SndAuto                                    //
//=====================================================================================//
class SndAuto
{
	std::auto_ptr<PositionesSnd> m_utter[5];
	std::string m_names[5];
	Muffle::HScript m_script, m_cycledScript, m_ambientScript;

public:
	enum STATE
	{
		AMBIENT, OPENING, CLOSING, OPEN, CLOSE
	};

private:
	STATE m_state;
	point3 m_pos;

public:
	SndAuto(const std::string names[5]);
	~SndAuto();

	void SetPos(const point3 &Pos);
	void SetState(STATE state);
	void Start();
	void Stop();
};
