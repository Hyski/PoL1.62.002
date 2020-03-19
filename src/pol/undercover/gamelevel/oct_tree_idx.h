#pragma once

#include "poly.h"
#include <mll/geometry/aabb3.h>
#include <mll/geometry/frustum.h>
#include <boost/shared_array.hpp>
#include <boost/utility.hpp>
#include <utility>

struct _ot_triangle;
class  vis_tris_observer;

///����������� ��������� ������.
/**
������� � ���������� ������� ��-�� ����, ��� vc6 �� �������� ���������� [<<, >>] ��� ���������
� ����� ��������.
*/
	struct _ot_triangle
	{
		unsigned v[3];		///< ������� ������
		int		 floor;		///< ����� ����� (������������ ��� ���������)
		unsigned mat_id;	///< ������������� ���������
		unsigned rendered;	///< ����� ���������� ������� � ������ ������ ����������� ����������
	};

/// ����� ��������� ������
/*!
	�� ���������� ������ ������������� �������� ������ ��������.
	������ �� ����������� ����������� � ���� ������ ������ ����
	�� ������������ � ����������� ������� ����.
*/
class oct_tree
{
public:
	class node;
    class visitor;
    class const_visitor;
	struct hit_info;


	typedef _ot_triangle triangle;				///< ��� ������������
	typedef vert_storage::vert vert;			///< ������ ��� ������� ����� ������������
	typedef vert_storage storage;				///< ��������� ������ ��� ���������;
	typedef std::vector<triangle> PolyList;		///< ��� �������� ������
	typedef std::vector<std::string> MatList;	///< ��� ������ ����������.
	typedef std::vector<triangle*> IdxPolyList; ///< ������ ������ �� ������������
	mll::geometry::aabb3 m_aabb;				///< ���� ��� ����
	mll::geometry::aabb3 m_box;
	//mll::geometry::frustum m_frust;				///< ������� ��� ����

	/// ��� ����� ����������� �������� ������ ������
    /**
	 ��� ������������ ������������ ������������� ��� ����, ����� � ��������� ���� ����� ���� ���������
	 ����� ���������, �������� [s_near|s_left|s_top]
	*/
	enum sides{ 
	 s_top=0x0,     ///<������ ����� �� ��� Z
	 s_bottom=0x4,  ///<������� ����� �� ��� Z
	 s_left=0x2,    ///<����� ������������� ������ �� ��� Y 
	  s_right=0,    ///<����� ������������� ����� �� ��� Y 
	  s_near=0x1,   ///<����� ������������� ������ �� ��� X 
	  s_far=0       ///<����� ������������� ����� �� ��� X 
	  };
public:
///@name ���������������.
///@{  
	oct_tree(void):m_floor_upto(65535),m_aabb(mll::algebra::point3(0.0f,0.0f,0.0f),mll::algebra::vector3f(1.0f,1.0f,1.0f))/*m_frust(0.1f,2.0f,3.14f/2.0f,1.0f)*/,m_pass(1) {};
	~oct_tree(void){};
///@}

	///������� �� ������ ����� ����� ���������
	void set_visible_floor(int upto);
	///�������� ������ �� ������ �������������.
	void construct(const PolyList &list, const storage &vstorage, const MatList& mats, float cell_size, int m_min_poly_count);
	///���������� � �����
	friend mll::io::obinstream& operator <<(mll::io::obinstream &o, const oct_tree& tree);
	///�������������� �� ������
	friend mll::io::ibinstream& operator >>(mll::io::ibinstream &i, oct_tree& tree);

public:
	typedef std::pair<int, int> floor_range_t;

	template<typename Fn> void traverse2(Fn fn, node *startnode=NULL);
	template<typename Fn> void traverse2(Fn fn, const node *startnode=NULL) const;

	void traverse(visitor *v, node *startnode=NULL);
	void traverse(const_visitor *v, const node *startnode=NULL) const;

	///����������� ����.
	float intersect(const mll::geometry::ray3 &ray, floor_range_t floor_range=floor_range_t(-10000,10000), hit_info *hit = NULL, const node *root_node = NULL) const;
	///�������� ��������� ������ �� ������� �������� ������ ������.
	void update(const mll::geometry::frustum &f, vis_tris_observer* obs=0 ,node *root_node = NULL);
	/// �������� ����� ����������.
	const MatList& get_materials() const {return m_materials;}
	/// �������� ���������� ������������
	void getShadowedTriangles(const mll::geometry::aabb3 &aabb, std::vector<mll::algebra::point3> &vertices,int floor=-1);
	/// �������� ������ �� ���������� ������������
	void getShadowedTriangles(std::vector<mll::algebra::point3> &vertices, node *root_node=NULL );
	/// �������� ������������ �� �����
	void cut(const mll::geometry::obb3 &obb, std::vector<point3> &vertices, std::vector<unsigned short> &indices,node *root_node=NULL);
	/// ������ ��������� ����
	const mll::geometry::aabb3& getBBox();

	/// �������� ��������� �� ������
	const node *get_root() const { return m_root.get(); }
	node *get_root() { return m_root.get(); }

	/// �������� "�����" ������ �������������, �������������� � ������.
	std::vector<unsigned> get_indices() const;
	const storage& get_storage() const { return m_vertices;};
	const PolyList& get_triangles() const {return m_triangles;};

	unsigned			m_pass;			  ///< ����� �������, ������������ ��� ��������� ���������� ������������� ���� � ������ ������������� �� �����������

protected:
	PolyList         	m_triangles;	  ///< ����� �������� ������������
	storage				m_vertices;       ///< ����� �������� �������
	std::auto_ptr<node> m_root;			  ///< ������ ������
	MatList             m_materials;      ///< ������ ���� ����������, ������������ � ������.
	int					m_floor_upto;	  ///< �� ����� ����� ����� ���������.

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
///@name ���������� ������.
///@{  

	unsigned NodeCount(const node *start=NULL) const;  ///< ���������� ����� � ������
	unsigned GetSize(const node *start=NULL) const;	   ///< ��������� ����� ���������� ������
	unsigned OverAllTriIdxs(const node *start=NULL) const;	 ///< ����� ���������� �������� �� ������������ � ������. ������ ��������� ���������� ������������� ������.
	unsigned PolyCount() const {return m_triangles.size();}	 ///< ���������� ��������� � ������
	unsigned VertCount() const {return m_vertices.size();}	 ///< ���������� ������ � ������
	unsigned MatCount() const {return m_materials.size();}	 ///< ���������� ���������� � ������

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


///���� ��������� ������.
/**
	���� ������ � ���� ������ �� ��� ������������, ������� ������������ � ��� ����������� �������.
	��� ����� �������� ������ �� ����� ������ ����, ���� ������� �������.
	��� ������ ������ � ����� ��������������� ������� ���������. ����� � �������������� ����� ��������
	��������� ������ ���������.
*/
class oct_tree::node
{
public:
	friend class oct_tree;
	///�������� ��������� ����
	enum visibility 
	{
		all,   ///<����� ���� ���� � ��� ��� ����
		none,  ///<�� ����� ���������
		partly ///<���� �������� ����� (��������� ����������� ������ �����)
	};

	///�������� ������ � �������� ���������
	/**
	����� ������������ oct_tree::sides.
	*/ 
	const node* operator[](int i) const{assert(i>=0 && i<8); return m_child[i].get();}
	///�����������
	node():m_has_children(false),m_visible(none)
	{
	}
public:
	///�������� ����������� �����.
	const mll::geometry::aabb3& bound()const {return m_bound;};
	bool has_children() const{return m_has_children;};

///@name ����� �� ��������� � private �������.
///@{  
public:
	oct_tree::IdxPolyList m_tris;		///< ������ ������ �� ������������, ���������� � ����
	bool m_has_children;                ///< ������������ ��� ��������� �������
	mll::geometry::aabb3 m_bound;		///< ����������� �����
	std::auto_ptr<node> m_child[8];		///< ������ �� �������� ����
	visibility m_visible;				///< ����� ���������, ������������ ��� ���������
///@}

///@name ���� ��������������� ������.
///@{  
	
	///���������� ������ �������������, �� ������� ��������� ����.
	void set_trinagles(const oct_tree::IdxPolyList &list);	
///@}

};

///��������������� ����� ��� ������ ��������� ������.
/**
������ ������������� - �������� ������ �� �����
*/
class oct_tree::visitor
{
public:
	virtual bool visit(oct_tree &tree, node *n)=0;  ///<���������� ��� ������ ������
													/**<
													\retval true ���� ����������
													\retval false ���� ���������� �����
													*/

};

///��������������� ����� ��� ������ ��������� ������.
/**
������ ������������� - ������ ������ � ����
*/
class oct_tree::const_visitor
{
public:
	virtual bool visit(const oct_tree &tree, const node *n)=0;    ///<���������� ��� ������ ������
															/**<
															\retval true ���� ����������
															\retval false ���� ���������� �����
															*/

};
