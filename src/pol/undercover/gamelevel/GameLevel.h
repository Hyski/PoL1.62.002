// GameLevel.h: interface for the GameLevel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMELEVEL_H__792421A0_3176_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_GAMELEVEL_H__792421A0_3176_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mark.h"
#include <common/textureMgr/dibdata.h>
#include "levelobjects.h"
#include "longshot.h"
#include "../world.h"
#include "grid.h"
//#include "DynObjectPool.h"
#include "../CommonTrace.h"
#include <Muffle/ISound.h>

#include <boost/shared_ptr.hpp>

struct BSPnode;
class GenericStorage;
class GameLevel;


namespace Shadows { class BaseShadow; }

//=====================================================================================//
//                                enum PoLTriangleDesc                                 //
//=====================================================================================//
enum PoLTriangleDesc
{
	mdTransparent			= 1<<0,		///< Прозрачно для видимости
	mdGhost					= 1<<1,		///< Прозрачно для физики
	mdRoof					= 1<<2,		///< Крыша
	mdCursor				= 1<<3,		///< Прозрачно для курсора
	mdShield				= 1<<4,		///< Является щитом
	mdCharacter				= 1<<5,		///< Является персонажем
	mdDynObject				= 1<<6,		///< Является объектом на уровне
	mdDynObjectTransparent	= 1<<7		///< Является прозрачным объектом на уровне
};

//=====================================================================================//
//                        class SoundUtter : public NamedSound                         //
//=====================================================================================//
class SoundUtter : public NamedSound
{
public:
	boost::shared_ptr<Muffle::HEmitter> m_Emitter2;

	SoundUtter() : m_Emitter2(new Muffle::HEmitter) {}

	point3& GetPos(){return Pos;};
	point3& GetVel(){return Vel;};
};

///////////////////////////////////////////////////
struct BusPath
{
	typedef std::vector<ipnt2_t> PathSeg;
	std::string Name;
	std::vector<PathSeg> Segments;
};
struct DestPoints
{
	struct DestPoint
	{
		point3 Pos;
		point3 Dir;
		int Special;
	};
	typedef std::vector<DestPoint> PntVec;
	std::string Name;
	PntVec Points;
};
///////////////////////////////////////////////////
class IExplosionAPI;
class ShapePool;
struct hit_s;
class LogicDamageStrategy;

class Grid;
class Bsp;
class DynamicVB;
class VShader;
class DynObjectPool;

//=====================================================================================//
//                                   class GameLevel                                   //
//=====================================================================================//
class GameLevel
{
public:
	enum MARK_TYPE { MT_DYNAMIC, MT_STATIC, MT_COUNT };

	struct TraceResult : public PoL::CommonTrace
	{
		point3 m_pos;		///< Точка попадания луча
		point3 m_norm;		///< Нормаль в точке попадания
	};

public:
	virtual ~GameLevel() {}

	virtual void EnableMarks(MARK_TYPE type, bool enable) = 0;
	virtual bool MarksEnabled(MARK_TYPE type) = 0;

	virtual void UpdateObjects(float Time) = 0;
//	//virtual void LoadGrid(std::string FName) = 0;
	virtual void Start() = 0;
//	virtual void Stop() = 0;

	virtual	void Save(GenericStorage &st) = 0;

	virtual DynObjectPool *GetLevelObjects() = 0;
	/// Обновить "видимость" геометрии
	virtual void Update(GraphPipe *) = 0;

public:
//	//сообщение о конце тура
//	virtual void EndTurn() = 0;
	// добавить отметину
	virtual void AddMark(boost::shared_ptr<BaseMark> L) = 0;
	//оставить в определенной точке уровня след
	virtual void DoMark(const point3 &Where, const float Rad, const std::string& Shader) = 0;
	// проапдейтить все отметины (кроме постоянных)
	virtual void UpdateMarks(float Time) = 0;
	//сопоставить названиям шейдеров указатели на них
	virtual void LinkShaders(GraphPipe *Pipe) = 0;
	//отрисовать все отметки
	virtual void DrawMarks() = 0;
	//возвращает точку пересечения луча с уровнем
	virtual bool TraceRay3(const ray &, unsigned int passThru, TraceResult *) = 0;
	//трассировать отрезок
	virtual bool TraceSegment3(const ray &r, unsigned int passThru, TraceResult *) = 0;
//	//вызывается перед запросом данных
//	virtual void PrepareData(GraphPipe *Pipe) = 0;
	//вызывается после окончания сеанса 
	virtual void Draw(GraphPipe *Pipe,bool Transparent) = 0;
	// собрать и рассчитать все треугольники, на кот. влияет отметина
	virtual void CollectPlanes(point3 &Pos, float Rad, BaseMark &m,int CurIndex=0) = 0;
	// собрать и рассчитать все треугольники, на кот. влияет тень
	virtual void CollectPlanesForShadow(Shadows::BaseShadow *shd, Primi *prim, float rad) = 0;

	/// Отображаются ли крыши
	virtual bool IsRoofVisible() const = 0;
	virtual void SetRoofVisible(bool v) = 0;

	virtual void OnStartChangeVideoMode() = 0;
	virtual void OnFinishChangeVideoMode() = 0;

	/// Возвращает сплайн камеры с заданным названием
	virtual const KeyAnimation *GetCameraPath(const std::string &path) const = 0;
	/// Возвращает название уровня
	virtual const std::string &GetLevelName() const = 0;
	/// Возвращает название уровня (без генеренной информации)
	std::string GetRawLevelName() const;
	/// Возвращает хексгрид
	virtual Grid *GetGrid() = 0;
	/// Возвращает путь автобуса по имени
	virtual BusPath *GetBusPath(const std::string &) = 0;
	/// Возвращает точки пути по имени
	virtual DestPoints *GetDestPoints(const std::string &) = 0;
	/// Возвращает дальний план
	virtual LongShot *GetLongShot() = 0;
};

//=====================================================================================//
//                inline std::string GameLevel::GetRawLevelName() const                //
//=====================================================================================//
/// Возвращает название уровня (без генеренной информации)
inline std::string GameLevel::GetRawLevelName() const
{
	return GetLevelName().substr(0,GetLevelName().find(':'));
}

#endif // !defined(AFX_GAMELEVEL_H__792421A0_3176_11D4_A0E0_525405F0AA60__INCLUDED_)
