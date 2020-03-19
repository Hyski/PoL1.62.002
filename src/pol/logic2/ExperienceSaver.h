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

	Implants_t m_implants;		///< ������ ������������
	ThingTags_t m_thingTags;	///< ������ ����� ����������

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
	/// ���������� ��������� ������
	static ExperienceSaver *inst();

	/// ��������� (��)������������
	void MakeSaveLoad(SavSlot &slot);
	/// �������� ���������
	void reset();

	/// ���������� �� ���������� � �����
	bool hasHeroInformation(const std::string &name) const;
	/// ���������� ���������� � �����
	const ExperienceInfo &getExperienceInfo(const std::string &name) const;
	/// ��������� ���������� �� �����
	void storeExperienceInfo(HumanEntity *entity);
};

}
