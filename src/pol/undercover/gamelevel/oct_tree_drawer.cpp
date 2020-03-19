#include "precomp.h"

#include "oct_tree_drawer.h"
#include "poly.h"

#include <Common/GraphPipe/GraphPipe.h>
#include "../IWorld.h"

static const unsigned int VertexFVF = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1);

namespace OctTreeDrawerDetail
{

	//=====================================================================================//
	//                                   struct AdjPred                                    //
	//=====================================================================================//
	template<typename T>
	struct AdjPred
	{
		unsigned int m_delta;
		const T &m_buffer;

		AdjPred(const T &b, unsigned int delta) : m_buffer(b), m_delta(delta) {}

		template<typename Val>
			bool operator()(const Val &l) const
		{
			return ((m_buffer[l.first].vb.m_start-m_delta) / D3DMAXNUMVERTICES)
				!= ((m_buffer[l.first].vb.m_start+m_buffer[l.first].verts-m_delta) / D3DMAXNUMVERTICES);
		}
	};

	//=====================================================================================//
	//                              AdjPred<T> makeAdjPred()                               //
	//=====================================================================================//
	template<typename T>
		AdjPred<T> makeAdjPred(const T &b, unsigned int d)
	{
		return AdjPred<T>(b,d);
	}

	//=====================================================================================//
	//                                    class VbInfo                                     //
	//=====================================================================================//
	class VbInfo
	{
		unsigned int m_dataEnd;
		static const unsigned int MaxVertexCount = D3DMAXNUMVERTICES;
		CComPtr<IDirect3DVertexBuffer7> m_vbuf;

	public:
		VbInfo() : m_dataEnd(0) {}

		bool canInsert(unsigned int amount) const
		{
			return amount < MaxVertexCount - m_dataEnd;
		}

		unsigned int insert(unsigned int amount)
		{
			assert( canInsert(amount) );
			unsigned int result = m_dataEnd;
			m_dataEnd += amount;
			return result;
		}

		void clearAndCreateBuffer()
		{
			D3DVERTEXBUFFERDESC aDesc;
			ZeroMemory(&aDesc,sizeof(aDesc));
			aDesc.dwSize = sizeof(aDesc);
			aDesc.dwCaps = 0;//D3DVBCAPS_WRITEONLY;// | D3DVBCAPS_SYSTEMMEMORY;
			//if(_OldRender) aDesc.dwCaps |= D3DVBCAPS_SYSTEMMEMORY;
			aDesc.dwFVF = VertexFVF;
			aDesc.dwNumVertices = m_dataEnd;

			HRESULT hres = D3DKernel::GetD3D()->CreateVertexBuffer(&aDesc,&m_vbuf,0);
			assert( SUCCEEDED(hres) );

			m_dataEnd = 0;
		}

		CComPtr<IDirect3DVertexBuffer7> getVertexBuffer() const
		{
			return m_vbuf;
		}
	};

	//=====================================================================================//
	//                                struct CanInsertInVb                                 //
	//=====================================================================================//
	struct CanInsertInVb
	{
		unsigned int m_amount;
		CanInsertInVb(unsigned int a) : m_amount(a) {}

		bool operator()(const VbInfo &vi) const
		{
			return vi.canInsert(m_amount);
		}
	};
}

using namespace OctTreeDrawerDetail;

//=====================================================================================//
//                            void oct_tree_drawer::reset()                            //
//=====================================================================================//
void oct_tree_drawer::reset()
{
	m_materials.clear();
	m_indices.clear();
	m_ibptr = 0;
	m_buffers.clear();
	m_tris.clear();
	m_sortedorder.clear();
	m_shaders.clear();
}

//=====================================================================================//
//                            void oct_tree_drawer::init()                             //
//=====================================================================================//
void oct_tree_drawer::init(const vert_storage &s, const oct_tree::PolyList &tris,
						   const std::vector<std::string> &mat_names)
{
	m_materials.resize(mat_names.size());

	for(int i = 0; i != m_materials.size(); ++i)
	{
		m_materials[i].m_name = mat_names[i];
	}

	//
	typedef std::map<unsigned int, vert_storage> Vertices_t;
	typedef std::map<unsigned int, int> TriNums_t;

	Vertices_t vertices;
	TriNums_t trinum; // кол-во треугольников по материалам

	//
	// заполняем m_tris
	// там лежат треугольники с локальными индексами по материалам
	for(unsigned int i=0; i<tris.size(); ++i)
	{
		unsigned int id = tris[i].mat_id;
		local_tri new_tri;
		//
		new_tri.mat_id = id;
		new_tri.a = vertices[id].add(s[tris[i].v[0]]);
		new_tri.b = vertices[id].add(s[tris[i].v[1]]);
		new_tri.c = vertices[id].add(s[tris[i].v[2]]);
		//
		m_tris.push_back(new_tri);
		++trinum.insert(std::make_pair(id,0)).first->second;
	}

	m_buffers.resize(m_materials.size());

	typedef std::vector<VbInfo> Vbufs_t;
	Vbufs_t vbufs;

	{
		unsigned int vcount = 0;
		Vertices_t::iterator it = vertices.begin();
		for(; it != vertices.end(); ++it)
		{
			unsigned int id = it->first;
			m_buffers[id].vb.m_start = vcount;
			m_buffers[id].verts = it->second.size();
			vcount += it->second.size();

		}

		it = vertices.begin();

		for(Vertices_t::iterator ii = vertices.begin(); ii != vertices.end(); ++ii)
		{
			Vbufs_t::iterator jj = std::find_if(vbufs.begin(),vbufs.end(),CanInsertInVb(m_buffers[ii->first].verts));
			if(jj == vbufs.end())
			{
				vbufs.push_back(VbInfo());
				jj = vbufs.begin() + vbufs.size() - 1;
			}

			m_buffers[ii->first].vb.m_start = jj->insert(m_buffers[ii->first].verts);
		}

		for(Vbufs_t::iterator vi = vbufs.begin(); vi != vbufs.end(); ++vi)
		{
			vi->clearAndCreateBuffer();
		}

		for(Vertices_t::iterator ii = vertices.begin(); ii != vertices.end(); ++ii)
		{
			Vbufs_t::iterator jj = std::find_if(vbufs.begin(),vbufs.end(),CanInsertInVb(m_buffers[ii->first].verts));
			assert( jj != vbufs.end() );
			jj->insert(m_buffers[ii->first].verts);

			m_buffers[ii->first].vb.m_vb = jj->getVertexBuffer();
		}
	}

	//
	HRESULT hres;

	unsigned int icount = 0;
	for(Vertices_t::iterator it=vertices.begin(); it!=vertices.end(); ++it)
	{
		icount += trinum[it->first]*3;
	}

	m_indices.resize(icount);

	vertex *data = 0;
	unsigned int vstart = 0;
	unsigned int istart = 0;
	CComPtr<IDirect3DVertexBuffer7> curbuf;

	for(Vertices_t::iterator it = vertices.begin(); it != vertices.end(); ++it)
	{
		unsigned int id = it->first;

		if(curbuf != m_buffers[id].vb.m_vb)
		{
			if(curbuf)
			{
				hres = curbuf->Unlock();
				assert( SUCCEEDED(hres) );
			}

			curbuf = m_buffers[id].vb.m_vb;

			DWORD lockedSize = 0;
			hres = curbuf->Lock(DDLOCK_NOSYSLOCK|DDLOCK_WRITEONLY|DDLOCK_DISCARDCONTENTS,(void **)&data,&lockedSize);
			assert( SUCCEEDED(hres) );
		}

		// заполняем VB точками по материалам
		for(int i = 0; i < it->second.size(); ++i)
		{
			vertex &v = *(data + m_buffers[id].vb.m_start + i);
			//
			v.m_pos	= it->second[i].pos;
			v.m_normal = mll::algebra::vector3(it->second[i].norm).normalize();
			v.m_uv	= it->second[i].uv;
			v.m_uv2	= it->second[i].lm_uv;
			//
		}

		//unsigned int temp = trinum[id]*3;
		//if(trinum[id]*3 > 65536) assert(!"65536 Indexes!");
		m_buffers[id].ib = ib_ptr(&m_indices[istart]);
		//

		m_buffers[id].trs = 0;

		//vstart += it->second.size();
		istart += trinum[id]*3;
	}

	if(curbuf)
	{
		hres = curbuf->Unlock();
		assert( SUCCEEDED(hres) );
		curbuf.Release();
	}
}

//=====================================================================================//
//                     void oct_tree_drawer::createVertexBuffer()                      //
//=====================================================================================//
CComPtr<IDirect3DVertexBuffer7> oct_tree_drawer::createVertexBuffer(unsigned int vcount)
{
	CComPtr<IDirect3DVertexBuffer7> result;

	D3DVERTEXBUFFERDESC aDesc;
	ZeroMemory(&aDesc,sizeof(aDesc));
	aDesc.dwSize = sizeof(aDesc);
	aDesc.dwCaps = D3DVBCAPS_WRITEONLY /*| D3DVBCAPS_SYSTEMMEMORY*/;
	//if(_OldRender) aDesc.dwCaps |= D3DVBCAPS_SYSTEMMEMORY;
	aDesc.dwFVF = VertexFVF;
	aDesc.dwNumVertices = vcount;

	HRESULT hres = D3DKernel::GetD3D()->CreateVertexBuffer(&aDesc,&result,0);
	assert( SUCCEEDED(hres) );

	return result;
}

//=====================================================================================//
//         oct_tree_drawer::MaterialTraits oct_tree_drawer::makeShaderTraits()         //
//=====================================================================================//
unsigned int oct_tree_drawer::makeShaderTraits(VShader *shd)
{
	unsigned int result = mtDynLight | mtPick | mtShadow | mtTwoSided | mtOpaque;

	const shader_struct *Shd = &shd->GetTraits();
	unsigned int flags = Shd->flags, sparam = Shd->surfaceParam;

	if(sparam & SHD_NODLIGHT)
	{
		result &= ~mtDynLight;
	}

	if(sparam & SHD_NONSOLID)
	{
		result &= ~mtPick;
	}

	if(sparam & SHD_TRANS)
	{
		result &= ~mtShadow;
	}

	if(flags & SHD_CULL_BACK)
	{
		result &= ~mtTwoSided;
	}

	if(shd->Transparent)
	{
		result &= ~mtOpaque;
	}

	return result;
}

//=====================================================================================//
//                       void oct_tree_drawer::updateMaterials()                       //
//=====================================================================================//
void oct_tree_drawer::updateMaterials()
{
	typedef std::map<std::string, unsigned int> SortedMats_t;
	SortedMats_t sorted_mats;

	for(unsigned int i=0; i < m_materials.size(); ++i)
	{
		std::string shd;

		GraphPipe *pipe = IWorld::Get()->GetPipe();
		VShader *shader = pipe->FindShader(m_materials[i].m_name);

		m_shaders.push_back(shader);
		m_materials[i].m_traits = makeShaderTraits(shader);

		sorted_mats[shd+m_materials[i].m_name] = i;
	}

	std::transform(sorted_mats.begin(),sorted_mats.end(),std::back_inserter(m_sortedorder),std::select2nd<SortedMats_t::value_type>());
}

//=====================================================================================//
//                        void oct_tree_drawer::resetVisible()                         //
//=====================================================================================//
/// подготовка к заполнению IB
void oct_tree_drawer::resetVisible()
{
	for(unsigned int i = 0; i < m_materials.size(); ++i)
	{
		m_buffers[i].trs = 0;
	}
}

//=====================================================================================//
//                        void oct_tree_drawer::tris_visible()                         //
//=====================================================================================//
/// добавление видимых треугольников
void oct_tree_drawer::tris_visible(const Triangles_t &tris,oct_tree::triangle* start_triangle,
								   int floor_upto, unsigned int pass )
{
	for ( std::vector<oct_tree::triangle*>::const_iterator i = tris.begin(); i != tris.end(); ++i )
	{
		if ( (*i)->rendered != pass )
		{
			(*i)->rendered = pass;
			if( floor_upto<(*i)->floor && floor_upto!=-1) continue;
			const unsigned int id = (*i)->mat_id;
			const unsigned int num = (*i) - start_triangle;

			unsigned int index = m_buffers[id].trs;
			hw_info &hwi = m_buffers[id];

			hwi.ib.m_start[index*3 + 0] = m_tris[num].a;
			hwi.ib.m_start[index*3 + 1] = m_tris[num].b;
			hwi.ib.m_start[index*3 + 2] = m_tris[num].c;
			++hwi.trs;
		}
	}
}

//=====================================================================================//
//                             void oct_tree_drawer::end()                             //
//=====================================================================================//
/// завершение модификации IB
void oct_tree_drawer::end()
{
}

//=====================================================================================//
//                            void oct_tree_drawer::draw()                             //
//=====================================================================================//
/// отрисовка 
void oct_tree_drawer::draw(unsigned int andfl, unsigned int notfl)
{
	if(m_sortedorder.empty()) return;

	const unsigned int compoundfl = andfl|notfl;
	IWorld::Get()->GetPipe()->SwitchRenderMode(GraphPipe::RM_3D);

	for(unsigned int i = 0; i < m_materials.size(); ++i)
	{
		unsigned int n = m_sortedorder[i];
		if(m_buffers[n].trs)
		{
			unsigned int traits = m_materials[n].m_traits;
			if((traits&compoundfl) == andfl)
			{
				VShader *shader = m_shaders[n];
				shader->Chop(
					D3DPT_TRIANGLELIST,
					m_buffers[n].vb.m_vb,
					m_buffers[n].vb.m_start,
					m_buffers[n].verts,
					m_buffers[n].ib.m_start,
					m_buffers[n].trs*3
					);
			}
		}
	}
}

