/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ �������� � ����� ��������� ��������
                                                                                
                                                                                
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
// ����� FixedSound - ��� �������, ������� ����������� ����������� ����
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
// ����� ActiveSound - ��� �������, ������� ����������� ��������� ����
//

//=====================================================================================//
//                             void ActiveSound::Update()                              //
//=====================================================================================//
// ���� ���������, ��� ��� Update ������ �������� ���������� � ��������
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
// ������������ ���� ��� ������ ��������
//
///////////////////////////////////////////////////////////////////////////////
namespace EffectUtility
{
	// �������� ��������� ����� � ��������� �� 0 �� 1 (float)
	float EffectUtility::get0F()
	{
		return (static_cast<float>(rand()))*0.000030518509f;
	}
	// �������� ��������� ����� � ��������� �� 0 �� f (float)
	float EffectUtility::get0F(float f)
	{
		return (static_cast<float>(rand()))*0.000030518509f*f;
	}

	
	// �������� ��������� ����� � ��������� �� -1 �� 1
	float EffectUtility::get_FF()
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f);
	}
	// �������� ��������� ����� � ��������� �� -f �� f
	float EffectUtility::get_FF(float f)
	{
		return (1 - 2.0f*(static_cast<float>(rand()))*0.000030518509f)*f;
	}
};

