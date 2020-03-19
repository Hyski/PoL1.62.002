#pragma once

#include "GameLevel.h"

class oct_tree;

namespace Shadows { class BaseShadow; }
namespace PoL { class IntersectionCache; }

//=====================================================================================//
//                                 class PoLLevelUtils                                 //
//=====================================================================================//
class PoLLevelUtils
{
public:
	typedef mll::geometry::ray3 ray3;

	typedef GameLevel::TraceResult TraceResult;
	
	struct TraceContext
	{
		oct_tree *m_tree;
		const mll::algebra::vector3 *m_normals;
		const char *m_triDesc;
	};

	static bool TraceRay(const TraceContext &, const ray3 &, unsigned int passThru, TraceResult *tr);
	static bool TraceSegment(const TraceContext &, const ray3 &, unsigned int passThru, TraceResult *tr);
	static bool TraceRay2(const PoL::IntersectionCache *, const ray3 &, unsigned int passThru, TraceResult *tr);
	static bool TraceSegment2(const PoL::IntersectionCache *, const ray3 &, unsigned int passThru, TraceResult *tr);
	//static bool TraceRay(oct_tree *t, const mll::geometry::ray3 &r, TraceResult *tr, bool roofs, const mll::algebra::vector3 *normals);
	//static bool TraceSegment(oct_tree *t, const mll::algebra::vector3 *normals, const mll::algebra::point3 &p1, const mll::algebra::point3 &p2, TraceResult *tr);
	static void CollectTrianglesForShadow(oct_tree *t, Shadows::BaseShadow &, Primi &prim, float rad);
	static void CollectTrianglesInSphere(oct_tree *t, Primi &prim, unsigned int col, const mll::algebra::point3 &, float rad);
};