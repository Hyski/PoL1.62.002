// LevelObjects.h: interface for the DynObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELOBJECTS_H__C1CF8E22_5E71_4465_A088_EF6C1A261F11__INCLUDED_)
#define AFX_LEVELOBJECTS_H__C1CF8E22_5E71_4465_A088_EF6C1A261F11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <common/graphpipe/simpletexturedobject.h>
#include <common/utils/optslot.h>
#include "../skin/keyanimation.h"
#include "objectsNG.h"
#include "explosionmanager.h"
#include <mll/utils/named_vars.h>

class DynObject;
class SavSlot;

class NamedEffect
{
public:
	unsigned int id;    // id эффекта, которое возвращает EffectManager
	std::string Name;//имя эффекта
	std::string EffectType; // название эффекта (smoke)
	point3 Color;

	point3 Position;//его расположение
	point3 Front;    //.. и ориентация
	point3 Up;       //..
	point3 Right;    //..
	void Load(FILE *f);
	void Save(FILE *f);
	bool MakeSaveLoad(SavSlot &slot);

	void LoadAlfa(const mll::utils::named_vars &vars, const mll::algebra::matrix3 &mtr);
};

class NamedSound
{
public:
	enum {NS_STATIC=0x01, NS_RANDOM=0x02, NS_CYCLED=0x04};
	std::vector<std::string> Names;
	std::string Name;
	int LastSoundNum;
	unsigned Type; //тип звука
	float Freq;
	float LastTimePlayed;
public:
	point3 Pos;
	point3 Vel;
	void Load(FILE *f);
	void Save(FILE *f);
	bool MakeSaveLoad(SavSlot &slot);
	void LoadAlfa(const mll::utils::named_vars &vars, const mll::algebra::matrix3 &mtr);
	void ParseSounds();
	const std::string &GetNextName();
	NamedSound():LastSoundNum(0),LastTimePlayed(0){;};
};

class PositionesSnd;


#endif // !defined(AFX_LEVELOBJECTS_H__C1CF8E22_5E71_4465_A088_EF6C1A261F11__INCLUDED_)
