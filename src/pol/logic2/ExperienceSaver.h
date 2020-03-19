#pragma once

class SavSlot;
class HumanEntity;

namespace PoL
{

//=====================================================================================//
//                                struct ExperienceInfo                                //
//=====================================================================================//
struct ExperienceInfo
{
	struct ThingTag
	{
		human_pack_type m_index;
		std::string m_data;
	};

	typedef std::vector<std::string> Implants_t;
	typedef std::vector<ThingTag> ThingTags_t;

	Implants_t m_implants;		///< Список имплантантов
	ThingTags_t m_thingTags;	///< Список всего остального

    int m_health;
    int m_wisdom;
    int m_morale;
    int m_accuracy;
    int m_strength;
    int m_reaction;
    int m_mechanics;
    int m_dexterity;
	int m_experience;
	int m_levelup_points;
	int m_level;
	int m_rank;
    float m_front_radius;
};

//=====================================================================================//
//                                class ExperienceSaver                                //
//=====================================================================================//
class ExperienceSaver
{
	typedef std::hash_map<std::string,ExperienceInfo> Infos_t;
	Infos_t m_infos;

public:
	/// Возвращает экземпляр класса
	static ExperienceSaver *inst();

	/// Выполнить (де)сериализацию
	void MakeSaveLoad(SavSlot &slot);
	/// Сбросить состояние
	void reset();

	/// Существует ли информация о герое
	bool hasHeroInformation(const std::string &name) const;
	/// Возвращает информацию о герое
	const ExperienceInfo &getExperienceInfo(const std::string &name) const;
	/// Сохранить информацию об опыте
	void storeExperienceInfo(HumanEntity *entity);
};

}
