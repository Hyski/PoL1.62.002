/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: классы эффектов и класс менеджера эффектов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "Effect.h"
#include <undercover/game.h>
#include <Muffle/ISound.h>

#if defined (EFFECTS_DEBUG) //for log
CLog LOG_CLASS;
#endif // for log

//
// класс FixedSound - для эффекта, который проигрывает неподвижный звук
//
FixedSound::FixedSound(const std::string& Name, const point3& Target)
{
	static Muffle::HScript script;

	if(!script)
	{
		script = Muffle::ISound::instance()->getScript("amb_onetime");
	}

	if(Game::RenderEnabled())
	{
		std::string filename("sounds/");
		filename += Name + ".wav";
		Muffle::HEmitter emitter = Muffle::ISound::instance()->createEmitter(script,filename.c_str());
		emitter->setPosition(Target.as_mll_vector());
		emitter->play();
	}
}


//
// класс ActiveSound - для эффекта, который проигрывает подвижный звук
//

//=====================================================================================//
//                             void ActiveSound::Update()                              //
//=====================================================================================//
// звук подвижный, так что Update должна изменить координаты и скорость
void ActiveSound::Update(const point3& Position, const point3& Velocity)
{
	pos = Position;
	vel = Velocity;
	emitter->setPosition(pos.as_mll_vector());
	emitter->setVelocity(vel.as_mll_vector());
}

//=====================================================================================//
//                             ActiveSound::ActiveSound()                              //
//=====================================================================================//
ActiveSound::ActiveSound(const std::string& Name)
{
	static Muffle::HScript script;

	if(!script)
	{
		script = Muffle::ISound::instance()->getScript("looping");
	}

	std::string filename("sounds/");
	filename += Name + ".wav";
	emitter = Muffle::ISound::instance()->createEmitter(script,filename.c_str());
	emitter->play();
}

//=====================================================================================//
//                             ActiveSound::~ActiveSound()                             //
//=====================================================================================//
ActiveSound::~ActiveSound()
{
	emitter->stop();
}

///////////////////////////////////////////////////////////////////////////////
//
// пространство имен для утилит эффектов
//
///////////////////////////////////////////////////////////////////////////////
namespace EffectUtility
{
	// получить случайное число в диапазоне от 0 до 1 (float)
	float EffectUtility::get0F()
	{
		return (static_cast<float>(rand()))*0.000030518509f;
	}
	// получить случайное число в диапазоне от 0 до f (float)
	float EffectUtility::get0F(float f)
	{
		return (static_cast<float>(rand()))*0.000030518509f*f;
	}

	
	// получить случайное число в диапазоне от -1 до 1
	float EffectUtility::get_FF()
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f);
	}
	// получить случайное число в диапазоне от -f до f
	float EffectUtility::get_FF(float f)
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f)*f;
	}
};

