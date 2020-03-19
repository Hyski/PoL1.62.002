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
	/// получить базовый путь до звуковых скриптов, напр. "/scripts/sound/"
	virtual const std::string& getScriptBasePath() = 0;
	/// получить базовый путь до звуков, напр. "/sounds/"
	virtual const std::string& getSoundBasePath() = 0;
	///	вывести отладочную информацию
	virtual void dbg_printf(short,short,const char *) = 0;
	///	получить файловую систему
	virtual ci_VFileSystem* fs() = 0;
	///	получить дескриптор окна
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
// Хэндл на скрипт источника звука
class IScript
{
protected:
	virtual ~IScript() {}

public:
	/// Возвращает информацию о скрипте
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

	/// Проиграть звук
	virtual int play() = 0;
	/// Проиграть звук
	virtual int play(HScript script, const std::string &sound, float startSeconds) = 0;

	/// Остановить все звуки в эмиттере
	virtual void stop() = 0;

	/// Установить позицию звука
	virtual void setPosition(const snd_vector &) = 0;
	/// Установить позицию звука
	virtual void setVelocity(const snd_vector &) = 0;
	/// Установить ориентацию конуса
	virtual void setConeOrientation(const snd_vector &) = 0;
	/// Установить громкость эмиттера
	virtual void setVolume(float) = 0;

	/// Возвращает состояние источника, проигрывает ли он на данный момент звук
	virtual bool isPlaying() = 0;

	/// Возвращает "валидность" эмиттера, т. е. правильный ли в нем скрипт и вавчик
	virtual bool isValid() const = 0;
};

//=====================================================================================//
//                                 class IThemeSession                                 //
//=====================================================================================//
class IThemeSession
{
public:
	virtual ~IThemeSession() {}

	/// Сменить музыкальную тему
	virtual void changeTheme(HScript script, const std::string &newTheme) = 0;
	virtual void changeTheme(HScript script) = 0;

	/// Запаузить тему
	virtual void pause() = 0;
	/// Возобновить проигрывание темы
	virtual void resume() = 0;

	void changeTheme(const std::string &scriptName, const std::string &newTheme);
	void changeTheme(const std::string &scriptName);

	/// Включить/выключить музыкальную тему
	virtual void mute(bool bMute = true) = 0;
};

//=====================================================================================//
//                     class ci_Sound : public ci_CommonInterface                      //
//=====================================================================================//
class MUFFLE_SPEC ISound
{
public:
	/// Список возможных каналов
	/// Громкость канала равна Master*ChannelVolume
	/// Не трогать порядок каналов!!! cMaster всегда должен быть в начале!
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

	/// Начать музыкальную сессию
	virtual HThemeSession beginSession() = 0;

	/// Создать источник звука
	virtual HEmitter createEmitter(Channel) = 0;
	virtual HEmitter createEmitter(HScript script) = 0;
	virtual HEmitter createEmitter(HScript script, const std::string& waveFile) = 0;
	HEmitter createEmitter(const std::string &scriptName);
	HEmitter createEmitter(const std::string &scriptName, const std::string &waveFile);

	/// Возвращает длительность звука в секундах
	virtual float getSoundDuration(const std::string &sound) = 0;

	/// Проиграть звук. После запуска звука с помощью этих функций 
	/// его никак проконтролировать нельзя. Если звук зациклен, то он будет
	/// играть вечно.
	virtual void emitSound(HScript script,const std::string& waveFile, float startSeconds = 0.0f) = 0;
	virtual void emitSound(HScript script, float startSeconds = 0.0f) = 0;
	void emitSound(const std::string &scriptName, const std::string &waveFile, float startSeconds = 0.0f);
	void emitSound(const std::string &scriptName, float startSeconds = 0.0f);

	/// Возвращает указатель на скрипт
	virtual HScript getScript(const std::string& scriptName) = 0;
	/// Внедрить скрипт в Muffle
	virtual HScript embedScript(const ScriptInfo &) = 0;

	/// Установить громкость звука для определенного канала
	virtual void setVolume(Channel, float volume) = 0;
	virtual void setFrequency(Channel, float factor) = 0;

	/// Получить громкость звука для определенного канала
	virtual float getVolume(Channel) const = 0;

	/// Включить/выключить определенный канал
	virtual void muteChannel(Channel, bool) = 0;

	/// Изменить позицию камеры
	virtual void setCamera(const snd_vector &dir,
						   const snd_vector &up,
						   const snd_vector &right,
						   const snd_vector &origin) = 0;

	/// Свойства среды
	/// ...

	/// Статические функции для инициализации звука извне каркаса

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
	std::string name;			///< Название скрипта
	std::string sound;			///< Имя файла со звуком
	ISound::Channel channel;	///< Канал воспроизведения

	bool looped;				///< Зацикленный ли звук
	bool is3d;					///< Трехмерный ли звук

	float min, max;				///< Минимальная и максимальная дистанции слышимости
	float minAngle, maxAngle;	///< Минимальный и максимальный углы конуса слышимости
	float outerVolume;			///< Громкость за пределами конуса

	float fadeTime;				///< Время фэйда в секундах. Если значение отрицательно,
								///< то берется значение по умолчанию

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