#pragma once

#include "poly.h"
#include <mll/geometry/aabb3.h>
#include <mll/geometry/frustum.h>
#include <boost/shared_array.hpp>
#include <boost/utility.hpp>
#include <utility>

struct _ot_triangle;
class  vis_tris_observer;

///Треугольник октетного дерева.
/**
вынесен в глобальную область из-за того, что vc6 не понимает операторов [<<, >>] для вложенных
в класс структур.
*/
	struct _ot_triangle
	{
		unsigned v[3];		///< индексы вершин
		int		 floor;		///< номер этажа (используется при отрисовке)
		unsigned mat_id;	///< идентификатор материала
		unsigned rendered;	///< номер последнего прохода в которм данный треугольник рендерился
	};

/// класс октетного дерева
/*!
	По указанному списку треугольников строится дерево индексов.
	ссылка на треугольник добавляется в узел дерева только если
	он пересекается с обрамляющим объемом узла.
*/
class oct_tree
{
public:
	class node;
    class visitor;
    class const_visitor;
	struct hit_info;


	typedef _ot_triangle triangle;				///< тип треугольника
	typedef vert_storage::vert vert;			///< именно эти вершины будем использовать
	typedef vert_storage storage;				///< хранилище вершин для полигонов;
	typedef std::vector<triangle> PolyList;		///< Тип входного списка
	typedef std::vector<std::string> MatList;	///< Тип список материалов.
	typedef std::vector<triangle*> IdxPolyList; ///< список ссылок на треугольники
	mll::geometry::aabb3 m_aabb;				///< бокс для тени
	mll::geometry::aabb3 m_box;
	//mll::geometry::frustum m_frust;				///< фрастум для тени

	/// для более визуального указания кусков дерева
    /**
	 Это перечисление используется исключительно для того, чтобы к элементам узла можно было обратится
	 более визуально, например [s_near|s_left|s_top]
	*/
	enum sides{ 
	 s_top=0x0,     ///<нижняя часть по оси Z
	 s_bottom=0x4,  ///<верхняя часть по оси Z
	 s_left=0x2,    ///<кусок расположенный дальше по оси Y 
	  s_right=0,    ///<кусок расположенный Ближе по оси Y 
	  s_near=0x1,   ///<кусок расположенный дальше по оси X 
	  s_far=0       ///<кусок расположенный Ближе по оси X 
	  };
public:
///@name Конструирование.
///@{  
	oct_tree(void):m_floor_upto(65535),m_aabb(mll::algebra::point3(0.0f,0.0f,0.0f),mll::algebra::vector3f(1.0f,1.0f,1.0f))/*m_frust(0.1f,2.0f,3.14f/2.0f,1.0f)*/,m_pass(1) {};
	~oct_tree(void){};
///@}

	///указать до какого этажа видна геометрия
	void set_visible_floor(int upto);
	///создание дерева по списку треугольников.
	void construct(const PolyList &list, const storage &vstorage, const MatList& mats, float cell_size, int m_min_poly_count);
	///сохранение в поток
	friend mll::io::obinstream& operator <<(mll::io::obinstream &o, const oct_tree& tree);
	///восстановление из потока
	friend mll::io::ibinstream& operator >>(mll::io::ibinstream &i, oct_tree& tree);

public:
	typedef std::pair<int, int> floor_range_t;

	template<typename Fn> void traverse2(Fn fn, node *startnode=NULL);
	template<typename Fn> void traverse2(Fn fn, const node *startnode=NULL) const;

	void traverse(visitor *v, node *startnode=NULL);
	void traverse(const_visitor *v, const node *startnode=NULL) const;

	///трассировка луча.
	float intersect(const mll::geometry::ray3 &ray, floor_range_t floor_range=floor_range_t(-10000,10000), hit_info *hit = NULL, const node *root_node = NULL) const;
	///обновить видимость дерева по данному видимому объему камеры.
	void update(const mll::geometry::frustum &f, vis_tris_observer* obs=0 ,node *root_node = NULL);
	/// получить имена материалов.
	const MatList& get_materials() const {return m_materials;}
	/// получить затененные треугольники
	void getShadowedTriangles(const mll::geometry::aabb3 &aabb, std::vector<mll::algebra::point3> &vertices,int floor=-1);
	/// получить ссылки на затененные треугольники
	void getShadowedTriangles(std::vector<mll::algebra::point3> &vertices, node *root_node=NULL );
	/// вырезать треугольники по боксу
	void cut(const mll::geometry::obb3 &obb, std::vector<point3> &vertices, std::vector<unsigned short> &indices,node *root_node=NULL);
	/// узнать описанный бокс
	const mll::geometry::aabb3& getBBox();

	/// получить указатель на корень
	const node *get_root() const { return m_root.get(); }
	node *get_root() { return m_root.get(); }

	/// получить "сырой" список треугольников, присутствующих в дереве.
	std::vector<unsigned> get_indices() const;
	const storage& get_storage() const { return m_vertices;};
	const PolyList& get_triangles() const {return m_triangles;};

	unsigned			m_pass;			  ///< номер прохода, использкется для выделения уникальных треугольников тени и отбора треугольников на отображение

protected:
	PolyList         	m_triangles;	  ///< здесь хранятся треугольники
	storage				m_vertices;       ///< здесь хранятся вершины
	std::auto_ptr<node> m_root;			  ///< корень дерева
	MatList             m_materials;      ///< список имен материалов, используемых в дереве.
	int					m_floor_upto;	  ///< до этого этажа видна геометрия.

private:
	/// TEMP
	//std::vector<mll::geometry::aabb3> m_boxes;
	/// TEMP
	node *rec_contruct(const IdxPolyList &list, const mll::geometry::aabb3 &bound);
	void split(const IdxPolyList &list, const plane &p, IdxPolyList *pos, IdxPolyList *neg);

private:
	float m_cell_size;
	int   m_min_poly_count;


public:
///@name Отладочный сервис.
///@{  

	unsigned NodeCount(const node *start=NULL) const;  ///< количество узлов в дереве
	unsigned GetSize(const node *start=NULL) const;	   ///< примерный объем занимаемой памяти
	unsigned OverAllTriIdxs(const node *start=NULL) const;	 ///< общее количество индексов на треугольники в дереве. Обычно превышает количество треугольников дерева.
	unsigned PolyCount() const {return m_triangles.size();}	 ///< количество полигонов в дереве
	unsigned VertCount() const {return m_vertices.size();}	 ///< количество вершин в дереве
	unsigned MatCount() const {return m_materials.size();}	 ///< количество материалов в дереве

///@}
	struct hit_info
	{
		std::string mat;
		mll::algebra::point2 uv;
	};


};

//=====================================================================================//
//                              void oct_tree::traverse()                              //
//=====================================================================================//
template<typename Fn>
void oct_tree::traverse2(Fn fn, node *startnode)
{
	if(!startnode) startnode = m_root.get();

	if(fn(startnode))
	{
		for(int i=0;i<8;i++)
		{
			if(startnode->m_child[i].get()) traverse2(fn,startnode->m_child[i].get());
		}
	}
}

//=====================================================================================//
//                           void oct_tree::traverse() const                           //
//=====================================================================================//
template<typename Fn>
void oct_tree::traverse2(Fn fn, const node *startnode) const
{
	if(!startnode) startnode = m_root.get();

	if(fn(startnode))
	{
		for(int i=0;i<8;i++)
		{
			if(startnode->m_child[i].get()) traverse2(fn,startnode->m_child[i].get());
		}
	}
}

class vis_tris_observer
{
public:
	virtual void tris_visible ( const std::vector<oct_tree::triangle*> &tris, oct_tree::triangle* start_triangle, int floor_upto, unsigned pass )=0;
	virtual void end()=0;
};


///узел октетного дерева.
/**
	узел хранит в себе ссылки на все треугольники, которые пересекаются с его обрамляющим объемом.
	Там также хранятся ссылки на более мелкие узлы, если таковые имеются.
	при обходе дерева в узлах устанавливается атрибут видимости. потом с использованием этого атрибута
	создается список отрисовки.
*/
class oct_tree::node
{
public:
	friend class oct_tree;
	///Варианты видимости узла
	enum visibility 
	{
		all,   ///<виден весь узел и все его дети
		none,  ///<не видно полностью
		partly ///<узел частично виден (требуется рекурсивный анализ детей)
	};

	///получить доступ к дочерним элементам
	/**
	можно использовать oct_tree::sides.
	*/ 
	const node* operator[](int i) const{assert(i>=0 && i<8); return m_child[i].get();}
	///Конструктор
	node():m_has_children(false),m_visible(none)
	{
	}
public:
	///получить обрамляющий объем.
	const mll::geometry::aabb3& bound()const {return m_bound;};
	bool has_children() const{return m_has_children;};

///@name Хотел бы перенести в private область.
///@{  
public:
	oct_tree::IdxPolyList m_tris;		///< список ссылок на треугольники, попадающие в узел
	bool m_has_children;                ///< используется при различных обходах
	mll::geometry::aabb3 m_bound;		///< обрамляющий объем
	std::auto_ptr<node> m_child[8];		///< ссылки на дочерние узлы
	visibility m_visible;				///< метка видимости, используется при отрисовке
///@}

///@name Этап конструирования дерева.
///@{  
	
	///установить список треугольников, на которые ссылается узел.
	void set_trinagles(const oct_tree::IdxPolyList &list);	
///@}

};

///вспомогательный класс для обхода октетного дерева.
/**
пример использования - загрузка дерева из файла
*/
class oct_tree::visitor
{
public:
	virtual bool visit(oct_tree &tree, node *n)=0;  ///<вызывается при обходе дерева
													/**<
													\retval true если продолжить
													\retval false если прекратить обход
													*/

};

///вспомогательный класс для обхода октетного дерева.
/**
пример использования - запись дерева в файл
*/
class oct_tree::const_visitor
{
public:
	virtual bool visit(const oct_tree &tree, const node *n)=0;    ///<вызывается при обходе дерева
															/**<
															\retval true если продолжить
															\retval false если прекратить обход
															*/

};
