#pragma once

class GameLevel;
class CharacterPool;
class ItemsPool;
class EffectManager;
class Camera;
class SavSlot;
class GenericStorage;
class Entity;
class GraphPipe;
class OptionsUpdater;

//=====================================================================================//
//                               struct StoredObjectInfo                               //
//=====================================================================================//
struct StoredObjectInfo
{
	bool m_isVisible;
	bool m_isDestroyed;
};

//=====================================================================================//
//                                    class IWorld                                     //
//=====================================================================================//
class IWorld
{
public:
	IWorld();
	virtual ~IWorld();

	/// Возвращает пул персонажей
	virtual CharacterPool *GetCharacterPool() = 0;
	/// Возвращает пул предметов
	virtual ItemsPool *GetItemsPool() = 0;
	/// Возвращает менеджер эффектов
	virtual EffectManager *GetEffects() = 0;

	/// Игровой тик
	virtual void Tick() = 0;
	/// Отрисовка
	virtual void Draw() = 0;

	/// Возвращает объект, обновляющий опции
	virtual OptionsUpdater *GetOptionsUpdater() = 0;
	/// Возвращает GraphPipe
	virtual GraphPipe *GetPipe() const = 0;
	/// Возвращает GameLevel
	virtual GameLevel *GetLevel() const = 0;
	/// Возвращает камеру
	virtual Camera *GetCamera() const = 0;

	/// Возвращает описание текущего уровня
	virtual std::string GetLevelDesc() const = 0;
	/// Возвращает описание уровня lev
	virtual std::string GetLevelDesc(const std::string &lev) const = 0;

	/// Создает псевдо-уровень и возвращает его имя
	virtual std::string CreateLevelAlias(const std::string &) = 0;
	/// Сменить уровень
	virtual void ChangeLevel(const std::string &LevName) = 0;

	/// Произвести загрузку/сохранение
	virtual bool MakeSaveLoad(GenericStorage &st) = 0;

	/// Возвращает информацию об объекте
	virtual const StoredObjectInfo *GetObjectInfo(const std::string &id) = 0;
	/// Сохранить информацию об объекте
	virtual void SetObjectInfo(const std::string &id, const StoredObjectInfo &info) = 0;
	/// Стереть информацию об объектах на определенном уровне
	virtual void ResetObjectInfo(const std::string &level) = 0;

	/// Возвращает экземпляр игрового мира
	static IWorld *Get();
};
