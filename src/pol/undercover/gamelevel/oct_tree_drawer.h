#pragma once

#include "oct_tree_idx.h"
#include <atlcomcli.h>

//#include <shaderfxshaderstate.h>
//#include <VisBufferMgr.h>
//#include <VisVertexRange.h>
//#include <VisIndexRange.h>
//#include <boost/shared_ptr.hpp>

class VShader;

//=====================================================================================//
//                   class oct_tree_drawer: public vis_tris_observer                   //
//=====================================================================================//
class oct_tree_drawer: public vis_tris_observer
{
	std::vector<unsigned short> m_indices;
	unsigned short *m_ibptr;

	struct Material
	{
		std::string m_name;
		unsigned int m_traits;
	};

public:
	enum MaterialTraits
	{
		mtSolid			= 1<<0,
		mtShadow		= 1<<1,
		mtPick			= 1<<2,
		mtOpaque		= 1<<3,
		mtDynLight		= 1<<4,
		mtTwoSided		= 1<<5,
		mtForceDword	= 0x7FFFFFFF
	};

	typedef std::vector<oct_tree::triangle*> Triangles_t;

	void reset();
	void init(const vert_storage &s, const oct_tree::PolyList &tris, const std::vector<std::string> &mat_names);
	void resetVisible();
	void end();
	void draw(unsigned int andfl = 0, unsigned int notfl = 0);
	void tris_visible(const Triangles_t &m_all_tris, oct_tree::triangle* start_triangle, int floor_upto, unsigned int pass);
	void updateMaterials();

	unsigned int matNumber() const
	{
		return m_materials.size();
	}

private:
	CComPtr<IDirect3DVertexBuffer7> createVertexBuffer(unsigned int size);
	static unsigned int makeShaderTraits(VShader *s);

	struct vertex
	{
		mll::algebra::point3 m_pos;
		mll::algebra::vector3 m_normal;
		mll::algebra::point2 m_uv;
		mll::algebra::point2 m_uv2;
	};

	struct vb_ptr
	{
		CComPtr<IDirect3DVertexBuffer7> m_vb;
		unsigned int m_start;
		vb_ptr() {}
		explicit vb_ptr(unsigned int s) : m_start(s) {}
	};

	struct ib_ptr
	{
		unsigned short *m_start;
		ib_ptr() {}
		explicit ib_ptr(unsigned short *s) : m_start(s) {}
	};

	typedef std::vector<unsigned short> indices_t;
	struct local_tri
	{
		int mat_id;
		unsigned short a,b,c;
	};
	struct hw_info
	{
		unsigned int trs;   // кол-во примитивов для рендера
		unsigned int verts;
		vb_ptr   vb;
		ib_ptr   ib;
	};

	std::vector<hw_info> m_buffers;

	std::vector<local_tri>		m_tris;
	std::vector<Material>		m_materials;
	std::vector<unsigned int>	m_sortedorder;
	std::vector<VShader*>		m_shaders;
};
