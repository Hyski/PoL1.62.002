#pragma once

#include "DynObject.h"
#include "DynObjectSet.h"

struct hit_s;
struct Station;
class ShapePool;
class LevelCase;
class GraphPipe;
class LogicDamageStrategy;

namespace ALD { class Manager; }

//=====================================================================================//
//                                 class DynObjectPool                                 //
//=====================================================================================//
class DynObjectPool
{
	class Impl;
	std::auto_ptr<Impl> m_pimpl;

	typedef std::vector<HDynObjectSet> ObjectSets_t;

public:
	//typedef std::multimap<std::string,DynObject> ObjContainer;
	typedef ObjectSets_t::iterator SetIterator_t;
	typedef ObjectSets_t::const_iterator ConstSetIterator_t;

	//class iterator;
	//friend class iterator;

	enum TRACE_TYPE{TT_SIGHT, TT_PHYSIC};
	//enum SWITCH_STATE{SS_OFF,SS_ON,SS_SWITCH};

public:
	float SwitchState(const std::string& Name,float Time/*,SWITCH_STATE State=SS_SWITCH*/);
	void DoExplosion(std::string ObjName,const hit_s& hit, LogicDamageStrategy *LogicStrategy);//При взрыве уничтожаются объекты...

	bool TraceRay(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm, std::string *Name, unsigned int passThru = mdTransparent/*TRACE_TYPE trace_type=TT_SIGHT*/);
	bool TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm, std::string *Name);
	bool TraceSegment2(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm, std::string *Name, unsigned int passThru = mdTransparent/*TRACE_TYPE trace_type=TT_SIGHT*/);
	void EraseByName(const std::string& Name);
	const LevelCase* GetCase(const std::string& Name);

	SetIterator_t BeginSets();
	SetIterator_t EndSets();
	ConstSetIterator_t BeginSets() const;
	ConstSetIterator_t EndSets() const;

	void Bake();

	void Draw(GraphPipe *Pipe, bool trans);
	void CollectHexesForObject(const std::string &ObjName,std::vector<ipnt2_t> *vec);
	void ClearObjectInfluence();

	void Stop();
	void Clear();
	void Start();
	void Update(float Time);
	void EndTurn();
	void MakeSaveLoad(SavSlot &s, const std::string &);

	DynObjectSet *GetObjectSet(const std::string &);
	const DynObjectSet *GetObjectSet(const std::string &) const;

	void LoadAlfa(ALD::Manager &, const std::string &);

	//очищать множество должен клиент!
	void CollectObjectsForPoints(const std::vector<point3> &pnts, std::set<std::string> *objects);

	void OnLostDevice();
	void OnResetDevice();

private:
	//friend class GameLevel;
	void Load(FILE *in);
	//void UpdateHG(DynObject *Obj);
	//void ChangeHG(const std::string &ObjName,float State, float z);
	void SetStationInfo(const std::string &Name,const Station &st);

public:
	ShapePool* GetShapePool();
	DynObjectPool();
	~DynObjectPool();
};

//=====================================================================================//
//                            class DynObjectPool::iterator                            //
//=====================================================================================//
//class DynObjectPool::iterator
//{
//public:
//	typedef DynObjectPool::ObjContainer::iterator ObjIt;
//
//	iterator(const ObjIt &Cur) : m_Cur(Cur) {}
//
//	bool operator ==(const iterator &a) const { return m_Cur==a.m_Cur; }
//	bool operator !=(const iterator &a) const { return m_Cur!=a.m_Cur; }
//
//	iterator &operator++() { ++m_Cur; return *this; }
//	iterator operator++(int) { iterator i(*this); ++(*this); return i; }
//
//	DynObject* operator->() { return &m_Cur->second; }
//	DynObject& operator*() { return m_Cur->second; }
//
//private:
//	ObjIt m_Cur;
//};

