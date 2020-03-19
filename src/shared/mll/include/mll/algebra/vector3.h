#if !defined(__VECTOR3_INCLUDED__)
#define __VECTOR3_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/algebra/numerical_traits.h>
#include <mll/algebra/_typedefs.h>
#include <mll/io/binstream.h>
#include <mll/io/iutils.h>
/////////////////////////////////////////////////////////////////////////

#include <ostream>

namespace mll
{

namespace algebra
{

/////////////////////////////////////////////////////////////////////////
////////////////////////    class base_vector3    ////////////////////////
/////////////////////////////////////////////////////////////////////////
template<class T>
class base_vector3
{
public:

	/////////////////////////////////////////////////////////////////////////
	typedef typename numerical_traits<T>::quotient_type quotient_type;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	union
	{
		struct
		{
			T x,y,z;
		};
		T v[3];
	};
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	����������� �� ���������
	base_vector3() {}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	��������������� �������� �������
	base_vector3(base_null_vector)
	:	x(numerical_traits<T>::get_null()),
		y(numerical_traits<T>::get_null()),
		z(numerical_traits<T>::get_null()) {}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	����������� �����������
	base_vector3(const base_vector3& v) : x(v.x),y(v.y),z(v.z) {}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	����������� �����������
	base_vector3(T _x,T _y,T _z) : x(_x),y(_y),z(_z) {}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	����������
	~base_vector3() {}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� �����������
	base_vector3& operator=(const base_vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������� ����
	base_vector3 operator+(void) const
	{
		return *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������� �����
	base_vector3 operator-(void) const
	{
		return base_vector3(-x,-y,-z);
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������������� ��������
	const base_vector3& operator+=(const base_vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������������� ���������
	const base_vector3& operator-=(const base_vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������������� ���������
	const base_vector3& operator*=(const T& v)
	{
		x *= v;
		y *= v;
		z *= v;

		return *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������������� �������
	const base_vector3& operator/=(const T& v)
	{
		const quotient_type iv = numerical_traits<T>::inverse(numerical_traits<T>::to_quotient_type(v));

		x *= iv;
		y *= iv;
		z *= iv;

		return *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������� �� �������
	T operator[](int index)
	{
		return v[index];
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	����������� �������� ������� �� �������
	const T& operator[](int index) const
	{
		return v[index];
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	���������� ������������ ����� �������
	T sqr_length(void) const
	{
		return x*x+y*y+z*z;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	���������� ����� �������
	quotient_type length(void) const 
	{
		return numerical_traits<T>::sqrt(sqr_length());
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	������������ �������
	base_vector3& normalize(void)
	{
		if(!numerical_traits<T>::is_null(sqr_length()))
		{
			const quotient_type iv = numerical_traits<T>::inverse(length());
			x *= iv;
			y *= iv;
			z *= iv;
		}

		return *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	��������� ������������ ��������
	T dot(const base_vector3& v) const
	{
		return x*v.x + y*v.y + z*v.z;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	��������� ������������ ��������
	base_vector3 cross(const base_vector3& v) const
	{
		return base_vector3(y*v.z - z*v.y,
							z*v.x - x*v.z,
							x*v.y - y*v.x);
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������������ ������� (factor(0) - �������� ������, factor(1) - ������������ ������)
	base_vector3 lerp(const base_vector3 &v, quotient_type factor) const
	{
		return (v-*this)*factor + *this;
	}
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	///��������� ������
	bool operator<(const base_vector3& v) const 
	{
		return x<v.x || (x==v.x && (y<v.y || (y==v.y && z<v.z))) ;
	}

};

template<class T>
inline base_vector3<T> operator+(const base_vector3<T>& u,const base_vector3<T>& v)
{
	return base_vector3<T>(u.x+v.x,u.y+v.y,u.z+v.z);
}

template<class T>
inline base_vector3<T> operator-(const base_vector3<T>& u,const base_vector3<T>& v)
{
	return base_vector3<T>(u.x-v.x,u.y-v.y,u.z-v.z);
}

template<class T>
inline base_vector3<T> operator*(const T& f, const base_vector3<T>& v)
{
	return base_vector3<T>(v.x*f,v.y*f,v.z*f);
}

template<class T>
inline base_vector3<T> operator*(const base_vector3<T>& v, const T& f)
{
	return base_vector3<T>(v.x*f,v.y*f,v.z*f);
}

template<class T>
inline base_vector3<T> operator/(const base_vector3<T>& v, const T& f)
{
	base_vector3<T> result(v);
	result *= numerical_traits<T>::inverse(f);
	return result;
}

template<class T>
inline bool operator==(const base_vector3<T>& u,const base_vector3<T>& v)
{
	return u.x==v.x && u.y==v.y && u.z==v.z;
}

template<class T>
inline bool operator!=(const base_vector3<T>& u,const base_vector3<T>& v)
{
	return (u.x!=v.x) || (u.y!=v.y) || (u.z!=v.z);
}

/////////////////////////////////////////////////////////////////////////
//	�������� ����� ������������ �����������
typedef base_vector3<float> vector3;
typedef base_vector3<float> vector3f;
typedef base_vector3<int> vector3i;
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// operator<< () /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline ::std::ostream &operator<< (::std::ostream &stream, const base_vector3<T> &pt)
{
	stream << "vector3(" << pt.x << "," << pt.y << "," << pt.z << ")";
	return stream;
}

/*=====================================================================================*\
 *                                    operator>> ()                                    * 
\*=====================================================================================*/
template<typename T>
inline ::std::istream &operator>> (::std::istream &stream, base_vector3<T> &pt)
{
	stream >> std::ws >> io::verify("vector3") >> std::ws >> io::verify("(");
	stream >> std::ws >> pt.x >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.y >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.z >> std::ws >> io::verify(")");
	return stream;
}

//=====================================================================================//
//                                    operator>>()                                     //
//=====================================================================================//
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_vector3<T> &pt)
{
	return io::do_raw_input(stream,pt);
}

//=====================================================================================//
//                                    operator<<()                                     //
//=====================================================================================//
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_vector3<T> &pt)
{
	return io::do_raw_output(stream,pt);
}

}	//	namesapce algebra

}	//	namesapce mll


#endif