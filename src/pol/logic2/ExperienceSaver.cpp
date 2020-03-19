#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "ExperienceSaver.h"

namespace PoL
{

//=====================================================================================//
//                      ExperienceSaver *ExperienceSaver::inst()                       //
//=====================================================================================//
ExperienceSaver *ExperienceSaver::inst()
{
	static ExperienceSaver es;
	return &es;
}

//=====================================================================================//
//                            void ExperienceSaver::reset()                            //
//=====================================================================================//
void ExperienceSaver::reset()
{
	m_infos.clear();
}

//=====================================================================================//
//                        void ExperienceSaver::MakeSaveLoad()                         //
//=====================================================================================//
void ExperienceSaver::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{
		slot << m_infos.size();

		for(Infos_t::const_iterator i = m_infos.begin(); i != m_infos.end(); ++i)
		{
			slot << i->first;
			slot << i->second.m_implants.size();

			for(ExperienceInfo::Implants_t::const_iterator ii = i->second.m_implants.begin(); ii != i->second.m_implants.end(); ++ii)
			{
				slot << *ii;
			}

			slot << i->second.m_health;
			slot << i->second.m_wisdom;
			slot << i->second.m_morale;
			slot << i->second.m_accuracy;
			slot << i->second.m_strength;
			slot << i->second.m_reaction;
			slot << i->second.m_mechanics;
			slot << i->second.m_dexterity;
			slot << i->second.m_experience;
			slot << i->second.m_front_radius;
			slot << i->second.m_level;
			slot << i->second.m_levelup_points;

			slot << i->second.m_thingTags.size();

			for(int ii = 0; ii != i->second.m_thingTags.size(); ++ii)
			{
				slot << (int)i->second.m_thingTags[ii].m_index;
				slot << i->second.m_thingTags[ii].m_data.size();
				slot.Save(&i->second.m_thingTags[ii].m_data[0],i->second.m_thingTags[ii].m_data.size());
			}

			slot << i->second.m_rank;
		}
	}
	else
	{
		reset();
		size_t infocount = 0;
		slot >> infocount;

		for(size_t i = 0; i < infocount; ++i)
		{
			std::string heroname;
			size_t implcount = 0;
			ExperienceInfo info;

			slot >> heroname;
			slot >> implcount;

			for(size_t ii = 0; ii != implcount; ++ii)
			{
				std::string implname;
				slot >> implname;
				info.m_implants.push_back(implname);
			}

			slot >> info.m_health;
			slot >> info.m_wisdom;
			slot >> info.m_morale;
			slot >> info.m_accuracy;
			slot >> info.m_strength;
			slot >> info.m_reaction;
			slot >> info.m_mechanics;
			slot >> info.m_dexterity;
			slot >> info.m_experience;
			slot >> info.m_front_radius;
			slot >> info.m_level;
			slot >> info.m_levelup_points;

			if(slot.GetStore()->GetVersion() >= 19)
			{
				size_t tcount;
				slot >> tcount;

				for(int ii = 0; ii != tcount; ++ii)
				{
					ExperienceInfo::ThingTag ttag;
					int hpt;
					size_t dsize;
					slot >> hpt;
					ttag.m_index = (human_pack_type)hpt;
					slot >> dsize;
					ttag.m_data.resize(dsize);
					slot.Load(&ttag.m_data[0],dsize);
					info.m_thingTags.push_back(ttag);
				}
			}

			if(slot.GetStore()->GetVersion() >= 20)
			{
				slot >> info.m_rank;
			}

			m_infos.insert(std::make_pair(heroname,info));
		}
	}
}

//=====================================================================================//
//                  bool ExperienceSaver::hasHeroInformation() const                   //
//=====================================================================================//
bool ExperienceSaver::hasHeroInformation(const std::string &name) const
{
	return m_infos.find(name) != m_infos.end();
}

//=====================================================================================//
//          const ExperienceInfo &ExperienceSaver::getExperienceInfo() const           //
//=====================================================================================//
const ExperienceInfo &ExperienceSaver::getExperienceInfo(const std::string &name) const
{
	Infos_t::const_iterator i = m_infos.find(name);

	if(i == m_infos.end())
	{
		throw CASUS("Hero experience record not found");
	}
	else
	{
		return i->second;
	}
}

//=====================================================================================//
//                     void ExperienceSaver::storeExperienceInfo()                     //
//=====================================================================================//
void ExperienceSaver::storeExperienceInfo(HumanEntity *entity)
{
	ExperienceInfo info;
	
	HumanContext *ctx = entity->GetEntityContext();
	for(HumanContext::iterator i = ctx->begin(HPK_IMPLANTS); i != ctx->end(); ++i)
	{
		info.m_implants.push_back(i->GetInfo()->GetRID());
	}

	human_pack_type hpktypes[] = {HPK_HEAD, HPK_BODY, HPK_HANDS, HPK_LKNEE, HPK_RKNEE, HPK_BACKPACK};

	for(int ii = 0; ii != sizeof(hpktypes)/sizeof(hpktypes[0]); ++ii)
	{
		for(HumanContext::iterator i = ctx->begin(hpktypes[ii]); i != ctx->end(); ++i)
		{
			SavSlotStr sss;
			i->SetEntity(0);
			DynUtils::SaveObj(sss,&*i);
			i->MakeSaveLoad(sss);
			i->SetEntity(entity);
			ExperienceInfo::ThingTag tag = {hpktypes[ii], sss.GetData()};
			info.m_thingTags.push_back(tag);
		}
	}

	info.m_health = ctx->GetLimits()->GetHealth();
	info.m_wisdom = ctx->GetLimits()->GetWisdom();
	info.m_morale = ctx->GetLimits()->GetMorale();
	info.m_accuracy = ctx->GetLimits()->GetAccuracy();
	info.m_strength = ctx->GetLimits()->GetStrength();
	info.m_reaction = ctx->GetLimits()->GetReaction();
	info.m_mechanics = ctx->GetLimits()->GetMechanics();
	info.m_dexterity = ctx->GetLimits()->GetDexterity();
	info.m_experience = ctx->GetTraits()->GetExperience();
	info.m_front_radius = ctx->GetLimits()->GetFrontRadius();
	info.m_levelup_points = ctx->GetTraits()->GetLevelupPoints();
	info.m_level = ctx->GetTraits()->GetLevel();
	info.m_rank = ctx->GetRank();

	m_infos[entity->GetInfo()->GetRID()] = info;
}

}
