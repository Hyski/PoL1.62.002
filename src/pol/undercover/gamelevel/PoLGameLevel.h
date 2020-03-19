#pragma once

#include "GameLevel.h"
#include "DynObjectPool.h"
#include "PoLLevelUtils.h"

#include "oct_tree_idx.h"
#include "oct_tree_drawer.h"

namespace ALD { class Manager; }

namespace PoL { class IntersectionCache; }

//=====================================================================================//
//                        class PoLGameLevel : public GameLevel                        //
//=====================================================================================//
class PoLGameLevel : public GameLevel
{
	typedef std::hash_map<std::string,KeyAnimation> CameraPaths_t;
	typedef std::vector<NamedEffect> Effects_t;
	typedef std::vector<SoundUtter> Sounds_t;
	typedef std::vector<mll::algebra::vector3> Normals_t;
	typedef std::vector<char> TriangleDesc_t;

	std::string m_levelName;
	std::string m_lmName;
	oct_tree m_octTree;
	oct_tree_drawer m_octTreeDrawer;

	DynObjectPool m_dynpool;
	std::auto_ptr<Grid> m_levelGrid;
	LongShot m_env;

	MarksPool m_marks;
	bool m_marksEnabled[MT_COUNT];
	static bool m_roofsVisible;

	Sounds_t m_levelSounds;
	Effects_t m_levelEffects;
	CameraPaths_t m_camPaths;

	Normals_t m_normals;
	TriangleDesc_t m_triDesc;

	std::auto_ptr<PoL::IntersectionCache> m_isectCache;

	PoLLevelUtils::TraceContext m_traceContext;

public:
	PoLGameLevel(GenericStorage &);
	PoLGameLevel(const std::string &);
	virtual ~PoLGameLevel();

	virtual void EnableMarks(MARK_TYPE type, bool enable);
	virtual bool MarksEnabled(MARK_TYPE type);

	virtual void UpdateObjects(float Time);
	virtual void Start();

	virtual DynObjectPool *GetLevelObjects() { return &m_dynpool; }
	virtual void Update(GraphPipe *);

	void EndTurn() {}

public:
	// добавить отметину
	virtual void AddMark(boost::shared_ptr<BaseMark> L);
	//оставить в определенной точке уровня след
	virtual void DoMark(const point3 &Where, const float Rad, const std::string& Shader);
	// проапдейтить все отметины (кроме постоянных)
	virtual void UpdateMarks(float Time);
	//сопоставить названиям шейдеров указатели на них
	virtual void LinkShaders(GraphPipe *Pipe);
	//отрисовать все отметки
	virtual void DrawMarks();
	//возвращает точку пересечения луча с уровнем
	virtual bool TraceRay3(const ray &, unsigned int passThru, TraceResult *);
	//трассировать отрезок
	virtual bool TraceSegment3(const ray &r, unsigned int passThru, TraceResult *);
	//вызывается после окончания сеанса 
	virtual void Draw(GraphPipe *Pipe,bool Transparent);
	// собрать и рассчитать все треугольники, на кот. влияет отметина
	virtual void CollectPlanes(point3 &Pos, float Rad, BaseMark &m,int CurIndex=0);
	// собрать и рассчитать все треугольники, на кот. влияет тень
	virtual void CollectPlanesForShadow(Shadows::BaseShadow *shd, Primi *prim, float rad);

	/// Отображаются ли крыши
	virtual bool IsRoofVisible() const;
	virtual void SetRoofVisible(bool v);

	virtual void OnStartChangeVideoMode();
	virtual void OnFinishChangeVideoMode();

	/// Возвращает сплайн камеры с заданным названием
	virtual const KeyAnimation *GetCameraPath(const std::string &path) const;
	/// Возвращает название уровня
	virtual const std::string &GetLevelName() const;
	/// Возвращает хексгрид
	virtual Grid *GetGrid();
	/// Возвращает путь автобуса по имени
	virtual BusPath *GetBusPath(const std::string &);
	/// Возвращает точки пути по имени
	virtual DestPoints *GetDestPoints(const std::string &);
	/// Возвращает дальний план
	virtual LongShot *GetLongShot();

	void Save(GenericStorage &st);

private:
	void Stop();

	void LoadLevel();

	void CalcNormals();
	void LoadCameras();
	void LoadSounds(ALD::Manager &);
	void LoadEffects(ALD::Manager &);
	void LoadObjects(ALD::Manager &);
	void LoadObjectsGeometry(ALD::Manager &);
	void CreateLightMap(std::istream &);
};