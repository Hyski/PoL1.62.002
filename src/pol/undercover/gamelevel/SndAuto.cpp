#include "precomp.h"

#include "SndAuto.h"
#include <Muffle/ISound.h>

//class ISndEmitter;

static const std::string Prefix("sounds/"),Suffix(".wav");

//=====================================================================================//
//                                 class PositionesSnd                                 //
//=====================================================================================//
class PositionesSnd
{
public:
	Muffle::HEmitter m_Emitter;
	point3 Pos;
	point3 Vel;

public:
	PositionesSnd() : m_Emitter(0)
	{
	}

	~PositionesSnd()
	{
		if(m_Emitter.get())
		{
			m_Emitter->stop();
			m_Emitter.reset();
		}
	}

	void SetPos(const point3 &pos)
	{
		if(m_Emitter.get())
		{
			Pos = pos;
			m_Emitter->setPosition(pos.as_mll_vector());
		}
	}

	const point3& GetPos()
	{
		return Pos;
	}

	const point3& GetVel()
	{
		return Vel;
	}

	void StopAndRelease()
	{
		if(m_Emitter.get())
		{
			m_Emitter->stop();
			m_Emitter.reset();
		}
	}
};

//=====================================================================================//
//                                 SndAuto::SndAuto()                                  //
//=====================================================================================//
SndAuto::SndAuto(const std::string names[5])
{
//	m_started = false;
	for(int i=0; i<5; i++)
	{
		m_names[i]=names[i];//std::string("sounds/")+names[i]+".wav";
		m_utter[i].reset(new PositionesSnd);
	}

	m_state = CLOSE;
	m_pos = point3(0.0f,0.0f,0.0f);

	m_script = Muffle::ISound::instance()->getScript("lodinamic");
	m_cycledScript = Muffle::ISound::instance()->getScript("lodinamiccycled");
	m_ambientScript = Muffle::ISound::instance()->getScript("loambient");
}

//=====================================================================================//
//                                 SndAuto::~SndAuto()                                 //
//=====================================================================================//
SndAuto::~SndAuto()
{
}

//=====================================================================================//
//                               void SndAuto::SetPos()                                //
//=====================================================================================//
void SndAuto::SetPos(const point3 &_pos)
{
	m_pos = _pos;

	for(int i=0; i<5; i++)
	{
		m_utter[i]->SetPos(m_pos);
	}
}

//=====================================================================================//
//                              void SndAuto::SetState()                               //
//=====================================================================================//
void SndAuto::SetState(STATE state)
{
	if(m_state!=state)
	{
		m_utter[m_state]->StopAndRelease();
		m_state = state;

		if(m_state != AMBIENT && !m_names[m_state].empty())
		{
			std::string h = Prefix + m_names[m_state] + Suffix;
			Muffle::HScript snd_script = ((m_state==CLOSE||m_state==OPEN)?m_cycledScript:m_script);
			m_utter[m_state]->m_Emitter = Muffle::ISound::instance()->createEmitter(snd_script,h.c_str());
			m_utter[m_state]->SetPos(m_pos);
			m_utter[m_state]->m_Emitter->play();
		}
	}
}

//=====================================================================================//
//                                void SndAuto::Start()                                //
//=====================================================================================//
void SndAuto::Start()
{
	if(!m_names[m_state].empty()) 
	{
		std::string h = Prefix + m_names[m_state] + Suffix;
		Muffle::HScript snd_script = ((m_state==CLOSE||m_state==OPEN)?m_cycledScript:m_script);
		m_utter[m_state]->m_Emitter = Muffle::ISound::instance()->createEmitter(snd_script,h.c_str());
		m_utter[m_state]->SetPos(m_pos);
		m_utter[m_state]->m_Emitter->play();
	}
}

//=====================================================================================//
//                                void SndAuto::Stop()                                 //
//=====================================================================================//
void SndAuto::Stop()
{
	for(int i=0; i<5; i++)
	{
		m_utter[i]->StopAndRelease();
	}
}
