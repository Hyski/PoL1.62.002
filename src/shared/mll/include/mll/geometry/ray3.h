#if !defined(__RAY3_INCLUDED__)
#define __RAY3_INCLUDED__

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

///���������� ����� ���� � 3-� ������ ������������
template<class T>
class base_ray3
{
public:

	typedef mll::algebra::base_matrix3<T> matrix_type;
	typedef mll::algebra::base_vector3<T> vector_type;
	typedef mll::algebra::base_point3<T> point_type;

	vector_type direction;						///<	����������� ����
	point_type origin;							///<	������� ����

public:

	///	����������� �� ���������
	base_ray3();

	///	����������� �����������
	base_ray3(const base_ray3<T>& r);

	///	����������� �� ����� � �����������
	base_ray3(const point_type& origin,const vector_type& direction);

	///	����������
	~base_ray3();

	///	��������� ����� �� ���� �� ���������� ���������� �� ������ ����
	point_type at(T factor) const;

	///	������������ �����������
	base_ray3 &normalize();

	///	�������� �� ����������� � ������������� (���������� ���������� �� ����)
	T intersect_triangle(const point_type& v1,const point_type& v2,const point_type& v3) const;

	///	��������� ����� ����������� � ������������� � ���������������� �����������
	///	���������� ����: ���������� �� ����, ����� � ���������������� �����������
	std::pair<T,point_type> hit_triangle(const point_type& v1,const point_type& v2,const point_type& v3) const;

	///	�������� ������������
	const base_ray3& operator=(const base_ray3<T>& r);

	///	�������� ����������������� ���� ��� ������ �������
	base_ray3& operator*=(const matrix_type& m);
};

template<class T>
inline base_ray3<T>::base_ray3()
{
}

template<class T>
inline base_ray3<T>::base_ray3(const base_ray3<T>& r) : origin(r.origin),
													    direction(r.direction)
{
}

template<class T>
inline base_ray3<T>::base_ray3(const point_type& origin,const vector_type& direction) : origin(origin),
																						direction(direction)
{
}

template<class T>
inline base_ray3<T>::~base_ray3()
{
}

//	��������� ����� �� ���� �� ���������� ���������� �� ������ ����
template<class T>
inline typename base_ray3<T>::point_type base_ray3<T>::at(T factor) const
{ 
	assert( factor >= static_cast<T>(0.0) );
	return origin + direction*factor; 
}

//	������������ �����������
template<class T>
inline base_ray3<T>& base_ray3<T>::normalize()
{ 
	direction.normalize();
	return *this; 
}

//	�������� �� ����������� � ������������� (���������� ���������� �� ����)
template<class T>
inline T base_ray3<T>::intersect_triangle(const point_type& v1,const point_type& v2,const point_type& v3) const
{
	return hit_triangle(v1,v2,v3).first;
}

//	��������� ����� ����������� � ������������� � ���������������� �����������
//	���������� ����: ���������� �� ����, ����� � ���������������� �����������
template<class T>
inline std::pair<T,typename base_ray3<T>::point_type> base_ray3<T>::hit_triangle(const point_type& v1,const point_type& v2,const point_type& v3) const
{
	typedef mll::algebra::numerical_traits<T> ntraits_t;
	const T epsilon = static_cast<T>(1.0e-5);
	const T one_epsilon = ntraits_t::get_one() + epsilon;

	const mll::algebra::vector3 e1 = v1-v3;
	const mll::algebra::vector3 e2 = v2-v3;
	const mll::algebra::vector3 p  = direction.cross(e2);

	const T a = e1.dot(p);

	if(ntraits_t::abs(a) >= epsilon)
	{
		const T f = ntraits_t::get_one()/a;
		const mll::algebra::vector3 s = origin-v3;
		const T u = f*(s.dot(p));

		if(u >= -epsilon && u <= one_epsilon)
		{
			const mll::algebra::vector3 q = s.cross(e1);
			const T v = f*(direction.dot(q));

			if(v >= -epsilon && (u+v) <= one_epsilon)
			{
				if(f*e2.dot(q) >= -epsilon)
				{
					return std::pair<T,point_type>(f*e2.dot(q),point_type(u,v,ntraits_t::get_one()-u-v));
				}
			}
		}
	}

	return std::pair<T,point_type>(ntraits_t::get_bad_ray_intersection(),point_type());
}

template<class T>
inline const base_ray3<T>& base_ray3<T>::operator=(const base_ray3<T>& r)
{
	origin = r.origin;
	direction = r.direction;

	return *this;
}

template<class T>
inline base_ray3<T>& base_ray3<T>::operator*=(const matrix_type& m)
{
	origin = origin*m;
	direction = direction*m;

	return *this;
}

template<class T>
inline base_ray3<T> operator*(const base_ray3<T>& r,const typename base_ray3<T>::matrix_type& m)
{
	base_ray3<T> rv(r);
	rv *= m;
	return rv;
}

/////////////////////////////////////////////////////////////////////////
//	�������� ����� ������������ �����������
typedef base_ray3<float> ray3;  ///<��� � ������������ � ��������� ������ ��������� ��������
typedef base_ray3<float> ray3f; ///<��� � ������������ � ��������� ������ ��������� ��������
/////////////////////////////////////////////////////////////////////////

}	//	namespace debug

}	//	namespace mll

#endif