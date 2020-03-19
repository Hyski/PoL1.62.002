#if !defined(__RAY2_INCLUDED__)
#define __RAY2_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
//#include <mll/algebra/matrix2.h>
#include <mll/algebra/vector2.h>
#include <mll/algebra/point2.h>
/////////////////////////////////////////////////////////////////////////

namespace mll 
{ 

namespace geometry
{

///���������� ����� ���� � 2-� ������ ������������
template<class T>
class base_ray2
{
public:

//	typedef mll::algebra::base_matrix2<T> matrix_type;
	typedef mll::algebra::base_vector2<T> vector_type;
	typedef mll::algebra::base_point2<T> point_type;

	vector_type direction;		///<	����������� ����
	point_type origin;			///<	������� ����

public:

	///	����������� �� ���������
	base_ray2();

	///	����������� �����������
	base_ray2(const base_ray2<T>& r);

	///	����������� �� ����� � �����������
	base_ray2(const point_type& origin,const vector_type& direction);

	~base_ray2();

	///��������� ����� �� ���� �� ���������� ���������� �� ������ ����
	point_type at(T factor) const { assert( factor >= static_cast<T>(0.0) ); return origin + direction*factor; }
	///������������ �����������
	base_ray2 &normalize() { direction.normalize(); return *this; }

	///	�������� ������������
	const base_ray2& operator=(const base_ray2<T>& r);

	/////////////////////////////////////////////////////////////////////////
	//	�������� ����������������� ���� ��� ������ �������
//	base_ray2& operator*=(const matrix_type& m);
	/////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::base_ray2()
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::base_ray2(const base_ray2<T>& r) : origin(r.origin),
													    direction(r.direction)
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::base_ray2(const point_type& origin,const vector_type& direction) : origin(origin),
																						direction(direction)
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline base_ray2<T>::~base_ray2()
{
}
/////////////////////////////////////////////////////////////////////////
template<class T>
inline const base_ray2<T>& base_ray2<T>::operator=(const base_ray2<T>& r)
{
	origin = r.origin;
	direction = r.direction;

	return *this;
}
/////////////////////////////////////////////////////////////////////////
//	�������� ����������������� ���� ��� ������ �������
//template<class T>
//inline base_ray2<T>& base_ray2<T>::operator*=(const matrix_type& m)
//{
//	origin = origin*m;
//	direction = direction*m;
//
//	return *this;
//}
/////////////////////////////////////////////////////////////////////////
//	������� �������� ����������������� ���� ��� ������ �������
//template<class T>
//inline base_ray2<T> operator*(const base_ray2<T>& r,const base_ray2<T>::matrix_type& m)
//{
//	base_ray2 rv(r);
//	rv *= m;
//	return rv;
//}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	�������� ����� ������������ �����������
typedef base_ray2<float> ray2;  ///<��� � ������������ � ��������� ������ ��������� ��������
typedef base_ray2<float> ray2f; ///<��� � ������������ � ��������� ������ ��������� ��������
/////////////////////////////////////////////////////////////////////////

}	//	namespace debug

}	//	namespace mll

#endif