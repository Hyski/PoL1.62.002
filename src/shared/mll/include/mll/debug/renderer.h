#if !defined(__RENDERER_INCLUDED__)
#define __RENDERER_INCLUDED__

/////////////////////////////////////////////////////////////////////////
//	STL Library files
#include <string>
#include <list>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//	MiST land Library files
#include <mll/_choose_lib.h>
#include <mll/_export_rules.h>
#include <mll/algebra/point2.h>
#include <mll/algebra/point3.h>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
using mll::algebra::point2;
using mll::algebra::point3;
/////////////////////////////////////////////////////////////////////////

namespace mll
{

namespace geometry
{
	template<class T> class base_frustum;
	template<class T> class base_aabb3;
	template<class T> class base_obb3;
	template<class T> class base_ray3;
}

namespace debug
{
/*
class marker;
*/
//=====================================================================================//
//                                class render_strategy                                //
//=====================================================================================//
class render_strategy
{
public:
	virtual ~render_strategy() {}
	///	���������� ���������� �����
	virtual void put_line(const point2& begin,const point2& end,unsigned int color) = 0;
	///	���������� ���������� �����
	virtual void put_line(const point3& begin,const point3& end,unsigned int color) = 0;
	///	���������� ���������� �����
	virtual void put_text(const point2& origin,const std::string& text,unsigned int color) = 0;
	///	���������� ���������� �����
	virtual void put_text(const point3& origin,const std::string& text,unsigned int color) = 0;
	///	������������� ���������� ����� �� �������� ���������
	virtual point2 to_screen(const point3& pt) = 0;
	///	���������� ��� ���������������� ������ (���� ������� �������)
	virtual void flush(void) {}

};

//=====================================================================================//
//                                   class renderer                                    //
//=====================================================================================//
class MLL_EXPORT renderer
{
public:

	/////////////////////////////////////////////////////////////////////////
	class command;
	/////////////////////////////////////////////////////////////////////////

private:

	typedef std::list<command*> commands_t;				//	��� ���������� ������

	render_strategy* m_rs;								//	��������� �� ��������� ���������
	commands_t m_commands;								//	��������� �������
	unsigned int m_color;								//	������� ����
	point3 m_origin3;									//	������� ������ ���������� ��� 3D
	point2 m_origin2;									//	������� ������ ���������� ��� 2D
	float m_meter;										//	������ ����� (�������)

private:
	renderer();
	~renderer();
public:
	///	���������� �������
	renderer& operator<<(const geometry::base_frustum<float>& f);
	///	���������� ��������������� �� ���� ��������������� ����
	renderer& operator<<(const geometry::base_aabb3<float>& b);
	///	���������� ��������������� ��������������� ����
	renderer& operator<<(const geometry::base_obb3<float>& b);
	/// ������� ���
	renderer& operator<<(const geometry::base_ray3<float>& r);
	///	������� ������ ������
	renderer& operator<<(const std::string& text);

public:
	///	���������� ��������� ��������� (������� ����������)
	render_strategy* set_strategy(render_strategy* strategy);
	///	������� ������� ����� �������� ������
	renderer& operator<<(renderer& (*f)(renderer&));
	///	��������� ��� ����������� �������
	void flush(void);

public:
	///	���������� ������� ������� ��������� 3D ��������
	void set_origin3(const point3& origin);
	///	�������� ������� ������� ��������� 3D ��������
	const point3& get_origin3(void) const;
	///	���������� ������� ������� ��������� 2D ��������
	void set_origin2(const point2& origin);
	///	�������� ������� ������� ��������� 2D ��������
	const point2& get_origin2(void) const;
	///	���������� ������� ���� �����������
	void set_color(unsigned int color);
	///	�������� ������� ���� �����������
	unsigned int get_color(void) const;
	///	���������� ������� �������� �����
	void set_meter(float m);
	///	�������� ������� �������� �����
	float get_meter(void) const;
	///	�������� ��������� �����������
	render_strategy* get_rs(void);


public:
	///	���������� ������� � ������� (������������� ��� ������������� � �������������)
	void insert_command(command* c);
	///	�������� ��������� ������
	static renderer* instance(void);

	friend std::auto_ptr<renderer>;
};

//	������� ������� ����� �������� ������
inline renderer& renderer::operator<<(renderer& (*f)(renderer&))
{
	return f(*this);
}
//	�������� ������� ������ 3D ���������
inline const point3& renderer::get_origin3(void) const
{
	return m_origin3;
}
//	�������� ������� ������ 2D ���������
inline const point2& renderer::get_origin2(void) const
{
	return m_origin2;
}
//	�������� ������� ���� �����������
inline unsigned int renderer::get_color(void) const
{
	return m_color;
}
//	�������� ������� ��������� �����
inline float renderer::get_meter(void) const
{
	return m_meter;
}
//	���������� ������� ������ 3D ���������
inline void renderer::set_origin3(const point3& origin)
{
	m_origin3 = origin;
}
//	���������� ������� ������ 2D ���������
inline void renderer::set_origin2(const point2& origin)
{
	m_origin2 = origin;
}
//	���������� ������� ���� �����������
inline void renderer::set_color(unsigned int color)
{
	m_color = color;
}
//	���������� ������� �������� �����
inline void renderer::set_meter(float m)
{
	m_meter = m;
}
//	�������� ��������� �����������
inline render_strategy* renderer::get_rs(void)
{
	return m_rs;
}

//=====================================================================================//
//                               ��������� �������������                               //
//=====================================================================================//
//=====================================================================================//
//                                   struct strategy                                   //
//=====================================================================================//
///	���������� ��������� ���������
struct strategy
{
	mll::debug::render_strategy* m_rs;
	strategy(mll::debug::render_strategy* rs) : m_rs(rs) {}
	friend mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::strategy& s);
};

inline mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::strategy& s)
{
	r.set_strategy(s.m_rs);
	return r;
}

//=====================================================================================//
//                            mll::debug::renderer& flush()                            //
//=====================================================================================//
///	���������� ��� ���������
inline mll::debug::renderer& flush(mll::debug::renderer& r)
{
	r.flush();
	return r;
}

//=====================================================================================//
//                                    struct color                                     //
//=====================================================================================//
///	���������� ������� ����
struct color
{
	unsigned int m_value;
	color(unsigned int value) : m_value(value) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::color& c);
};

//=====================================================================================//
//                                   struct moveto2                                    //
//=====================================================================================//
///	���������� ������� ������ 2D ���������
struct moveto2
{
	point2 m_value;
	moveto2(const point2& value) : m_value(value) {}
	moveto2(float x,float y) : m_value(x,y) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::moveto2& o);
};

//=====================================================================================//
//                                   struct moveto3                                    //
//=====================================================================================//
///	���������� ������� ������ 3D ���������
struct moveto3
{
	point3 m_value;
	moveto3(const point3& value) : m_value(value) {}
	moveto3(float x,float y,float z) : m_value(x,y,z) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::moveto3& o);
};

//=====================================================================================//
//                                    struct text3                                     //
//=====================================================================================//
///	������� ���� ������ �����
struct text3
{
	const std::string m_value;
	text3(const std::string& value) : m_value(value) {}
	friend MLL_EXPORT mll::debug::renderer& operator<<(mll::debug::renderer& r,const mll::debug::text3& o);
};

//=====================================================================================//
//                        ������� ��� ������������� �����������                        //
//=====================================================================================//
#if defined(MLL_ENABLE_DEBUG_RENDERER)
#define MLL_DEBUG_RENDER(List)				*mll::debug::renderer::instance() << List
#else
#define MLL_DEBUG_RENDER(List)				
#endif

/*
/////////////////////////////////////////////////////////////////////////
//////////////////////////    class renderer    //////////////////////////
/////////////////////////////////////////////////////////////////////////
class MLL_EXPORT renderer
{
public:

	/////////////////////////////////////////////////////////////////////////
	class command;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	///////////////////////////    class meter    ///////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class meter
	{
	public:
		float value;
		meter() : value(1) {}
		meter(const meter& m) : value(m.value) {}
		meter(float v) : value(v) {}
		~meter() {}
		const meter& operator=(const meter& m) { value=m.value; return *this; }
	};
	/////////////////////////////////////////////////////////////////////////

private:

	/////////////////////////////////////////////////////////////////////////
	typedef std::list<command*> commands_t;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	commands_t m_commands;					//	������ �������
	render_strategy* m_rs;					//	������ �� ��������� ���������
	point2 m_origin2;						//	������ ���������� ��� 2D
	point3 m_origin3;						//	������ ���������� ��� 3D
	color m_color;							//	���� �� ���������
	meter m_meter;							//	�������� �����
	/////////////////////////////////////////////////////////////////////////

public:
	renderer(render_strategy* rs);
	~renderer();
public:
	//	���������� ������
	renderer& operator<<(const marker& m);
	//	���������� ��������������� �� ���� ��������������� ����
	renderer& operator<<(const geometry::base_aabb3<float> &r);
	//	���������� �������
	renderer& operator<<(const geometry::base_frustum<float> &r);
	//	���������� �����
	renderer& operator<<(const line &l);
	//	������� ��������� �����
	renderer& operator<<(const text2& t);
	//	���������� ���
	renderer& operator<<(const geometry::base_ray3<float>& r);
	//	���������� ���� �� ���������
	renderer& operator<<(const color& c);
	//	���������� �������� ����� �� ���������
	renderer& operator<<(const meter& m);
	//	���������� ���
	void flush(void);

public:

	/////////////////////////////////////////////////////////////////////////
	//	���������� ������� ���� �����������
	void set_color(const color& c);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������� ���� �����������
	const color& get_color(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	���������� ������� ������ 2D ���������
	void set_origin2(const point2& origin);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������� ������ 2D ���������
	const point2& get_origin2(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	���������� ������� ������ 3D ���������
	void set_origin3(const point3& origin);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������� ������ 3D ���������
	const point3& get_origin3(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	���������� ������� �������� �����
	void set_meter(const meter& m);
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ������� ��������� �����
	const meter& get_meter(void) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	//	�������� ��������� �����������
	render_strategy* get_rs(void);
	/////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////
//	�������� ������� ���� �����������
inline const color& renderer::get_color(void) const
{
	return m_color;
}
/////////////////////////////////////////////////////////////////////////
//	�������� ������� ������ 2D ���������
inline const point2& renderer::get_origin2(void) const
{
	return m_origin2;
}
/////////////////////////////////////////////////////////////////////////
//	�������� ������� ������ 3D ���������
inline const point3& renderer::get_origin3(void) const
{
	return m_origin3;
}
/////////////////////////////////////////////////////////////////////////
//	�������� ������� ��������� �����
inline const renderer::meter& renderer::get_meter(void) const
{
	return m_meter;
}
/////////////////////////////////////////////////////////////////////////
//	�������� ��������� �����������
inline render_strategy* renderer::get_rs(void)
{
	return m_rs;
}
/////////////////////////////////////////////////////////////////////////

//=====================================================================================//
//                                     class text2                                     //
//=====================================================================================//
class MLL_EXPORT text2
{
public:

	mll::algebra::point2 m_pt;						//	����� ������
	std::string m_text;								//	�����

public:
	text2();
	text2(const char* text,...);
	text2(int x,int y,const char* text,...);
	text2(const mll::algebra::point2& pt,const char* text,...);

};
*/
}	//	namespace debug

}	//	namespace mll



#endif