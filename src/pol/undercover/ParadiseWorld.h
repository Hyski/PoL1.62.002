#pragma once

#include "IWorld.h"
#include "Character.h"
#include "Common/3DEffects/EffectManager.h"

//=====================================================================================//
//                         class ParadiseWorld: public IWorld                          //
//=====================================================================================//
class ParadiseWorld : public IWorld
{
	unsigned int m_aliasCounter;

	EffectManager m_effectsMgr;

	ItemsPool m_items;
	CharacterPool m_charpool;
	std::auto_ptr<GameLevel> m_level;

	GraphPipe *m_pipe;

	typedef std::hash_map<std::string,std::string> LevelDescMap_t;
	LevelDescMap_t m_levelsDesc;

	typedef std::hash_map<std::string,StoredObjectInfo> ObjectInfos_t;
	ObjectInfos_t m_objectInfos;

public:
	ParadiseWorld(GraphPipe *pipe);
	virtual ~ParadiseWorld();

	/// Возвращает пул персонажей
	virtual CharacterPool *GetCharacterPool();
	/// Возвращает пул предметов
	virtual ItemsPool *GetItemsPool();
	/// Возвращает менеджер эффектов
	virtual EffectManager *GetEffects();

	/// Игровой тик
	virtual void Tick();
	/// Отрисовка
	virtual void Draw();

	/// Возвращает объект, обновляющий опции
	virtual OptionsUpdater *GetOptionsUpdater();
	/// Возвращает GraphPipe
	virtual GraphPipe *GetPipe() const { return m_pipe; }
	/// Возвращает GameLevel
	virtual GameLevel *GetLevel() const;
	/// Возвращает камеру
	virtual Camera *GetCamera() const;

	/// Возвращает описание текущего уровня
	virtual std::string GetLevelDesc() const;
	/// Возвращает описание уровня lev
	virtual std::string GetLevelDesc(const std::string &lev) const;

	/// Создает псевдо-уровень и возвращает его имя
	virtual std::string CreateLevelAlias(const std::string &);
	/// Сменить уровень
	virtual void ChangeLevel(const std::string &lev);

	/// Произвести загрузку/сохранение
	virtual bool MakeSaveLoad(GenericStorage &st);

	/// Возвращает информацию об объекте
	virtual const StoredObjectInfo *GetObjectInfo(const std::string &id);
	/// Сохранить информацию об объекте
	virtual void SetObjectInfo(const std::string &id, const StoredObjectInfo &info);
	/// Стереть информацию об объектах на определенном уровне
	virtual void ResetObjectInfo(const std::string &level);

private:
	/// Загрузить описания уровней из таблицы
	void LoadLevelNames();

	/// Загрузить описания уровней из сейва
	void LoadLevelDescs(SavSlot &slot);
	/// Сохранить описания уровней в сейв
	void SaveLevelDescs(SavSlot &slot);

	/// Загрузить описания объектов из сейва
	void LoadObjectDescs(SavSlot &slot);
	/// Сохранить описания объектов в сейв
	void SaveObjectDescs(SavSlot &slot);
};
