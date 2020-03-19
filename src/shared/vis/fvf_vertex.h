#if !defined(__FVF_VERTEX_H_INCLUDED_7552370787048739__)
#define __FVF_VERTEX_H_INCLUDED_7552370787048739__

#include <d3d9.h>
#include <d3dx9core.h>
#include <mll/algebra/point3.h>
#include <mll/algebra/vector3.h>
#include <mll/algebra/point2.h>

#pragma pack(push,1)

/**
\class fvf_vertex
Возможные поля:
 - point3 m_pos
 - float m_blend[n]
 - float m_rhw
 - unsigned char m_idx[4]
 - unsigned int m_diffuse { unsigned char m_diffuse_r, m_diffuse_g, m_diffuse_b, m_diffuse_a ;}
 - vector3 m_normal
 - m_specular   { unsigned char m_specular_r, m_specular_g, m_specular_b, m_specular_a ;}
 - point2 m_uvN 
 - point3 m_uvwN
 - float m_uN
 - float m_uvsN[4]
 - float m_psize
*/
namespace detail
{

	using mll::algebra::point3;
	using mll::algebra::point2;
	using mll::algebra::vector3;

template<unsigned FVF> struct _fvf_position;
template<unsigned FVF> struct _fvf_normal;
template<unsigned FVF> struct _fvf_psize;
template<unsigned FVF> struct _fvf_diffuse;
template<unsigned FVF> struct _fvf_specular;
template<unsigned FVF> struct _fvf_texcoord1;
template<unsigned FVF> struct _fvf_texcoord2;
template<unsigned FVF> struct _fvf_texcoord3;
template<unsigned FVF> struct _fvf_texcoord4;
template<unsigned FVF> struct _fvf_texcoord5;
template<unsigned FVF> struct _fvf_texcoord6;
template<unsigned FVF> struct _fvf_texcoord7;
template<unsigned FVF> struct _fvf_texcoord8;

//=====================================================================================//
//                                struct _fvf_position                                 //
//=====================================================================================//
template<unsigned FVF>
struct _fvf_position
{
//	typedef typename _fvf_normal<FVF>::data base_class;
	template<unsigned Pos, bool LastBeta> struct _data_helper;
	typedef _data_helper<FVF&D3DFVF_POSITION_MASK,((FVF&D3DFVF_LASTBETA_UBYTE4) != 0) > data;

	template<> struct _data_helper<0,false>
	{
		static const bool has_pos = false;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 0;
		static const bool has_blend_indices = false;
	};

	template<> struct _data_helper<D3DFVF_XYZ,false>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 0;
		static const bool has_blend_indices = false;
		point3 m_pos;
	};

	template<> struct _data_helper<D3DFVF_XYZRHW,false>
	{
		static const bool has_pos = true;
		static const bool has_rhw = true;
		static const unsigned int blend_weight_count = 0;
		static const bool has_blend_indices = false;
		point3 m_pos;
		float m_rhw;
	};

	template<> struct _data_helper<D3DFVF_XYZB1,false>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 1;
		static const bool has_blend_indices = false;
		point3 m_pos;
		float m_blend[1];
	};

	template<> struct _data_helper<D3DFVF_XYZB2,false>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 2;
		static const bool has_blend_indices = false;
		point3 m_pos;
		float m_blend[2];
	};

	template<> struct _data_helper<D3DFVF_XYZB3,false>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 3;
		static const bool has_blend_indices = false;
		point3 m_pos;
		float m_blend[3];
	};

	template<> struct _data_helper<D3DFVF_XYZB4,false>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 4;
		static const bool has_blend_indices = false;
		point3 m_pos;
		float m_blend[4];
	};

	//template<> struct _data_helper<D3DFVF_XYZB1,true>
	//{
	//	point3 m_pos;
	//	DWORD m_indices;
	//};

	template<> struct _data_helper<D3DFVF_XYZB2,true>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 1;
		static const bool has_blend_indices = true;
		point3 m_pos;
		float m_blend[1];
		unsigned char m_idx[4];
	};

	template<> struct _data_helper<D3DFVF_XYZB3,true>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 2;
		static const bool has_blend_indices = true;
		point3 m_pos;
		float m_blend[2];
		unsigned char m_idx[4];
	};

	template<> struct _data_helper<D3DFVF_XYZB4,true>
	{
		static const bool has_pos = true;
		static const bool has_rhw = false;
		static const unsigned int blend_weight_count = 3;
		static const bool has_blend_indices = true;
		point3 m_pos;
		float m_blend[3];
		unsigned char m_idx[4];
	};
};

//=====================================================================================//
//                                 struct _fvf_normal                                  //
//=====================================================================================//
template<unsigned FVF>
struct _fvf_normal
{
	typedef typename _fvf_position<FVF>::data base_class;
	template<bool Pos> struct _data_helper;
	typedef _data_helper<(FVF&D3DFVF_NORMAL)!=0> data;

	template<> struct _data_helper<true> : base_class
	{
		static const bool has_normal = true;
		vector3 m_normal;
	};

	template<> struct _data_helper<false> : base_class
	{
		static const bool has_normal = false;
	};
};

//=====================================================================================//
//                                  struct _fvf_psize                                  //
//=====================================================================================//
template<unsigned FVF>
struct _fvf_psize
{
	typedef typename _fvf_normal<FVF>::data base_class;
	template<bool Pos> struct _data_helper;
	typedef _data_helper<(FVF&D3DFVF_PSIZE)!=0> data;

	template<> struct _data_helper<true> : base_class
	{
		static const bool has_psize = true;
		float m_psize;
	};

	template<> struct _data_helper<false> : base_class
	{
		static const bool has_psize = false;
	};
};

//=====================================================================================//
//                                 struct _fvf_diffuse                                 //
//=====================================================================================//
template<unsigned FVF>
struct _fvf_diffuse
{
	typedef typename _fvf_psize<FVF>::data base_class;
	template<bool Pos> struct _data_helper;
	typedef _data_helper<(FVF&D3DFVF_DIFFUSE)!=0> data;

	template<> struct _data_helper<true> : base_class
	{
		static const bool has_diffuse = true;
		union
		{
			unsigned m_diffuse;
			struct
			{
				unsigned char m_diffuse_b;
				unsigned char m_diffuse_g;
				unsigned char m_diffuse_r;
				unsigned char m_diffuse_a;
			};
		};
	};
	template<> struct _data_helper<false> : base_class
	{
		static const bool has_diffuse = false;
	};
};

//=====================================================================================//
//                                struct _fvf_specular                                 //
//=====================================================================================//
template<unsigned FVF>
struct _fvf_specular
{
	typedef typename _fvf_diffuse<FVF>::data base_class;
	template<bool Pos> struct _data_helper;
	typedef _data_helper<(FVF&D3DFVF_SPECULAR)!=0> data;

	template<> struct _data_helper<true> : base_class
	{
		static const bool has_specular = true;
		union
		{
			unsigned m_specular;
			struct
			{
				unsigned char m_specular_b;
				unsigned char m_specular_g;
				unsigned char m_specular_r;
				unsigned char m_specular_a;
			};
		};
	};
	template<> struct _data_helper<false> : base_class
	{
		static const bool has_specular = false;
	};
};

//=====================================================================================//
//                                 struct _fvf_switch                                  //
//=====================================================================================//
template<bool C1, bool C2, bool C3, bool C4> struct _fvf_switch;
template<> struct _fvf_switch<true,false,false,false> { enum { value = 1 }; };
template<> struct _fvf_switch<false,true,false,false> { enum { value = 2 }; };
template<> struct _fvf_switch<false,false,true,false> { enum { value = 3 }; };
template<> struct _fvf_switch<false,false,false,true> { enum { value = 4 }; };

//=====================================================================================//
//                           struct _fvf_get_tex_coord_count                           //
//=====================================================================================//
template<unsigned FVF, unsigned CoordN>
struct _fvf_get_tex_coord_size
{
	enum { t = (FVF>>(16 + CoordN*2))&0x3 };
	enum { value = _fvf_switch
					<
						t==D3DFVF_TEXTUREFORMAT1,
						t==D3DFVF_TEXTUREFORMAT2,
						t==D3DFVF_TEXTUREFORMAT3,
						t==D3DFVF_TEXTUREFORMAT4
					>::value };
};

//=====================================================================================//
//                           struct _fvf_get_tex_coord_count                           //
//=====================================================================================//
template<unsigned FVF>
struct _fvf_get_tex_coord_count
{
	enum { value = (FVF&D3DFVF_TEXCOUNT_MASK)>>D3DFVF_TEXCOUNT_SHIFT };
};

//=====================================================================================//
//                               struct _if_else_helper                                //
//=====================================================================================//
template<unsigned A, unsigned B>
struct _if_else_helper
{
	template<bool C> struct binder;
	template<> struct binder<true> { enum { value = A }; };
	template<> struct binder<false> { enum { value = B }; };
};

//=====================================================================================//
//                                   struct _if_else                                   //
//=====================================================================================//
template<bool C, unsigned A, unsigned B>
struct _if_else
{
	enum { value = _if_else_helper<A,B>::template binder<C>::value };
};

template<unsigned int A, unsigned int B>
struct _less
{
	enum { value = A < B };
};

//=====================================================================================//
//                                struct _fvf_texcoord1                                //
//=====================================================================================//
template<unsigned FVF>
struct _fvf_texcoord1
{
	typedef typename _fvf_specular<FVF>::data base_class;
	template<unsigned int count> struct _data_helper;
	enum { _tcoord_count = _if_else
							<
								_less<_fvf_get_tex_coord_count<FVF>::value,1>::value,
								0,
								_fvf_get_tex_coord_size<FVF,0>::value
							>::value };

	template<> struct _data_helper<0> : base_class
	{
		static const unsigned int texcoord_count = _fvf_get_tex_coord_count<FVF>::value;
		static const unsigned int texcoord1_size = 0;
	};

	template<> struct _data_helper<1> : base_class
	{
		static const unsigned int texcoord_count = _fvf_get_tex_coord_count<FVF>::value;
		static const unsigned int texcoord1_size = 1;
		float m_u;
	};

	template<> struct _data_helper<2> : base_class
	{
		static const unsigned int texcoord_count = _fvf_get_tex_coord_count<FVF>::value;
		static const unsigned int texcoord1_size = 2;
		point2 m_uv;
	};

	template<> struct _data_helper<3> : base_class
	{
		static const unsigned int texcoord_count = _fvf_get_tex_coord_count<FVF>::value;
		static const unsigned int texcoord1_size = 3;
		point3 m_uvw;
	};

	template<> struct _data_helper<4> : base_class
	{
		static const unsigned int texcoord_count = _fvf_get_tex_coord_count<FVF>::value;
		static const unsigned int texcoord1_size = 4;
		float m_uvs[4];
	};

	typedef _data_helper<_tcoord_count> data;
};

#define	FVF_TEXCOORD(N,Nm1)																\
template<unsigned FVF>																	\
struct _fvf_texcoord##N																	\
{																						\
	typedef typename _fvf_texcoord##Nm1<FVF>::data base_class;							\
	template<unsigned count> struct _data_helper;										\
	enum { _tcoord_count = _if_else														\
							<															\
								_less<_fvf_get_tex_coord_count<FVF>::value,N>::value,	\
								0,														\
								_fvf_get_tex_coord_size<FVF,Nm1>::value					\
							>::value };													\
	typedef _data_helper<_tcoord_count> data;											\
																						\
	template<> struct _data_helper<0> : base_class										\
	{																					\
		static const unsigned int texcoord##N##_size = 0;								\
	};																					\
	template<> struct _data_helper<1> : base_class										\
	{																					\
		static const unsigned int texcoord##N##_size = 1;								\
		float m_u##N;																	\
	};																					\
	template<> struct _data_helper<2> : base_class										\
	{																					\
		static const unsigned int texcoord##N##_size = 2;								\
		point2 m_uv##N;																	\
	};																					\
	template<> struct _data_helper<3> : base_class										\
	{																					\
		static const unsigned int texcoord##N##_size = 3;								\
		point3 m_uvw##N;																\
	};																					\
	template<> struct _data_helper<4> : base_class										\
	{																					\
		static const unsigned int texcoord##N##_size = 4;								\
		float m_uvs##N[4];																\
	};																					\
}

FVF_TEXCOORD(2,1);
FVF_TEXCOORD(3,2);
FVF_TEXCOORD(4,3);
FVF_TEXCOORD(5,4);
FVF_TEXCOORD(6,5);
FVF_TEXCOORD(7,6);
FVF_TEXCOORD(8,7);

#undef FVF_TEXCOORD

}

//=====================================================================================//
//                                  struct fvf_vertex                                  //
//=====================================================================================//
template<unsigned int FVF>
struct fvf_vertex : detail::_fvf_texcoord8<FVF>::data
{
	enum { fvf = FVF };
};

template<typename FVF>
struct fvf_vertex_traits
{
	static const bool has_pos = FVF::has_pos;
	static const bool has_rhw = FVF::has_rhw;
	static const bool has_blend_indices = FVF::has_blend_indices;
	static const bool has_normal = FVF::has_normal;
	static const bool has_psize = FVF::has_psize;
	static const bool has_diffuse = FVF::has_diffuse;
	static const bool has_specular = FVF::has_specular;
	static const unsigned int texcoord_count = FVF::texcoord_count;
	static const unsigned int texcoord1_size = FVF::texcoord1_size;
	static const unsigned int texcoord2_size = FVF::texcoord2_size;
	static const unsigned int texcoord3_size = FVF::texcoord3_size;
	static const unsigned int texcoord4_size = FVF::texcoord4_size;
	static const unsigned int texcoord5_size = FVF::texcoord5_size;
	static const unsigned int texcoord6_size = FVF::texcoord6_size;
	static const unsigned int texcoord7_size = FVF::texcoord7_size;
	static const unsigned int texcoord8_size = FVF::texcoord8_size;
	static const unsigned int blend_weight_count = FVF::blend_weight_count;
};

//=====================================================================================//
//                             inline unsigned fvf_size()                              //
//=====================================================================================//
inline unsigned int fvf_size(unsigned int fvf)
{
	return D3DXGetFVFVertexSize(fvf);
}

#pragma pack(pop)

// Проверка возможности branch / merge branches

#endif // !defined(__FVF_VERTEX_H_INCLUDED_7552370787048739__)