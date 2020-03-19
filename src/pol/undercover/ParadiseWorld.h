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

	/// ���������� ��� ����������
	virtual CharacterPool *GetCharacterPool();
	/// ���������� ��� ���������
	virtual ItemsPool *GetItemsPool();
	/// ���������� �������� ��������
	virtual EffectManager *GetEffects();

	/// ������� ���
	virtual void Tick();
	/// ���������
	virtual void Draw();

	/// ���������� ������, ����������� �����
	virtual OptionsUpdater *GetOptionsUpdater();
	/// ���������� GraphPipe
	virtual GraphPipe *GetPipe() const { return m_pipe; }
	/// ���������� GameLevel
	virtual GameLevel *GetLevel() const;
	/// ���������� ������
	virtual Camera *GetCamera() const;

	/// ���������� �������� �������� ������
	virtual std::string GetLevelDesc() const;
	/// ���������� �������� ������ lev
	virtual std::string GetLevelDesc(const std::string &lev) const;

	/// ������� ������-������� � ���������� ��� ���
	virtual std::string CreateLevelAlias(const std::string &);
	/// ������� �������
	virtual void ChangeLevel(const std::string &lev);

	/// ���������� ��������/����������
	virtual bool MakeSaveLoad(GenericStorage &st);

	/// ���������� ���������� �� �������
	virtual const StoredObjectInfo *GetObjectInfo(const std::string &id);
	/// ��������� ���������� �� �������
	virtual void SetObjectInfo(const std::string &id, const StoredObjectInfo &info);
	/// ������� ���������� �� �������� �� ������������ ������
	virtual void ResetObjectInfo(const std::string &level);

private:
	/// ��������� �������� ������� �� �������
	void LoadLevelNames();

	/// ��������� �������� ������� �� �����
	void LoadLevelDescs(SavSlot &slot);
	/// ��������� �������� ������� � ����
	void SaveLevelDescs(SavSlot &slot);

	/// ��������� �������� �������� �� �����
	void LoadObjectDescs(SavSlot &slot);
	/// ��������� �������� �������� � ����
	void SaveObjectDescs(SavSlot &slot);
};
