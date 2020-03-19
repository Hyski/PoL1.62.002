//=====================================================================================//
//                             Sound interface definitions                             //
//                             Copyright by MiST land 2002                             //
//=====================================================================================//
//  Description:                                                                       //
//=====================================================================================//
//  Author: Sergei V. Zagursky aka GvozdodeR                                           //
//=====================================================================================//

#if !defined(__ISOUND_H_INCLUDED_2023399421164473__)
#define __ISOUND_H_INCLUDED_2023399421164473__

namespace Muffle
{

class SndServices;
class IScript;
class IEmitter;
class IThemeSession;
class ci_VFileSystem;

//=====================================================================================//
//                                 struct SndServices                                  //
//=====================================================================================//
class SndServices
{
public:
	virtual ~SndServices() {}
	/// �������� ������� ���� �� �������� ��������, ����. "/scripts/sound/"
	virtual const std::string& getScriptBasePath() = 0;
	/// �������� ������� ���� �� ������, ����. "/sounds/"
	virtual const std::string& getSoundBasePath() = 0;
	///	������� ���������� ����������
	virtual void dbg_printf(short,short,const char *) = 0;
	///	�������� �������� �������
	virtual ci_VFileSystem* fs() = 0;
	///	�������� ���������� ����
	virtual HWND window() = 0;

};
//=====================================================================================//

typedef mll::algebra::vector3 snd_vector;

#if !defined(__USE_STATIC_MUFFLE__)
#	if defined(__MUFFLE_EXPORTS__)
#		define MUFFLE_SPEC __declspec(dllexport)
#	else
#		define MUFFLE_SPEC __declspec(dllimport)
#	endif
#else
#	define MUFFLE_SPEC
#endif

struct ScriptInfo;

//=====================================================================================//
//                                    class IScript                                    //
//=====================================================================================//
// ����� �� ������ ��������� �����
class IScript
{
protected:
	virtual ~IScript() {}

public:
	/// ���������� ���������� � �������
	virtual const Muffle::ScriptInfo &getInfo() const = 0;
};

typedef const IScript *HScript;
typedef std::auto_ptr<IEmitter> HEmitter;
typedef std::auto_ptr<IThemeSession> HThemeSession;

//=====================================================================================//
//                                   class IEmitter                                    //
//=====================================================================================//
class IEmitter
{
public:
	virtual ~IEmitter() {}

	/// ��������� ����
	virtual int play() = 0;
	/// ��������� ����
	virtual int play(HScript script, const std::string &sound, float startSeconds) = 0;

	/// ���������� ��� ����� � ��������
	virtual void stop() = 0;

	/// ���������� ������� �����
	virtual void setPosition(const snd_vector &) = 0;
	/// ���������� ������� �����
	virtual void setVelocity(const snd_vector &) = 0;
	/// ���������� ���������� ������
	virtual void setConeOrientation(const snd_vector &) = 0;
	/// ���������� ��������� ��������
	virtual void setVolume(float) = 0;

	/// ���������� ��������� ���������, ����������� �� �� �� ������ ������ ����
	virtual bool isPlaying() = 0;

	/// ���������� "����������" ��������, �. �. ���������� �� � ��� ������ � ������
	virtual bool isValid() const = 0;
};

//=====================================================================================//
//                                 class IThemeSession                                 //
//=====================================================================================//
class IThemeSession
{
public:
	virtual ~IThemeSession() {}

	/// ������� ����������� ����
	virtual void changeTheme(HScript script, const std::string &newTheme) = 0;
	virtual void changeTheme(HScript script) = 0;

	/// ��������� ����
	virtual void pause() = 0;
	/// ����������� ������������ ����
	virtual void resume() = 0;

	void changeTheme(const std::string &scriptName, const std::string &newTheme);
	void changeTheme(const std::string &scriptName);

	/// ��������/��������� ����������� ����
	virtual void mute(bool bMute = true) = 0;
};

//=====================================================================================//
//                     class ci_Sound : public ci_CommonInterface                      //
//=====================================================================================//
class MUFFLE_SPEC ISound
{
public:
	/// ������ ��������� �������
	/// ��������� ������ ����� Master*ChannelVolume
	/// �� ������� ������� �������!!! cMaster ������ ������ ���� � ������!
	enum Channel
	{
		cMaster,
		cThemes,
		cSpeech,
		cEffects,
		cMenu,
		cAmbient,
		cDebug,
		cChannelCount
	};

	virtual void manage() = 0;

	/// ������ ����������� ������
	virtual HThemeSession beginSession() = 0;

	/// ������� �������� �����
	virtual HEmitter createEmitter(Channel) = 0;
	virtual HEmitter createEmitter(HScript script) = 0;
	virtual HEmitter createEmitter(HScript script, const std::string& waveFile) = 0;
	HEmitter createEmitter(const std::string &scriptName);
	HEmitter createEmitter(const std::string &scriptName, const std::string &waveFile);

	/// ���������� ������������ ����� � ��������
	virtual float getSoundDuration(const std::string &sound) = 0;

	/// ��������� ����. ����� ������� ����� � ������� ���� ������� 
	/// ��� ����� ����������������� ������. ���� ���� ��������, �� �� �����
	/// ������ �����.
	virtual void emitSound(HScript script,const std::string& waveFile, float startSeconds = 0.0f) = 0;
	virtual void emitSound(HScript script, float startSeconds = 0.0f) = 0;
	void emitSound(const std::string &scriptName, const std::string &waveFile, float startSeconds = 0.0f);
	void emitSound(const std::string &scriptName, float startSeconds = 0.0f);

	/// ���������� ��������� �� ������
	virtual HScript getScript(const std::string& scriptName) = 0;
	/// �������� ������ � Muffle
	virtual HScript embedScript(const ScriptInfo &) = 0;

	/// ���������� ��������� ����� ��� ������������� ������
	virtual void setVolume(Channel, float volume) = 0;
	virtual void setFrequency(Channel, float factor) = 0;

	/// �������� ��������� ����� ��� ������������� ������
	virtual float getVolume(Channel) const = 0;

	/// ��������/��������� ������������ �����
	virtual void muteChannel(Channel, bool) = 0;

	/// �������� ������� ������
	virtual void setCamera(const snd_vector &dir,
						   const snd_vector &up,
						   const snd_vector &right,
						   const snd_vector &origin) = 0;

	/// �������� �����
	/// ...

	/// ����������� ������� ��� ������������� ����� ����� �������

public:
	static ISound *instance();
	static void initInstance(SndServices *);
	static void shutInstance();
	static bool isInitialized();
};

//=====================================================================================//
//                                  struct ScriptInfo                                  //
//=====================================================================================//
struct ScriptInfo
{
	std::string name;			///< �������� �������
	std::string sound;			///< ��� ����� �� ������
	ISound::Channel channel;	///< ����� ���������������

	bool looped;				///< ����������� �� ����
	bool is3d;					///< ���������� �� ����

	float min, max;				///< ����������� � ������������ ��������� ����������
	float minAngle, maxAngle;	///< ����������� � ������������ ���� ������ ����������
	float outerVolume;			///< ��������� �� ��������� ������

	float fadeTime;				///< ����� ����� � ��������. ���� �������� ������������,
								///< �� ������� �������� �� ���������

	ScriptInfo()
	:	channel(ISound::cEffects),
		looped(false),
		is3d(true),
		min(5.0f),
		max(100.0f),
		minAngle(360.0f),
		maxAngle(360.0f),
		outerVolume(1.0f),
		fadeTime(-1.0f)
	{
	}
};

//=====================================================================================//
//                           inline void ISound::emitSound()                           //
//=====================================================================================//
inline void ISound::emitSound(const std::string &scriptName, const std::string &waveFile,
							  float startSeconds)
{
	emitSound(getScript(scriptName),waveFile,startSeconds);
}

//=====================================================================================//
//                          inline void ISound::emitSound();                           //
//=====================================================================================//
inline void ISound::emitSound(const std::string &scriptName, float startSeconds)
{
	emitSound(getScript(scriptName),startSeconds);
}

//=====================================================================================//
//                       inline HEmitter ISound::createEmitter()                       //
//=====================================================================================//
inline HEmitter ISound::createEmitter(const std::string &scriptName, const std::string &waveFile)
{
	return createEmitter(getScript(scriptName),waveFile);
}

//=====================================================================================//
//                       inline HEmitter ISound::createEmitter()                       //
//=====================================================================================//
inline HEmitter ISound::createEmitter(const std::string &scriptName)
{
	return createEmitter(getScript(scriptName));
}

//=====================================================================================//
//                      inline void IThemeSession::changeTheme()                       //
//=====================================================================================//
inline void IThemeSession::changeTheme(const std::string &scriptName, const std::string &newTheme)
{
	changeTheme(ISound::instance()->getScript(scriptName),newTheme);
}

//=====================================================================================//
//                      inline void IThemeSession::changeTheme()                       //
//=====================================================================================//
inline void IThemeSession::changeTheme(const std::string &scriptName)
{
	changeTheme(ISound::instance()->getScript(scriptName));
}

}	//	namespace Muffle

#undef MUFFLE_SPEC

#endif // !defined(__ISOUND_H_INCLUDED_2023399421164473__)