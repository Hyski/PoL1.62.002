#include "precomp.h"
#include <mll/debug/renderer.h>
#include <mll/geometry/frustum.h>

/////////////////////////////////////////////////////////////////////////
using namespace mll::geometry;
using namespace mll::algebra;
using namespace mll::debug;
/////////////////////////////////////////////////////////////////////////

//=====================================================================================//
//                               class renderer::command                               //
//=====================================================================================//
class renderer::command
{
public:
	virtual ~command() {}
	///	выполнить команду
	virtual void execute(renderer& r) = 0;

};

namespace commands
{

//=====================================================================================//
//                                   class set_text2                                   //
//=====================================================================================//
class set_text2 : public renderer::command
{
private:
	const std::string m_text;			//	отображаемый текст
public:
	///	сконструировать команду
	set_text2(const std::string& text) : m_text(text) {}
	///	выполнить команду
	void execute(renderer& r) { r.get_rs()->put_text(r.get_origin2(),m_text,r.get_color()); }
};

//=====================================================================================//
//                                   class set_text3                                   //
//=====================================================================================//
class set_text3 : public renderer::command
{
private:
	const std::string m_text;			//	отображаемый текст
public:
	///	сконструировать команду
	set_text3(const std::string& text) : m_text(text) {}
	///	выполнить команду
	void execute(renderer& r) { r.get_rs()->put_text(r.get_origin3(),m_text,r.get_color()); }
};


//=====================================================================================//
//                                  class set_origin2                                  //
//=====================================================================================//
class set_origin2 : public renderer::command
{
private:
	point2 m_point;				//	значение точки
public:
	///	сконструировать команду
	set_origin2(const point2& pt) : m_point(pt) {}
	///	выполнить команду
	void execute(renderer& r) { r.set_origin2(m_point); }
};

//=====================================================================================//
//                                  class set_origin3                                  //
//=====================================================================================//
class set_origin3 : public renderer::command
{
private:
	point3 m_point;				//	значение точки
public:
	///	сконструировать команду
	set_origin3(const point3& pt) : m_point(pt) {}
	///	выполнить команду
	void execute(renderer& r) { r.set_origin3(m_point); }
};

//=====================================================================================//
//                                   class set_color                                   //
//=====================================================================================//
class set_color : public renderer::command
{
private:
	unsigned int m_color;		//	значение цвета
public:
	///	сконструировать команду
	set_color(unsigned int color) : m_color(color) {}
	///	выполнить команду
	void execute(renderer& r) { r.set_color(m_color); }
};

//=====================================================================================//
//                                  class put_rline3                                   //
//=====================================================================================//
class put_rline3 : public renderer::command
{
private:
	point3 m_begin,m_end;
public:
	//	сконструировать команду
	put_rline3(const point3& begin,const point3& end) : m_begin(begin),m_end(end) {}
	//	выполнить команду
	void execute(renderer& r) { r.get_rs()->put_line(r.get_origin3()+(m_begin-origin)*r.get_meter(),r.get_origin3()+(m_end-origin)*r.get_meter(),r.get_color()); }
};

}

//=====================================================================================//
//                                   class renderer                                    //
//=====================================================================================//
renderer::renderer()
: m_rs(0),
  m_meter(1),
  m_origin2(origin),
  m_origin3(origin),
  m_color(0xffffffff)
{
}

renderer::~renderer()
{
	//	удаляем невыполненые команды
	for(commands_t::iterator it=m_commands.begin();it!=m_commands.end();++it)
		delete *it;
}
//	установить стратегию отрисовки (вернуть предыдущую)
render_strategy* renderer::set_strategy(render_strategy* strategy)
{
	render_strategy* prev = m_rs;
	m_rs = strategy;
	return prev;
}
//	выполнить все накопленные команды
void renderer::flush(void)
{
	if(m_rs)
	{
		//	выполняем и удаляем команды
		for(commands_t::iterator it=m_commands.begin();it!=m_commands.end();++it)
		{ (*it)->execute(*this); delete *it; }
		//	вызываем у статегии сброс буферизированных данных
		m_rs->flush();
	}
	else
	{
		//	просто удаляем команды
		for(commands_t::iterator it=m_commands.begin();it!=m_commands.end();++it)
		{ delete *it; }
	}
	//	очищаем массив от указателей
	m_commands.clear();
}
//	установить команду в очередь (предназначено для использования в манипуляторах)
void renderer::insert_command(command* c)
{
	assert( c );
	m_commands.push_back(c);
}
//	отобразить фрастум
renderer& renderer::operator<<(const base_frustum<float>& f)
{
	m_commands.push_back(new commands::set_origin3(point3(0,0,0)));
	for(int i=0;i<4;i++)
	{
		m_commands.push_back(new commands::put_rline3(f[i],f[i+4]));
		m_commands.push_back(new commands::put_rline3(f[i],f[(i+1)%4]));
		m_commands.push_back(new commands::put_rline3(f[i+4],f[(i+1)%4+4]));
	}

	return *this;
}
//	отобразить ориентированный по осям орграничивающий бокс
renderer& renderer::operator<<(const base_aabb3<float>& b)
{
	m_commands.push_back(new commands::set_origin3(point3(0,0,0)));
	for(int i=0;i<4;i++)
	{
		m_commands.push_back(new commands::put_rline3(b[i],b[i+4]));
		m_commands.push_back(new commands::put_rline3(b[i],b[(i+1)%4]));
		m_commands.push_back(new commands::put_rline3(b[i+4],b[(i+1)%4+4]));
	}

	return *this;
}
//	отобразить ориентированный орграничивающий бокс
renderer& renderer::operator<<(const base_obb3<float>& b)
{
	typedef base_obb3<float> bo_t;

	m_commands.push_back(new commands::set_origin3(point3(0,0,0)));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_origin],b[bo_t::pt_x]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_origin],b[bo_t::pt_y]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_origin],b[bo_t::pt_z]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_xy],b[bo_t::pt_x]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_xy],b[bo_t::pt_y]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_xy],b[bo_t::pt_xyz]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_xz],b[bo_t::pt_x]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_xz],b[bo_t::pt_z]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_xz],b[bo_t::pt_xyz]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_yz],b[bo_t::pt_y]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_yz],b[bo_t::pt_z]));
	m_commands.push_back(new commands::put_rline3(b[bo_t::pt_yz],b[bo_t::pt_xyz]));

	return *this;
}

//	вывести луч
renderer& renderer::operator<<(const base_ray3<float>& r)
{
	vector3 auxiliary_axis = (vector3(r.direction).normalize()==vector3(1,0,0))?vector3(0,1,0):vector3(1,0,0);
	const point3 rend = r.direction-origin;

	vector3 ox(r.direction);
	vector3 oy(ox.cross(auxiliary_axis));
	vector3 oz(ox.cross(oy));

	ox.normalize();
	oy.normalize();
	oz.normalize();

	matrix3 rm(
		ox.x,oy.x,oz.x,0,
		ox.y,oy.y,oz.y,0,
		ox.z,oy.z,oz.z,0,
		rend.dot(-ox),rend.dot(-oy),rend.dot(-oz),1);

	m_commands.push_back(new commands::set_origin3(r.origin));
	m_commands.push_back(new commands::put_rline3(origin,rend));

	const int lines_num = 8;
	const float one = 0.05f;
	const float one3m = one*3.0f;
	point3 const_points[2*lines_num] =
	{
		point3(0,0,0), 
		point3(-one3m,-one,0),
		point3(0,0,0), 
		point3(-one3m,one,0),
		point3(0,0,0), 
		point3(-one3m,0,-one),
		point3(0,0,0), 
		point3(-one3m,0,one),

		point3(-one3m,-one,0),
		point3(-one3m,0,one),

		point3(-one3m,0,one),
		point3(-one3m,one,0),

		point3(-one3m,one,0),
		point3(-one3m,0,-one),

		point3(-one3m,0,-one),
		point3(-one3m,-one,0)
	};

	point3* points = const_points;

	rm.invert();

	for(int i=0;i<lines_num;i++)
	{
		*points = (*points)*rm; points++;
		*points = (*points)*rm; points++;
		m_commands.push_back(new commands::put_rline3(*(points-2),*(points-1)));
	}

	return *this;
}
//	вывести строку текста
renderer& renderer::operator<<(const std::string& text)
{
	m_commands.push_back(new commands::set_text2(text));
	return *this;
}

//=====================================================================================//
//                     внешние операторы сдвига для манипуляторов                      //
//=====================================================================================//
mll::debug::renderer& mll::debug::operator<<(mll::debug::renderer& r,const mll::debug::color& c)
{
	r.insert_command(new commands::set_color(c.m_value));
	return r;
}

mll::debug::renderer& mll::debug::operator<<(mll::debug::renderer& r,const mll::debug::moveto2& o)
{
	r.insert_command(new commands::set_origin2(o.m_value));
	return r;
}

mll::debug::renderer& mll::debug::operator<<(mll::debug::renderer& r,const mll::debug::moveto3& o)
{
	r.insert_command(new commands::set_origin3(o.m_value));
	return r;
}

mll::debug::renderer& mll::debug::operator<<(mll::debug::renderer& r,const mll::debug::text3& o)
{
	r.insert_command(new commands::set_text3(o.m_value));
	return r;
}


//=====================================================================================//
//                                renderer::m_instance                                 //
//=====================================================================================//
//std::auto_ptr<renderer> renderer::m_instance;

//	получить инстанцию класса
renderer* renderer::instance(void)
{
	static renderer inst;
	return &inst;
}

/*
/////////////////////////////////////////////////////////////////////////
#include <mll/geometry/frustum.h>
#include <mll/geometry/aabb3.h>
#include <mll/io/printf.h>
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
using namespace mll::geometry;
using namespace mll::algebra;
using namespace mll::debug;
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////    class renderer::command    /////////////////////
/////////////////////////////////////////////////////////////////////////
class renderer::command
{
public:
	virtual ~command() {}

	/////////////////////////////////////////////////////////////////////////
	//	выполнить команду
	virtual void execute(renderer* r) = 0;
	/////////////////////////////////////////////////////////////////////////
};
/////////////////////////////////////////////////////////////////////////

namespace commands
{
	/////////////////////////////////////////////////////////////////////////
	///////////////////////    class set_origin3    /////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class set_origin3 : public renderer::command
	{
	private:
		point3 m_point;
	public:
		/////////////////////////////////////////////////////////////////////////
		//	сконструировать команду
		set_origin3(const point3& pt) : m_point(pt) {}
		/////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////
		//	выполнить команду
		void execute(renderer* r) { r->set_origin3(m_point); }
		/////////////////////////////////////////////////////////////////////////
	};
	/////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////

	////////////////////////    class put_rline3    /////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class put_rline3 : public renderer::command
	{
	private:
		point3 m_begin,m_end;
	public:
		/////////////////////////////////////////////////////////////////////////
		//	сконструировать команду
		put_rline3(const point3& begin,const point3& end) : m_begin(begin),m_end(end) {}
		/////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////
		//	выполнить команду
		void execute(renderer* r) { r->get_rs()->put_line(r->get_origin3()+(m_begin-origin)*r->get_meter().value,r->get_origin3()+(m_end-origin)*r->get_meter().value,r->get_color().value); }
		/////////////////////////////////////////////////////////////////////////
	};
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	///////////////////////    class set_origin2    /////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class set_origin2 : public renderer::command
	{
	private:
		point3 m_point;
	public:
		/////////////////////////////////////////////////////////////////////////
		//	сконструировать команду
		set_origin2(const point3& pt) : m_point(pt) {}
		/////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////
		//	выполнить команду
		void execute(renderer* r) { r->set_origin2(r->get_rs()->to_screen(m_point)); }
		/////////////////////////////////////////////////////////////////////////
	};
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	////////////////////////    class put_rline2    /////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class put_rline2 : public renderer::command
	{
	private:
		point2 m_begin,m_end;
	public:
		/////////////////////////////////////////////////////////////////////////
		//	сконструировать команду
		put_rline2(const point2& begin,const point2& end) : m_begin(begin),m_end(end) {}
		/////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////
		//	выполнить команду
		void execute(renderer* r) { r->get_rs()->put_line(r->get_origin2()+(m_begin-origin),r->get_origin2()+(m_end-origin),r->get_color().value); }
		/////////////////////////////////////////////////////////////////////////
	};
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	////////////////////////    class set_color    //////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class set_color : public renderer::command
	{
	private:
		color m_sv;
	public:
		/////////////////////////////////////////////////////////////////////////
		//	сконструировать команду
		set_color(const color& setting_value) : m_sv(setting_value) {}
		/////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////
		//	выполнить команду
		void execute(renderer* r) { r->set_color(m_sv); }
		/////////////////////////////////////////////////////////////////////////
	};
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	////////////////////////    class set_meter    //////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class set_meter : public renderer::command
	{
	private:
		renderer::meter m_sv;
	public:
		/////////////////////////////////////////////////////////////////////////
		//	сконструировать команду
		set_meter(const renderer::meter& setting_value) : m_sv(setting_value) {}
		/////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////
		//	выполнить команду
		void execute(renderer* r) { r->set_meter(m_sv); }
		/////////////////////////////////////////////////////////////////////////
	};
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	////////////////////////    class set_text2    //////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class set_text2 : public renderer::command
	{
	private:
		std::string m_text;
		point2 m_pt;
	public:
		/////////////////////////////////////////////////////////////////////////
		//	сконструировать команду
		set_text2(const point2 pt,const std::string& value) : m_pt(pt),m_text(value) {}
		/////////////////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////////////////
		//	выполнить команду
		void execute(renderer* r) { r->get_rs()->put_text(m_pt,m_text,r->get_color().value); }
		/////////////////////////////////////////////////////////////////////////
	};
	/////////////////////////////////////////////////////////////////////////

}

/////////////////////////////////////////////////////////////////////////
//////////////////////////    class renderer    //////////////////////////
/////////////////////////////////////////////////////////////////////////
renderer::renderer(render_strategy* rs) : m_rs(rs),
										  m_meter(1),
										  m_origin2(origin),
										  m_origin3(origin),
										  m_color(0xffffffff)
{
}

renderer::~renderer()
{
	//	удаляем невыполненые команды
	for(commands_t::iterator it=m_commands.begin();it!=m_commands.end();++it)
		delete *it;
}

/////////////////////////////////////////////////////////////////////////
//	отобразить маркер
renderer& renderer::operator<<(const marker& m)
{
	m_commands.push_back(new commands::set_origin2(m.get_origin()));

	switch(m.get_type())
	{
	case marker::mt_point:
		{
			m_commands.push_back(new commands::put_rline2(point2(origin),origin+vector2(1,1)));
		}
		break;
	case marker::mt_x:
		{
			m_commands.push_back(new commands::put_rline2(origin-vector2(2,2),origin+vector2(3,3)));
			m_commands.push_back(new commands::put_rline2(origin-vector2(2,-2),origin+vector2(3,-3)));
		}
		break;
	case marker::mt_plus:
		{
			m_commands.push_back(new commands::put_rline2(origin-vector2(0,2),origin+vector2(0,3)));
			m_commands.push_back(new commands::put_rline2(origin-vector2(2,0),origin+vector2(3,0)));
		}
		break;
	};

	return *this;
}
//	вывести линию
renderer& renderer::operator<<(const line &l)
{
	m_commands.push_back(new commands::set_origin3(l.m_a));
	m_commands.push_back(new commands::put_rline3(origin,(l.m_b-l.m_a)-origin));
	return *this;
}
//	вывести двумерный текст
renderer& renderer::operator<<(const text2& t)
{
	m_commands.push_back(new commands::set_text2(t.m_pt,t.m_text));

	return *this;
}
//	отобразить frustum
renderer& renderer::operator<<(const frustum &r)
{
	m_commands.push_back(new commands::set_origin3(point3(0,0,0)));
	for(int i=0;i<4;i++)
	{
		m_commands.push_back(new commands::put_rline3(r[i],r[i+4]));
		m_commands.push_back(new commands::put_rline3(r[i],r[(i+1)%4]));
		m_commands.push_back(new commands::put_rline3(r[i+4],r[(i+1)%4+4]));
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////
//	отобразить aabb3
renderer& renderer::operator<<(const aabb3 &r)
{
	for(int i=0;i<4;i++)
	{
		m_commands.push_back(new commands::put_rline3(r[i],r[i+4]));
		m_commands.push_back(new commands::put_rline3(r[i],r[(i+1)%4]));
		m_commands.push_back(new commands::put_rline3(r[i+4],r[(i+1)%4+4]));
	}
	return *this;
}
/////////////////////////////////////////////////////////////////////////
//	отобразить луч
renderer& renderer::operator<<(const base_ray3<float>& r)
{
	vector3 auxiliary_axis = (vector3(r.direction).normalize()==vector3(1,0,0))?vector3(0,1,0):vector3(1,0,0);
	const point3 rend = r.direction-origin;

	vector3 ox(r.direction);
	vector3 oy(ox.cross(auxiliary_axis));
	vector3 oz(ox.cross(oy));

	ox.normalize();
	oy.normalize();
	oz.normalize();

	matrix3 rm(
		ox.x,oy.x,oz.x,0,
		ox.y,oy.y,oz.y,0,
		ox.z,oy.z,oz.z,0,
		rend.dot(-ox),rend.dot(-oy),rend.dot(-oz),1);

	m_commands.push_back(new commands::set_origin3(r.origin));
	m_commands.push_back(new commands::put_rline3(origin,rend));

	const int lines_num = 8;
	const float one = 0.05;
	const float one3m = one*3;
	point3 const_points[2*lines_num] =
	{
		point3(0,0,0), 
		point3(-one3m,-one,0),
		point3(0,0,0), 
		point3(-one3m,one,0),
		point3(0,0,0), 
		point3(-one3m,0,-one),
		point3(0,0,0), 
		point3(-one3m,0,one),

		point3(-one3m,-one,0),
		point3(-one3m,0,one),

		point3(-one3m,0,one),
		point3(-one3m,one,0),

		point3(-one3m,one,0),
		point3(-one3m,0,-one),

		point3(-one3m,0,-one),
		point3(-one3m,-one,0)
	};

	point3* points = const_points;

	rm.invert();

	for(int i=0;i<lines_num;i++)
	{
		*points = (*points)*rm; points++;
		*points = (*points)*rm; points++;
		m_commands.push_back(new commands::put_rline3(*(points-2),*(points-1)));
	}

	return *this;
}
/////////////////////////////////////////////////////////////////////////
//	установить цвет по умолчанию
renderer& renderer::operator<<(const color& c)
{
	m_commands.push_back(new commands::set_color(c));
	return *this;
}
/////////////////////////////////////////////////////////////////////////
//	установить значение метра по умолчанию
renderer& renderer::operator<<(const meter& m)
{
	m_commands.push_back(new commands::set_meter(m));
	return *this;
}
/////////////////////////////////////////////////////////////////////////
//	отобразить все
void renderer::flush(void)
{
	//	выполняем и удаляем команды
	for(commands_t::iterator it=m_commands.begin();it!=m_commands.end();++it)
	{
		(*it)->execute(this);
		delete *it;
	}

	m_commands.clear();
}
/////////////////////////////////////////////////////////////////////////
//	установить текущий цвет отображения
void renderer::set_color(const color& c)
{
	m_color = c;
}
/////////////////////////////////////////////////////////////////////////
//	установить текущее начало 2D координат
void renderer::set_origin2(const point2& origin)
{
	m_origin2 = origin;
}
/////////////////////////////////////////////////////////////////////////
//	установить текущее начало 3D координат
void renderer::set_origin3(const point3& origin)
{
	m_origin3 = origin;
}
/////////////////////////////////////////////////////////////////////////
//	установить текущее значение метра
void renderer::set_meter(const meter& m)
{
	m_meter = m;
}
/////////////////////////////////////////////////////////////////////////

//=====================================================================================//
//                                     class text2                                     //
//=====================================================================================//
text2::text2() : m_pt(0,0)
{
}

text2::text2(const char* text,...) : m_pt(0,0)
{
	va_list list;

	va_start(list,text);
	m_text = mll::io::vprintf(text,list);
	va_end(list);
}

text2::text2(int x,int y,const char* text,...) : m_pt(x,y)
{
	va_list list;

	va_start(list,text);
	m_text = mll::io::vprintf(text,list);
	va_end(list);
}

text2::text2(const mll::algebra::point2& pt,const char* text,...) : m_pt(pt)
{
	va_list list;

	va_start(list,text);
	m_text = mll::io::vprintf(text,list);
	va_end(list);
}
*/