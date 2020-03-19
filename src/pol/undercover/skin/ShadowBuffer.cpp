#include "precomp.h"

#include "shadow.h"
#include <undercover/gamelevel/GameLevel.h>
#include <common/GraphPipe/statesmanager.h>

namespace Shadows {

  CLog bufLog;

//=====================================================================================//
//                            ShadowBuffer::ShadowBuffer()                             //
//=====================================================================================//
ShadowBuffer::ShadowBuffer()
{
	m_primitive.Verts.reserve(MAXTRIANGLES*3);
	m_primitive.Prim = Primi::TRIANGLE;
	m_primitive.Contents = Primi::NEEDTRANSFORM|Primi::DIFFUSE;
}

//=====================================================================================//
//                            ShadowBuffer::~ShadowBuffer()                            //
//=====================================================================================//
ShadowBuffer::~ShadowBuffer()
{
}

//=====================================================================================//
//                              void ShadowBuffer::Draw()                              //
//=====================================================================================//
void ShadowBuffer::Draw(GraphPipe * pipe, const std::string &shader)
{
	StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)IDENTITYMAT);
#ifdef _HOME_VERSION
	if(m_primitive.GetVertexCount()) pipe->Chop(shader,&m_primitive,"Shadow");
#else
	if(m_primitive.GetVertexCount()) pipe->Chop(shader,&m_primitive);
#endif
}

//=====================================================================================//
//                                inline void polyAdd()                                //
//=====================================================================================//
inline void polyAdd(gPoint *outPolygon, unsigned &outVertices, bool lastVertex, float x, float y)
{
	const float epsilon = 1e-6f;
	if (outVertices == 0 ||
		(
			fabs(x - outPolygon[outVertices-1].pX) > epsilon ||
			fabs(y - outPolygon[outVertices-1].pY) > epsilon
		) && (
			!lastVertex ||
			fabs (x - outPolygon[0].pX) > epsilon ||
			fabs (y - outPolygon[0].pY) > epsilon
		)
	) 
	{
		outPolygon[outVertices].pX = x;
		outPolygon[outVertices].pY = y;
		outVertices++;
	}
}

//=====================================================================================//
//                         void ShadowBuffer::FlushGeometry()                          //
//=====================================================================================//
void ShadowBuffer::FlushGeometry()
{
}

//=====================================================================================//
//                         void ShadowBuffer::CollectPlanes()                          //
//=====================================================================================//
void ShadowBuffer::CollectPlanes(BaseShadow *shadow, float Rad, int CurIndex)
{
	m_primitive.Verts.clear();
	IWorld::Get()->GetLevel()->CollectPlanesForShadow(shadow,&m_primitive,Rad);
}

/*
//=====================================================================================//
//                            void ShadowBuffer::cutPoly()                             //
//=====================================================================================//
void ShadowBuffer::cutPoly(gPoly &triangle, gPoly &result, float minx, float miny, float maxx, float maxy)
{
	gPoint outPoly1[gPoly::MAXVERTS], outPoly2[gPoly::MAXVERTS];
	gPoint *inPolygon = triangle.verts;
	gPoint *outPolygon = outPoly1;
	unsigned inVertices = triangle.vCount;
	unsigned outVertices = 0;

	do
	{
		// edge1 - лева€ вертикальна€ граница
		// dx = 0; dy = 1;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (px > minx);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cx > minx);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = cx - px;

					if (denominator != 0.0)
					{
						float t = (minx - px)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge2 - верхн€€ горизонтальна€ граница
		// dx = 1; dy = 0;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (py < maxy);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cy < maxy);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = py - cy;

					if (denominator != 0.0)
					{
						float t = (py - maxy)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge3 - права€ вертикальна€ граница
		// dx = 0; dy = -1;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (px < maxx);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cx < maxx);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = px - cx;

					if (denominator != 0.0)
					{
						float t = (px - maxx)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}

			// if polygon is wiped out, break

			if (outVertices < 3) break;

			// switch input/output polygons
			inVertices = outVertices;
			inPolygon = outPolygon;
			if (outPolygon == outPoly2) outPolygon = outPoly1; else outPolygon = outPoly2;
		}

		// edge4 - нижн€€ горизонтальна€ граница
		// dx = -1; dy = 0;
		{
			outVertices = 0;
			bool lastVertex = false;
			float px = inPolygon[0].pX;
			float py = inPolygon[0].pY;
			bool prevVertexInside = (py > miny);
			unsigned intersectionCount = 0;

			for (int i = 1; i <= inVertices; ++i)
			{
				float cx, cy; // - текуща€ вершина треугольника

				// инициализаци€ cx, cy
				if (i < inVertices)
				{
					cx = inPolygon[i].pX;
					cy = inPolygon[i].pY;
				} else
				{
					cx = inPolygon->pX;
					cy = inPolygon->pY;
					lastVertex = true;
				}

				// if starting vertex is visible then put it into the output array
				if (prevVertexInside) polyAdd (outPolygon, outVertices, lastVertex, px, py);

				bool curVertexInside = (cy > miny);

				// if vertices are on different sides of the edge then look where we're intersecting
				if (prevVertexInside != curVertexInside)
				{
					float denominator = cy - py;

					if (denominator != 0.0)
					{
						float t = (miny - py)/denominator;
						float tx, ty;

						if (t <= 0.0)
						{
							tx = px;
							ty = py;
						} else if (t >= 1.0)
						{
							tx = cx;
							ty = cy;
						} else
						{
							tx = px+t*(cx-px);
							ty = py+t*(cy-py);
						}

						polyAdd (outPolygon, outVertices, lastVertex, tx, ty);
					}

					if (++intersectionCount >= 2)
					{
						if (fabs(denominator) < 1) intersectionCount = 0; 
						else
						{
							// drop out, after adding all vertices left in input polygon
							if (curVertexInside)
							{
								memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(gPoint));
								outVertices += inVertices - i;
							}

							break;
						}
					}
				}

				px = cx;
				py = cy;

				prevVertexInside = curVertexInside;
			}
		}
	} while (0);

	// вывод результата
	if (outVertices < 3) result.vCount = 0;
	else
	{
		result.vCount = outVertices;
		memcpy (result.verts, outPolygon, outVertices*sizeof(gPoint));
	}
}
*/

// —проецировать точку на плоскость
inline void projPoint2Surface (point3 &pt, point3 &lt, point3 &n, float D, float invltDotn)
{
	const float DIST = 0.01;
	pt -= lt*(D + n.Dot(pt))*invltDotn;
	pt += DIST*n;
}

inline void addVertex (point3 *outPolygon, unsigned &outVertices, bool lastVertex, point3 pt)
{
	const float epsilon = 1e-6f;
	if (outVertices == 0 ||
		(
			fabs(pt.x - outPolygon[outVertices-1].x) > epsilon ||
			fabs(pt.y - outPolygon[outVertices-1].y) > epsilon ||
			fabs(pt.z - outPolygon[outVertices-1].z) > epsilon
		) && (
			!lastVertex ||
			fabs (pt.x - outPolygon[0].x) > epsilon ||
			fabs (pt.y - outPolygon[0].y) > epsilon ||
			fabs (pt.z - outPolygon[0].z) > epsilon
		)
	) 
	{
		outPolygon[outVertices] = pt;
		outVertices++;
	}
}

/*
//=====================================================================================//
//                             inline void chopTriangle()                              //
//=====================================================================================//
// ќбрезать треугольник плоскостью
inline void chopTriangle(const point3 *inPolygon, const point3 &n, const point3 &Pos,
						 point3 *outPolygon, unsigned &outVertices)
{
	unsigned inVertices = 3;
	outVertices = 0;
	bool lastVertex = false;
	point3 prev = inPolygon[0];
	bool prevVertexInside = (n.Dot(prev - Pos) > 0);
	unsigned intersectionCount = 0;

	for (int i = 0; i <= inVertices; ++i)
	{
		point3 cur;

		if (i < inVertices)
		{
			cur = inPolygon[i];
		} else
		{
			cur = *inPolygon;
			lastVertex = true;
		}

		if (prevVertexInside) addVertex (outPolygon, outVertices, lastVertex, prev);

		bool curVertexInside = (n.Dot(cur - Pos) > 0);

		if (prevVertexInside != curVertexInside)
		{
			float denominator = n.Dot(cur - prev);

			if (denominator != 0.0)
			{
				float t = n.Dot (Pos - prev) / denominator;
				point3 temp;

				if (t <= 0.0f)
				{
					temp = prev;
				} else if (t >= 1.0f)
				{
					temp = cur;
				} else
				{
					temp = prev + t*(cur - prev);
				}

				addVertex (outPolygon, outVertices, lastVertex, temp);
			}

			if (++intersectionCount >= 2)
			{
				if (fabs(denominator) < 1) intersectionCount = 0;
				else
				{
					if (curVertexInside)
					{
						memcpy (outPolygon + outVertices, inPolygon + i, (inVertices-i)*sizeof(point3));
						outVertices += inVertices - i;
					}

					break;
				}
			}
		}

		prev = cur;
		prevVertexInside = curVertexInside;

	}

	if (outVertices < 3) outVertices = 0;
}
*/

//  ак относительно плоскости расположен параллелепипед
// -1 - позади нормали
// 0 - на обеих сторонах
// 1 - спереди нормали
inline int TestPlanes(const point3 *originz, const Plane *p)
{
//	float testDot = p->Normal.Dot (n);
	float maxd, mind;
	float d1;

	maxd = mind = d1 = p->TestPoint(originz[0]);
	for (int i = 1; i < 8; ++i)
	{
		d1 = p->TestPoint (originz[i]);
		if (maxd < d1) maxd = d1; else if (mind > d1) mind = d1;
	}

//	maxd += 1; mind -= 1;

	if (maxd <= 0) return -1;
	if (mind >= 0)return 1;

	return 0;
}

// Ќаходитс€ ли треугольник в параллелепипеде тени
inline bool cullTriangle(const point3 *triangle, const point3 &x, const point3 &y, const point3 &z, const point3 *originz, float *Rad)
{
	float dot1, dot2, dot3, tdot;

	tdot = x.Dot (*originz);
	dot1 = x.Dot (triangle[0]) - tdot;
	dot2 = x.Dot (triangle[1]) - tdot;
	dot3 = x.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[0] < 0) && (-dot2 + Rad[0] < 0) && (-dot3 + Rad[0] < 0)) return true;

	tdot = y.Dot (*originz);
	dot1 = y.Dot (triangle[0]) - tdot;
	dot2 = y.Dot (triangle[1]) - tdot;
	dot3 = y.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[1] < 0) && (-dot2 + Rad[1] < 0) && (-dot3 + Rad[1] < 0)) return true;

	tdot = z.Dot (*originz);
	dot1 = z.Dot (triangle[0]) - tdot;
	dot2 = z.Dot (triangle[1]) - tdot;
	dot3 = z.Dot (triangle[2]) - tdot;

	if ((dot1 < 0) && (dot2 < 0) && (dot3 < 0)) return true;
	if ((-dot1 + Rad[2] < 0) && (-dot2 + Rad[2] < 0) && (-dot3 + Rad[2] < 0)) return true;

	return false;
}

}; // namespace