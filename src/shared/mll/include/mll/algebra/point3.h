#if !defined(__POINT3_INCLUDED__)
#define __POINT3_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/algebra/_typedefs.h>
#include <mll/algebra/vector3.h>
#include <mll/io/iutils.h>
/////////////////////////////////////////////////////////////////////////

#include <ostream>	// ��� �������� ��������� ������ � �����

namespace mll
{

namespace algebra
{

///�������������� 3-� ������ �����.
/**
	���� ����� ���������. ��� ��������� �������� ���������� �������.
	\param T ��� ����������.
*/
template<class T>
class base_point3
{
public:

	typedef typename numerical_traits<T>::quotient_type quotient_type;

	///����������
	union
	{
		struct
		{
			T x,y,z;
		};
		T v[3];
	};

	base_point3() {}
	base_point3(const base_point3& v) : x(v.x),y(v.y),z(v.z) {}
	base_point3(T _x,T _y,T _z) : x(_x),y(_y),z(_z) {}
	base_point3(const base_origin&) : x(numerical_traits<T>::get_null()),
											   y(numerical_traits<T>::get_null()),
											   z(numerical_traits<T>::get_null()) {}

	~base_point3() {}
///�������� ������������
	base_point3& operator=(const base_point3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}

///�������� ������������
	base_point3& operator=(const base_origin& v)
	{
		x = y = z =numerical_traits<T>::get_null();

		return *this;
	}

///������� +
	base_point3 operator+(void) const
	{
		return *this;
	}

///������� -
	base_point3 operator-(void) const
	{
		return base_point3(-x,-y,-z);
	}

///�������� � �������������
	const base_point3& operator+=(const base_vector3<T>& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}
///��������� � �������������
	const base_point3& operator-=(const base_vector3<T>& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

///��������� � �������������
	const base_point3& operator*=(const T& v)
	{
		x *= v;
		y *= v;
		z *= v;

		return *this;
	}
///������� � �������������
	const base_point3& operator/=(const T& v)
	{
		const quotient_type iv = numerical_traits<T>::inverse(numerical_traits<T>::to_quotient_type(v));

		x *= iv;
		y *= iv;
		z *= iv;

		return *this;
	}

///��������� �� ���������
	bool operator==(const base_point3& v) const 
	{
		return x==v.x && y==v.y && z==v.z;
	}
///��������� ������
	bool operator<(const base_point3& v) const 
	{
		return x<v.x || (x==v.x && (y<v.y || (y==v.y && z<v.z))) ;
	}
///��������� �� �����������
	bool operator!=(const base_point3& v) const 
	{
		return x!=v.x || y!=v.y || z!=v.z;
	}

///��������� ������������
	T dot(const base_vector3<T>& v) const
	{
		return x*v.x + y*v.y + z*v.z;
	}
///�������� ������������ � ������������� factor
	base_point3 lerp(const base_point3 &p,quotient_type factor) const
	{
		return *this + (p-*this)*factor;
	}
};


template<class T>
inline base_vector3<T> operator-(const base_point3<T>& u,const base_point3<T>& v)
{
	return base_vector3<T>(u.x-v.x,u.y-v.y,u.z-v.z);
}

template<class T>
inline base_point3<T> operator+(const base_point3<T>& u,const base_vector3<T>& v)
{
	return base_point3<T>(u.x+v.x,u.y+v.y,u.z+v.z);
}

template<class T>
inline base_point3<T> operator-(const base_point3<T>& u,const base_vector3<T>& v)
{
	return base_point3<T>(u.x-v.x,u.y-v.y,u.z-v.z);
}

template<class T>
inline base_point3<T> operator+(const base_vector3<T>& v,const base_point3<T>& u)
{
	return base_point3<T>(u.x+v.x,u.y+v.y,u.z+v.z);
}

template<class T>
inline base_point3<T> operator-(const base_vector3<T>& v,const base_point3<T>& u)
{
	return base_point3<T>(v.x-u.x,v.y-u.y,v.z-u.z);
}

template<class T>
inline base_point3<T> operator+(const base_origin&,const base_vector3<T>& v)
{
	return base_point3<T>(v.x,v.y,v.z);
}

template<class T>
inline base_point3<T> operator-(const base_origin&,const base_vector3<T>& v)
{
	return base_point3<T>(-v.x,-v.y,-v.z);
}

template<class T>
inline base_point3<T> operator+(const base_vector3<T>& v,const base_origin&)
{
	return base_point3<T>(v.x,v.y,v.z);
}

template<class T>
inline base_point3<T> operator-(const base_vector3<T>& v,const base_origin&)
{
	return base_point3<T>(v.x,v.y,v.z);
}

template<class T>
inline base_vector3<T> operator-(const base_point3<T>& v,const base_origin&)
{
	return base_vector3<T>(v.x,v.y,v.z);
}

template<class T>
inline base_point3<T> operator*(const base_point3<T> &p, const T &f)
{
	base_point3<T> r(p);
	r *= f;
	return r;
}

template<class T>
inline base_point3<T> operator*(const T &f, const base_point3<T> &p)
{
	base_point3<T> r(p);
	r *= f;
	return r;
}

template<class T>
inline base_point3<T> operator/(const base_point3<T> &p, const T &f)
{
	base_point3<T> r(p);
	r /= f;
	return r;
}

template<class T>
inline base_point3<T> operator/(const T &f, const base_point3<T> &p)
{
	base_point3<T> r(p);
	r /= f;
	return r;
}

typedef base_point3<float> point3; ///< ����� � ������������ � ��������� ������ ��������� ��������
typedef base_point3<float> point3f;///< ����� � ������������ � ��������� ������ ��������� ��������
typedef base_point3<int> point3i;  ///< ����� � �������������� ������������

///����� � ����� stl
template<typename T>
inline ::std::ostream &operator<< (::std::ostream &stream, const base_point3<T> &pt)
{
	stream << "point3(" << pt.x << "," << pt.y << "," << pt.z << ")";
	return stream;
}

///������ �� ������ stl
template<typename T>
inline ::std::istream &operator>> (::std::istream &stream, base_point3<T> &pt)
{
	stream >> io::verify("point3") >> std::ws >> io::verify("(");
	stream >> std::ws >> pt.x >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.y >> std::ws >> io::verify(",");
	stream >> std::ws >> pt.z >> std::ws >> io::verify(")");
	return stream;
}

///������ �� ��������� ������
template<typename T>
inline io::ibinstream &operator>>(io::ibinstream &stream, base_point3<T> &pt)
{
	return io::do_raw_input(stream,pt);
}

///����� � �������� �����
template<typename T>
inline io::obinstream &operator<<(io::obinstream &stream, const base_point3<T> &pt)
{
	return io::do_raw_output(stream,pt);
}

}	//	namesapce algebra

}	//	namesapce mll


#endif