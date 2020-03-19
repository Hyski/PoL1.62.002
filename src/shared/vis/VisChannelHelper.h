#if !defined(__VIS_CHANNEL_HELPER_H_INCLUDED_4240531301943244__)
#define __VIS_CHANNEL_HELPER_H_INCLUDED_4240531301943244__

#include "VisChannelBase.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

namespace detail
{

//=====================================================================================//
//                                struct ChannelTraits                                 //
//=====================================================================================//
template<typename T, typename Tag>
struct ChannelTraits
{
	typedef T Value_t;
	typedef Tag Tag_t;
};

template<typename Traits>
struct GetTypeName;

#define VIS_FOREACH_CHANNEL_TYPE(A)	\
	A(Position3)					\
	A(Normal3)						\
	A(TexCoord2)					\
	A(TexCoord3)					\
	A(Float)						\
	A(UInt)							\
	A(Index16)

#define VIS_CHANNEL_TRAITSPosition3	::Vis::detail::ChannelTraits<::Vis::point3,TransformChannelTag>
#define VIS_CHANNEL_TRAITSNormal3	::Vis::detail::ChannelTraits<::Vis::vector3,TransformChannelTag>
#define VIS_CHANNEL_TRAITSTexCoord2	::Vis::detail::ChannelTraits<::Vis::point2,NonMutatingChannelTag>
#define VIS_CHANNEL_TRAITSTexCoord3	::Vis::detail::ChannelTraits<::Vis::point3,NonMutatingChannelTag>
#define VIS_CHANNEL_TRAITSFloat		::Vis::detail::ChannelTraits<float,NonMutatingChannelTag>
#define VIS_CHANNEL_TRAITSUInt		::Vis::detail::ChannelTraits<unsigned int,NonMutatingChannelTag>
#define VIS_CHANNEL_TRAITSIndex16	::Vis::detail::ChannelTraits<unsigned short,GrowChannelTag>

#define VIS_MAKE_TYPE_NAME(A)	\
	template<> struct GetTypeName< VIS_CHANNEL_TRAITS##A > { static std::string get() { return #A; } };

VIS_FOREACH_CHANNEL_TYPE(VIS_MAKE_TYPE_NAME)

#undef VIS_MAKE_TYPE_NAME

}

}

#endif // !defined(__VIS_CHANNEL_HELPER_H_INCLUDED_4240531301943244__)