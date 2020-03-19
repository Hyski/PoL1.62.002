/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   18.07.2001

************************************************************************/
#include "Precomp.h"
#include <Muffle/ISound.h>
#include "WidgetSoundMgr.h"

WidgetSoundMgr::Deleter WidgetSoundMgr::m_Deleter;
WidgetSoundMgr::WidgetSoundMgr() : m_Player(new Player)
{
}

WidgetSoundMgr::~WidgetSoundMgr()
{
	delete m_Player;
}
//	создать сэмпл 
WidgetSoundMgr::Sample* WidgetSoundMgr::CreateSample(const char* name)
{
	static std::map<std::string,Reference*>::iterator i;

	i = m_Samples.find(name);
	if(i == m_Samples.end())
	{
		m_Samples.insert(std::map<std::string,Reference*>::value_type(name,new Reference(new Sample(name))));
		i = m_Samples.find(name);
	}

	return i->second->Get();
}
//	создать сэмпл 
WidgetSoundMgr::Sample* WidgetSoundMgr::CreateSample(const char* name,const char* description, float vol)
{
	static std::map<std::string,Reference*>::iterator i;

	i = m_Samples.find(name);
	if(i == m_Samples.end())
	{
		m_Samples.insert(std::map<std::string,Reference*>::value_type(name,new Reference(new Sample(description,name,vol))));
		i = m_Samples.find(name);
	}

	return i->second->Get();
}
//	уменьшить ссылку на сэмпл
void WidgetSoundMgr::Release(WidgetSoundMgr::Sample* sample)
{
	std::map<std::string,Reference*>::iterator i;

	i = m_Samples.find(*sample->Description());
	if(i!=m_Samples.end())
	{
		if(!i->second->Release())
		{
			//	останавливаем сэмпл
			m_Player->Stop(i->second->Get());
			//	уничтожаем сэмпл
			delete i->second;
			m_Samples.erase(i);
		}
	}
}
//	очистить все
void WidgetSoundMgr::Release(void)
{
	for(std::map<std::string,Reference*>::iterator i = m_Samples.begin();i!=m_Samples.end();i++)
	{
		delete i->second;
	}
	m_Samples.clear();
}

//	обновить громкость звуков
void WidgetSoundMgr::UpdateVolume(void)
{
	/* by Flif
	for(std::map<std::string,Reference*>::iterator i = m_Samples.begin();i!=m_Samples.end();i++)
	{
		WidgetSoundMgr::Sample* sample = i->second->Get();
		i->second->Release();
	}
	*/
}

//***************************************************************//
//	WidgetSoundMgr::Sample

//=====================================================================================//
//                          WidgetSoundMgr::Sample::Sample()                           //
//=====================================================================================//
WidgetSoundMgr::Sample::Sample(const char* description)
:	m_Description(new std::string(description)),
	m_Name(0),
	m_volume(1.0f)
{
	// Получим указатель на звуковой скрипт
	m_Script = Muffle::ISound::instance()->getScript(description);
}

//=====================================================================================//
//                          WidgetSoundMgr::Sample::Sample()                           //
//=====================================================================================//
WidgetSoundMgr::Sample::Sample(const char* description,const char* name, float vol)
:	m_Description(new std::string(description)),
	m_Name(new std::string(name)),
	m_volume(vol)
{
	// Получим указатель на звуковой скрипт
	m_Script = Muffle::ISound::instance()->getScript(description);
}

//=====================================================================================//
//                          WidgetSoundMgr::Sample::~Sample()                          //
//=====================================================================================//
WidgetSoundMgr::Sample::~Sample()
{
	if(m_Description) delete m_Description;
	if(m_Name) delete m_Name;
}

//=====================================================================================//
//                       void WidgetSoundMgr::Sample::Release()                        //
//=====================================================================================//
void WidgetSoundMgr::Sample::Release(void)
{
	WidgetSoundMgr::Instance()->Release(this);
}

//***************************************************************//
//	class WidgetSoundMgr::Reference
WidgetSoundMgr::Reference::Reference(WidgetSoundMgr::Sample* value)
{
	m_Value = value;
	m_Counter = 0;
}

WidgetSoundMgr::Reference::~Reference()
{
	if(m_Value)
		delete m_Value;
}

//***************************************************************//
//	class WidgetSoundMgr::Player
//=====================================================================================//
//                          WidgetSoundMgr::Player::Player()                           //
//=====================================================================================//
WidgetSoundMgr::Player::Player()
{
}

//=====================================================================================//
//                          WidgetSoundMgr::Player::~Player()                          //
//=====================================================================================//
WidgetSoundMgr::Player::~Player()
{
}

//=====================================================================================//
//                         void WidgetSoundMgr::Player::Play()                         //
//=====================================================================================//
void WidgetSoundMgr::Player::Play(Widget::Player::Sample* sample)
{
	if(sample)
	{
		if (!Muffle::ISound::instance()) return;

		WidgetSoundMgr::Sample *casted = static_cast<WidgetSoundMgr::Sample*>(sample);

		if(!casted->Emitter().get())
		{
			if(!casted->Name())
			{
				casted->Emitter() = Muffle::ISound::instance()->createEmitter(casted->Script());
			}
			else
			{
				casted->Emitter() = Muffle::ISound::instance()->createEmitter(casted->Script(),*casted->Name());
			}
		}

		casted->Emitter()->play();
		casted->Emitter()->setVolume(casted->Volume());
	}
}

//=====================================================================================//
//                         void WidgetSoundMgr::Player::Stop()                         //
//=====================================================================================//
void WidgetSoundMgr::Player::Stop(Widget::Player::Sample* sample)
{
	if(sample)
	{
		if(!Muffle::ISound::instance()) return;

		WidgetSoundMgr::Sample *casted = static_cast<WidgetSoundMgr::Sample*>(sample);

		if(casted->Emitter().get())
		{
			casted->Emitter()->stop();
			casted->Emitter().reset();
		}
	}
}
