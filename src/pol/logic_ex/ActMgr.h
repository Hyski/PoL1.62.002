#pragma once

#include "FlexibleSSForward.h"

namespace PoL
{

//=====================================================================================//
//                                  struct EventData                                   //
//=====================================================================================//
struct EventData
{
	std::string m_level;		///< Уровень, на котором происходит событие
	unsigned int m_phase;		///< Фаза
	unsigned int m_episode;		///< Эпизод
	std::string m_object;		///< Объект события, тот, кто действует
	std::string m_subject;		///< Субъект события, тот, на которого направлено действие

	/// Уникальные идентификаторы верны только тогда, когда они представляют сущности
	::eid_t m_object_id;		///< Уникальный идентификатор объекта
	::eid_t m_subject_id;		///< Уникальный идентификатор субъекта

	EventData() : m_object_id(0), m_subject_id(0) {}
};

//=====================================================================================//
//                                    class ActMgr                                     //
//=====================================================================================//
class ActMgr
{
public:
	/// Возвращает true, если сейчас играется скриптовая сцена
	static bool isPlayingScene();
	/// Возвращает true, если сейчас находимся в play
	static bool isInPlay();
	/// Тик для проигрывания сцены
	static void play();
	/// Начать проигрывать сцену
	static void start(HAct);

	/// Сбросить в лог состояние всех объектов сюжета
	static void dumpState();

	/// Вызвать при использовании игроком объекта
	static void onUseObject(const EventData &);
	/// Вызвать при использовании игроком сущности
	static void onUseEntity(const EventData &);
	/// Вызвать в момент убийства сущности
	static void onKillEntity(const EventData &);
	/// Вызвать при старте уровня
	static void onLevelEnter(const EventData &);
	/// Вызвать при выходе с уровня
	static void onLevelExit(const EventData &);
	/// Вызвать при старте игры
	static void onStartGame(const EventData &);
	/// Вызвать при переходе на новый эпизод
	static void onStartEpisode(const EventData &);
	/// Вызвать при рождении сущности
	static void onEntitySpawned(const EventData &);
	/// Вызвать при смене фазы
	static void onPhaseChanged(const EventData &);
	/// Вызывать при изменении команды персонажа
	static void onEntityTeamChanged(const EventData &);
	/// Вызывать при начале хода игрока
	static void onStartTurn(const EventData &);
	/// Вызывать при окончании хода игрока
	static void onEndTurn(const EventData &);
	/// Вызывать при окончании хода игрока
	static void onDestroyEntity(const EventData &);
	/// Вызывать при окончании хода игрока
	static void onRelChanged(const EventData &);

	/// Сохранить состояние скриптовых сцен
	static void store(std::ostream &);
	/// Восстановить состояние скриптовых сцен
	static void restore(std::istream &, unsigned int );

	/// Сбросить на диск параметры текущей версии скриптов
	static void dumpScriptParameters();

	/// Для тестирования различных фич
	static void test();
	static void test(const std::string &);
	static void reset();
	
	static void cleanup();
};

}