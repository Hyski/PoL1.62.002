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

	/// ���������� ��� ����������
	virtual CharacterPool *GetCharacterPool() = 0;
	/// ���������� ��� ���������
	virtual ItemsPool *GetItemsPool() = 0;
	/// ���������� �������� ��������
	virtual EffectManager *GetEffects() = 0;

	/// ������� ���
	virtual void Tick() = 0;
	/// ���������
	virtual void Draw() = 0;

	/// ���������� ������, ����������� �����
	virtual OptionsUpdater *GetOptionsUpdater() = 0;
	/// ���������� GraphPipe
	virtual GraphPipe *GetPipe() const = 0;
	/// ���������� GameLevel
	virtual GameLevel *GetLevel() const = 0;
	/// ���������� ������
	virtual Camera *GetCamera() const = 0;

	/// ���������� �������� �������� ������
	virtual std::string GetLevelDesc() const = 0;
	/// ���������� �������� ������ lev
	virtual std::string GetLevelDesc(const std::string &lev) const = 0;

	/// ������� ������-������� � ���������� ��� ���
	virtual std::string CreateLevelAlias(const std::string &) = 0;
	/// ������� �������
	virtual void ChangeLevel(const std::string &LevName) = 0;

	/// ���������� ��������/����������
	virtual bool MakeSaveLoad(GenericStorage &st) = 0;

	/// ���������� ���������� �� �������
	virtual const StoredObjectInfo *GetObjectInfo(const std::string &id) = 0;
	/// ��������� ���������� �� �������
	virtual void SetObjectInfo(const std::string &id, const StoredObjectInfo &info) = 0;
	/// ������� ���������� �� �������� �� ������������ ������
	virtual void ResetObjectInfo(const std::string &level) = 0;

	/// ���������� ��������� �������� ����
	static IWorld *Get();
};
