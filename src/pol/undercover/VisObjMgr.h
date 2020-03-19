#pragma once

#include "CommonTrace.h"

class GraphPipe;

namespace PoL
{

class VisObj;

//=====================================================================================//
//                                   class VisObjMgr                                   //
//=====================================================================================//
class VisObjMgr
{
	typedef mll::geometry::ray3 ray3;
	typedef mll::geometry::aabb3 aabb3;
	typedef mll::algebra::point3 point3;
	typedef mll::algebra::vector3 vector3;

	struct VisObjData
	{
		VisObj *m_object;		///< ��������� �� ��� ������
		aabb3 m_bbox;			///< �����
		bool m_bboxDirty;		///< ���������, ��� ����� ���������
		unsigned int m_ctr;		///< ��������������� ������� ��� ������� �������

		VisObjData() : m_object(0), m_bboxDirty(true), m_ctr(0) {}

		void setObject(VisObj *obj);
		void setDirtyBbox(bool b) { m_bboxDirty = b; }
	};

	typedef std::vector<VisObjData> Objects_t;
	typedef std::vector<long> FreeList_t;

	Objects_t m_objects;
	FreeList_t m_freeList;
	int m_dirtyBoxCount;
	unsigned int m_dataCtr;

private:
	VisObjMgr();
	~VisObjMgr();

public:
	/// �������� ������
	void addObject(VisObj *);
	/// ������ ������
	void removeObject(VisObj *);
	/// ��������� �� ��������� ������
	void setBboxChanged(VisObj *);

	struct TraceResult : public CommonTrace
	{
		int m_kind;				///< ��. VisObjKind
		point3 m_hitPoint;
		vector3 m_normal;
		std::string m_id;
	};

	void render(GraphPipe *, bool trnsp);
	bool traceRay(const ray3 &, unsigned int passThru, TraceResult *);
	bool traceSegment(const ray3 &, unsigned int passThru, TraceResult *);
	//bool traceRayBbox(const mll::algebra::ray3 &, unsigned int passThru, TraceResult *);

	static VisObjMgr *inst();

private:
	void enlarge();
	void updateDirtyBoxes();
	void updateObjPlace(VisObjData *);
};

}
