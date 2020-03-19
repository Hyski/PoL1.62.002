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
	mdTransparent			= 1<<0,		///< ��������� ��� ���������
	mdGhost					= 1<<1,		///< ��������� ��� ������
	mdRoof					= 1<<2,		///< �����
	mdCursor				= 1<<3,		///< ��������� ��� �������
	mdShield				= 1<<4,		///< �������� �����
	mdCharacter				= 1<<5,		///< �������� ����������
	mdDynObject				= 1<<6,		///< �������� �������� �� ������
	mdDynObjectTransparent	= 1<<7		///< �������� ���������� �������� �� ������
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
		point3 m_pos;		///< ����� ��������� ����
		point3 m_norm;		///< ������� � ����� ���������
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
	/// �������� "���������" ���������
	virtual void Update(GraphPipe *) = 0;

public:
//	//��������� � ����� ����
//	virtual void EndTurn() = 0;
	// �������� ��������
	virtual void AddMark(boost::shared_ptr<BaseMark> L) = 0;
	//�������� � ������������ ����� ������ ����
	virtual void DoMark(const point3 &Where, const float Rad, const std::string& Shader) = 0;
	// ������������ ��� �������� (����� ����������)
	virtual void UpdateMarks(float Time) = 0;
	//����������� ��������� �������� ��������� �� ���
	virtual void LinkShaders(GraphPipe *Pipe) = 0;
	//���������� ��� �������
	virtual void DrawMarks() = 0;
	//���������� ����� ����������� ���� � �������
	virtual bool TraceRay3(const ray &, unsigned int passThru, TraceResult *) = 0;
	//������������ �������
	virtual bool TraceSegment3(const ray &r, unsigned int passThru, TraceResult *) = 0;
//	//���������� ����� �������� ������
//	virtual void PrepareData(GraphPipe *Pipe) = 0;
	//���������� ����� ��������� ������ 
	virtual void Draw(GraphPipe *Pipe,bool Transparent) = 0;
	// ������� � ���������� ��� ������������, �� ���. ������ ��������
	virtual void CollectPlanes(point3 &Pos, float Rad, BaseMark &m,int CurIndex=0) = 0;
	// ������� � ���������� ��� ������������, �� ���. ������ ����
	virtual void CollectPlanesForShadow(Shadows::BaseShadow *shd, Primi *prim, float rad) = 0;

	/// ������������ �� �����
	virtual bool IsRoofVisible() const = 0;
	virtual void SetRoofVisible(bool v) = 0;

	virtual void OnStartChangeVideoMode() = 0;
	virtual void OnFinishChangeVideoMode() = 0;

	/// ���������� ������ ������ � �������� ���������
	virtual const KeyAnimation *GetCameraPath(const std::string &path) const = 0;
	/// ���������� �������� ������
	virtual const std::string &GetLevelName() const = 0;
	/// ���������� �������� ������ (��� ���������� ����������)
	std::string GetRawLevelName() const;
	/// ���������� ��������
	virtual Grid *GetGrid() = 0;
	/// ���������� ���� �������� �� �����
	virtual BusPath *GetBusPath(const std::string &) = 0;
	/// ���������� ����� ���� �� �����
	virtual DestPoints *GetDestPoints(const std::string &) = 0;
	/// ���������� ������� ����
	virtual LongShot *GetLongShot() = 0;
};

//=====================================================================================//
//                inline std::string GameLevel::GetRawLevelName() const                //
//=====================================================================================//
/// ���������� �������� ������ (��� ���������� ����������)
inline std::string GameLevel::GetRawLevelName() const
{
	return GetLevelName().substr(0,GetLevelName().find(':'));
}

#endif // !defined(AFX_GAMELEVEL_H__792421A0_3176_11D4_A0E0_525405F0AA60__INCLUDED_)
