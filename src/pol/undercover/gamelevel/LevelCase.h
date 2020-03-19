#pragma once

//=====================================================================================//
//                                   class LevelCase                                   //
//=====================================================================================//
class LevelCase
{
	int m_wisdom;	///< Интеллект, необходимый для использования
	int m_weight;	///
	int m_respawn;
	int m_exp;
	std::string m_setName, m_objName;
	bool m_isStorage, m_db, m_elevator;

public:
	LevelCase()
	:	m_wisdom(0),
		m_weight(0),
		m_respawn(0),
		m_exp(0),
		m_isStorage(false),
		m_db(false),
		m_elevator(false)
	{
	}

	//это простой ящик
	bool IsStorage() const { return m_isStorage; }
	//это база данных
	bool IsDatabase() const { return m_db; }
	//узнать символьную метку ящика
	const std::string& GetName() const { return m_objName; }
	//узнать название набора предметов
	const std::string& GetItems() const { return m_setName; }    
	//является ли лифтом
	bool IsElevator() const { return m_elevator; }
	int GetWisdom() const { return m_wisdom; }
	int GetWeight() const { return m_weight; }
	int GetRespawn() const { return m_respawn; }
	int GetExperience() const { return m_exp; }

	void ReadStorageInfo(const std::string &info);
	void ReadDatabaseInfo(const std::string &info);

	void SetObjName(const std::string &name) { m_objName = name; }

	/// Убрать этот метод немедленно!!!
	void SetElevator(bool v) { m_elevator = v; }
};
