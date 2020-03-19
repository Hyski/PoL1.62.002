#if !defined(__PLANE_INCLUDED__)
#define __PLANE_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	Standard library files
#include <math.h>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/algebra/matrix3.h>
#include <mll/algebra/vector3.h>
#include <mll/algebra/point3.h>
/////////////////////////////////////////////////////////////////////////

namespace mll 
{ 

namespace geometry
{

///����� ���������
	/**
		���������� ��������� ��� ��������� ���� ���������
		\param T ��� ���������
	*/
template<class T>
class base_plane
{
public:

	algebra::base_vector3<T> m_normal;				///<	������� � ���������
	T m_distance;									///<	���������� �� ��������� �� ������ ���������

public:
	base_plane();
	base_plane(const base_plane<T>& p);
	base_plane(const algebra::base_vector3<T>& n,const algebra::base_point3<T>& pt);
	base_plane(const algebra::base_point3<T>& pt1,
			   const algebra::base_point3<T>& pt2,
			   const algebra::base_point3<T>& pt3);
	~base_plane();
public:
	///	�������� �����������
	base_plane<T>& operator=(const base_plane<T>& p);
	///	�������� ���������� �� ����� �� ���������
	T classify(const algebra::base_point3<T>& pt) const;
	///	�������� ���������� �� ����� �� ��������� c �������� ���������
	T classify(const algebra::base_point3<T>& pt,T epsilon) const;
	///	�������� ��������� �� ������� �������������
	base_plane<T>& operator*=(const algebra::base_matrix3<T>& m);

};

template<class T>
inline base_plane<T>::base_plane()
{
}

template<class T>
inline base_plane<T>::base_plane(const base_plane<T>& p) : m_normal(p.m_normal),
														   m_distance(p.m_distance)
{
}

template<class T>
inline base_plane<T>::base_plane(const algebra::base_vector3<T>& n,const algebra::base_point3<T>& pt)
{
	(m_normal = n).normalize();
	m_distance = pt.dot(-m_normal);//.dot(pt-origin);
}

template<class T>
inline base_plane<T>::base_plane(const algebra::base_point3<T>& pt1,
								 const algebra::base_point3<T>& pt2,
								 const algebra::base_point3<T>& pt3)
{
	(m_normal = (pt3-pt1).cross(pt2-pt1)).normalize();
	m_distance = pt1.dot(-m_normal);//.dot(pt1-origin);
}

template<class T>
inline base_plane<T>::~base_plane()
{
}
//	�������� �����������
template<class T>
inline base_plane<T>& base_plane<T>::operator=(const base_plane<T>& p)
{
	m_normal = p.m_normal;
	m_distance = p.m_distance;

	return *this;
}
//	�������� ���������� �� ����� �� ���������
template<class T>
inline T base_plane<T>::classify(const algebra::base_point3<T>& pt) const
{
    return pt.dot(m_normal) + m_distance; //.dot(pt-origin)+m_distance;
}
/**
\brief	�������� ���������� �� ����� �� ��������� c �������� ���������

���� ����� �������� � epsilon ����������� ���������, ������� ���������� 0
*/
template<class T>
inline T base_plane<T>::classify(const algebra::base_point3<T>& pt,T epsilon) const
{
    T result = pt.dot(m_normal) + m_distance; //.dot(pt-origin)+m_distance;
	return (fabsf(result)<m_epsilon)?0:result;
}
//	�������� ��������� �� ������� �������������
template<class T>
inline base_plane<T>& base_plane<T>::operator*=(const algebra::base_matrix3<T>& m)
{
	const algebra::base_point3<T> x0 = algebra::base_point3<T>((m_normal*(-m_distance))-algebra::origin);
	m_normal = m_normal*m;
	m_distance = -(m_normal.dot((x0*m)-algebra::origin));
	return *this;
}

template<class T>
inline base_plane<T> operator*(const base_plane<T>& p,const algebra::base_matrix3<T>& m)
{
	base_plane<T> res(p);
	return res*=m;
}

typedef base_plane<float> planef; ///<��������� � ������������ � ������������ � ��������� ������ ��������� ��������
typedef base_plane<float> plane;  ///<��������� � ������������ � ������������ � ��������� ������ ��������� ��������

}	//	namespace geometry

}	//	namespace mll

#endif