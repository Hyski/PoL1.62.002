#pragma once

#include "SndAuto.h"
#include "GameLevel.h"
#include "objectsNG.h"
#include "DynObjectInfo.h"
#include "../Skin/KeyAnimation.h"
#include <Common/GraphPipe/SimpleTexturedObject.h>

//class SndAuto;
class DynObjectSet;

//=====================================================================================//
//                                   class DynObject                                   //
//=====================================================================================//
class DynObject : public MlRefCounted, DynObjectInfo, boost::noncopyable
{
	TexObject m_texObject;
	KeyAnimation m_anim;

	bool m_inMotion;		///< Анимируется ли объект на данный момент
	float m_animState;		///< Нормированное состояние анимации
	float m_targetState;	///< Состояние анимации, к которому идет проигрывание
	float m_startTime;		///< Время начала анимации
	bool m_isRoof;			///< Относится ли объект к крышам

	DynObjectSet *m_set;
	SndAuto m_sound;

	bool m_isTransparentShader;

public:
	DynObject(const DynObjectOldData &);
	~DynObject();

private: //свойства формы
	BBox Bound;     //BBox в мировых координатах
	BBox LocalBound;//BBox в локальных координатах объекта  (константа)

	D3DMATRIX m_world;    //мировая матрица
	D3DMATRIX m_invWorld; //обратная к мировой матрица
	ShapePool::Handle m_MyShape;

public:    
	bool TraceRay(const ray &r, float *Pos, point3 *Norm);
	bool TraceSegment(const ray &r, float *Pos, point3 *Norm);
	void UpdateBound();
	//void EndTurn();

	/// Запустить звуки, связанные с объектом
	void StartSound();

	/// Прозрачен ли объект
	bool IsTransparentShader() const { return m_isTransparentShader; }

	/// Получить меш
	TexObject *GetTexObject() { return &m_texObject; }
	const TexObject *GetTexObject() const { return &m_texObject; }

	/// Относится ли к крышам
	bool IsRoof() const { return m_isRoof; }

	/// Анимируется ли на данный момент объект
	bool IsInMotion() const { return m_inMotion; }
	/// Возвращает длительность полного цикла анимации объекта в кадрах
	float GetAnimLength() const { return m_anim.GetLastTime(); }

	/// Получить мировую матрицу
	const D3DMATRIX &GetWorldMatrix() const { return m_world; }
	/// Получить мировую матрицу
	const D3DMATRIX &GetInvWorldMatrix() const { return m_invWorld; }
	/// Возвращает aabb объекта в мировых координатах
	const BBox &GetBound() const { return Bound; }
	/// Возвращает aabb объекта в локальных координатах
	const BBox &GetLocalBound() const { return LocalBound; }

	void SetShapeHandle(ShapePool::Handle h);
	ShapePool::Handle GetShapeHandle() const { return m_MyShape; }
	void SetAnimation(const KeyAnimation &anim) { m_anim = anim; }
	void SetWorldMatrix(const D3DMATRIX &world) { m_world = world; }
	void SetInvWorldMatrix(const D3DMATRIX &world) { m_invWorld = world; }
	void SetTexObject(const TexObject &texo);

	/// Получить группу, в которой находится объект
	DynObjectSet *GetSet() { return m_set; }
	const DynObjectSet *GetSet() const { return m_set; }

	/// Установить группу, в которой находится объект
	void SetSet(DynObjectSet *set) { m_set = set; }

	/// Сохранить объект
	bool MakeSave(SavSlot &);
	/// Восстановить объект
	bool MakeLoad(SavSlot &);

	/// Обладает ли объект геометрией
	bool HasGeometry() const { return m_texObject.PartNum && m_texObject.Parts[0]->prim.Verts.size(); }

	bool operator<(const DynObject &r) const { return GetName() < r.GetName(); }

	struct StringCompare
	{
		bool operator()(const DynObject &l, const std::string &r) const { return l.GetName() < r; }
		bool operator()(const std::string &l, const DynObject &r) const { return l < r.GetName(); }
	};

	void UpdateAfterLoad();

public:
	unsigned int GetTraceParams() const
	{
		unsigned int result = 0;
		if(IsGhost())
		{
			result |= mdGhost;
			result |= mdCursor;
		}
		if(IsTransparent()) result |= mdTransparent|mdDynObjectTransparent;
		return result;
	}

	using DynObjectInfo::GetName;
	using DynObjectInfo::IsGhost;
	using DynObjectInfo::IsTransparent;
	using DynObjectInfo::IsDestructible;

	using DynObjectInfo::IsAnimated;
	using DynObjectInfo::IsSoundable;

	using DynObjectInfo::GetSoundInfo;
	using DynObjectInfo::GetDestructibleInfo;

private: // Интерфейс для DynObjectSet
	float ChangeState(float tau);
	float ChangeState(float target, float tau);
	void Update(float tau);

private:
	friend class DynObjectSet;
	friend class DoorProperty;
	friend class TrainProperty;
	friend class ElevatorProperty;

	void Load5(SavSlot &);
	void Load4(SavSlot &);

	void UpdateOnTime(float tau);
};

typedef MlHandle<DynObject> HDynObject;
