#pragma once

#include "ActMgr.h"

namespace PoL
{

//=====================================================================================//
//                                 class TaskDatabase                                  //
//=====================================================================================//
class TaskDatabase
{
public:
	class Connection;
	friend class Connection;

	enum MessageType
	{
		mtKilled,
		mtDead,
		mtItemUsed,
		mtPersonUsed,
		mtItemDestroyed,
		mtEndTurn,
		mtStartTurn,
		mtEntitySpawned,
		mtNamedEntitySpawned,
		mtHeroTeamChanged,
		mtEpisodeChanged,
		mtLeaveLevel,
		mtEnterLevel,
		mtPhaseChanged,
		mtEntityDestroyed,
		mtRelationChanged
	};

private:
	typedef std::hash_map<std::string,int> ItemList_t;
	typedef std::hash_map<std::string,point3> DeadPositions_t;

	ItemList_t m_deadList, m_killedList, m_usedList;
	DeadPositions_t m_deadPositions;

	struct HandlerData
	{
		typedef boost::function<void ()> Fn_t;

		Fn_t m_fn;
		mutable bool m_enabled;
		mutable int m_priority;
		std::string m_subj;
		MessageType m_mt;

		bool operator<(const HandlerData &r) const
		{
			if(m_mt == r.m_mt) return m_subj < r.m_subj;
			return m_mt < r.m_mt;
		}
	};

	typedef std::multiset<HandlerData> Handlers_t;
	typedef std::set< ::eid_t > SaidList_t;
	Handlers_t m_handlers;

	EventData m_lastMsg;
	bool m_processingMessage;
	bool m_processingUseMessage;
	int m_messageCount;

	SaidList_t m_saidList;

public:
	TaskDatabase();
	~TaskDatabase();

	/// Прицепить наблюдателя на событие
	Connection attach(HandlerData::Fn_t fn, MessageType t);
	Connection attach(HandlerData::Fn_t fn, const std::string &, MessageType t);

	/// Вызвать при использовании игроком объекта
	void onUseObject(const EventData &);
	/// Вызвать при использовании игроком сущности
	void onUseEntity(const EventData &);
	/// Вызвать в момент убийства сущности
	void onKillEntity(const EventData &);
	/// Вызвать в конце хода
	void onEndTurn(const EventData &);
	/// Вызвать в начале хода
	void onStartTurn(const EventData &);
	/// Вызвать при появлении сущности
	void onEntitySpawned(const EventData &);
	/// Вызвать при изменении команды, в которой состоит сущность
	void onEntityTeamChanged(const EventData &);
	/// Вызвать при смене эпизода
	void onEpisodeChanged(const EventData &);
	/// Вызвать при выходе с уровня
	void onLeaveLevel(const EventData &);
	/// Вызвать при входе на уровень
	void onEnterLevel(const EventData &);
	/// Вызвать при изменении фазы
	void onPhaseChange(const EventData &);
	/// Вызвать при уничтожении сущности
	void onEntityDestroyed(const EventData &);
	/// Вызвать при изменении отношений
	void onRelationChanged(const EventData &);

	/// Добавить чувака в список тех, кто что-то сказал
	void addToSaidList( ::eid_t id );
	/// Находится ли чувак в списке тех, кто что-то сказал
	bool isInSaidList( ::eid_t id ) const;

	/// Мертв ли персонаж
	bool isDead(const std::string &n) const
	{
		ItemList_t::const_iterator i = m_deadList.find(n);
		return i != m_deadList.end() && i->second;
	}

	/// Убит ли персонаж игроком
	bool isKilled(const std::string &n) const
	{
		ItemList_t::const_iterator i = m_killedList.find(n);
		return i != m_killedList.end() && i->second;
	}

	/// Говорили ли с персонажем
	bool isPersonUsed(const std::string &n) const
	{
		ItemList_t::const_iterator i = m_usedList.find(n);
		return i != m_usedList.end() && i->second;
	}

	bool getDeadEntityPos(const std::string &name, point3 *pos);

	/// Возвращает последнее принятое сообщение
	bool isProcessingMessage() const { return m_processingMessage; }
	const EventData &getMessageInProcess() const { return m_lastMsg; }
	int getMessageCount() const { return m_messageCount; }

	/// Сохранить состояние базы
	void store(mll::io::obinstream &) const;
	/// Восстановить состояние базы
	void restore(mll::io::ibinstream &);

	/// Сбросить состояние на начальное
	void reset();

	static TaskDatabase *instance();

private:
	void clearSaidList();
	void doNotify(const HandlerData &);

	void notify(MessageType mt);
	void notify(const std::string &n, MessageType mt);

	void sayRandomPhrase(eid_t);
};

//=====================================================================================//
//                           class TaskDatabase::Connection                            //
//=====================================================================================//
class TaskDatabase::Connection
{
	friend class TaskDatabase;
	Handlers_t::iterator m_itor;

	Connection(Handlers_t::iterator i) : m_itor(i) {}

	Handlers_t::iterator end() const { return TaskDatabase::instance()->m_handlers.end(); }

public:
	Connection() : m_itor(end()) {}
	Connection(Connection &c) : m_itor(c.m_itor) { c.m_itor = end(); }
	~Connection()
	{
		if(m_itor != end())
		{
			TaskDatabase::instance()->m_handlers.erase(m_itor);
		}
	}

	Connection &operator=(Connection &c)
	{
		Connection copy(c);
		swap(copy);
		return *this;
	}

	void swap(Connection &c)
	{
		std::swap(m_itor,c.m_itor);
	}

	void enable(bool v)
	{
		if(m_itor != end())
		{
			m_itor->m_enabled = v;
		}
	}

	void setPriority(int p)
	{
		if(m_itor != end())
		{
			m_itor->m_priority = p;
		}
	}

	bool isEnabled() const
	{
		return m_itor != end() && m_itor->m_enabled;
	}
};

}