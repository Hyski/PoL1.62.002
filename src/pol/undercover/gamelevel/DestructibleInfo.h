#pragma once

#include "ExplosionManager.h"

//=====================================================================================//
//                               struct DestructibleInfo                               //
//=====================================================================================//
struct DestructibleInfo
{
	enum PrimaryEffect
	{
		peNone,
		peGlass,
		peMetal,
		peWood,
		peStone,
		peCarton
	};

	enum SecondaryEffect
	{
		seNone,
		seBlackSmoke,
		seWhiteSmoke,
		seSparkles,
		seFire
	};

	enum TertiaryEffect
	{
		teNone,
		teBlast
	};

	PrimaryEffect m_primaryEffect;
	SecondaryEffect m_secondaryEffect;
	TertiaryEffect m_tertiaryEffect;
	unsigned int m_hitPoints;

	hit_s m_damage;

	DestructibleInfo()
	:	m_primaryEffect(peNone),
		m_secondaryEffect(seNone),
		m_tertiaryEffect(teNone),
		m_hitPoints(0)
	{
		m_damage.m_radius = -1.0f;
		m_damage.m_dmg[0].Type = hit_s::DT_NONE;
		m_damage.m_dmg[1].Type = hit_s::DT_NONE;
		m_damage.m_dmg[2].Type = hit_s::DT_NONE;
	}
};
