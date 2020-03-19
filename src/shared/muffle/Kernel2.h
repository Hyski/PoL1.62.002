#pragma once

#include "dsound.h"
#include "Worker2.h"
#include "filemgmt.h"
#include "ScriptMgr.h"
#include "ThemeManager2.h"
#include "primarybuffer.h"
#include "Synchronized2.h"
#include "BufferCtrlMgr2.h"
#include "PlayingSoundMgr2.h"

namespace Muffle2
{

//=====================================================================================//
//                                    class Kernel                                     //
//=====================================================================================//
class Kernel : public Synchronized
{
	Muffle::SndServices *m_ss;
	Muffle::DirectSound m_directSound;
	Muffle::PrimaryBuffer m_primaryBuffer;
	Worker m_worker;
	Muffle::HFileMgmt m_fileMgmt;
	Muffle::ScriptMgr m_scriptMgr;
	HPlayingSoundMgr m_psmgr;
	BufferCtrlMgr m_bufCtrlMgr;
	ThemeManager m_themeMgr;

public:
	Kernel(Muffle::SndServices *ss);
	~Kernel();

	/// Создать эмиттер
	Muffle::HEmitter createEmitter(Muffle::ISound::Channel);
	Muffle::HEmitter createEmitter(Muffle::HScript script, const std::string &file);
	/// Возвращает указатель на скрипт
	Muffle::HScript getScript(const std::string& scriptName);
	/// Внедрить скрипт в ScriptMgr
	Muffle::HScript embedScript(const Muffle::ScriptInfo &info);
	/// Установить позицию камеры
	void setCamera(const vec &dir, const vec &up, const vec &right, const vec &origin);
	/// Создать новую музыкальную сессию
	Muffle::HThemeSession beginSession();


	/// Возвращает IDirectSound8
	IDirectSound8 *getDirectSound() { return m_directSound.dsound(); }
	/// Возвращает префикс пути к звукам
	const std::string &getSoundPathPrefix() const { return m_ss->getSoundBasePath(); }

	/// Возвращает экземпляр Kernel
	static Kernel *inst();
};

}
