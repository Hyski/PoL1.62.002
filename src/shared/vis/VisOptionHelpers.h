#if !defined(__VIS_OPTION_HELPERS_H_INCLUDED_2074000990995753__)
#define __VIS_OPTION_HELPERS_H_INCLUDED_2074000990995753__

#include "Vis.h"

//=====================================================================================//
//                                   namespace Maya                                    //
//=====================================================================================//
namespace Vis
{

#define VIS_FOREACH_PARAMETER_TYPE(A)	\
	A(Vector)	\
	A(Integer)	\
	A(String)	\
	A(Float)	\
	A(Boolean)

#define VIS_GET_TYPEVector				vector3
#define VIS_GET_TYPEInteger				int
#define VIS_GET_TYPEString				std::string
#define VIS_GET_TYPEFloat				float
#define VIS_GET_TYPEBoolean				bool

#define VIS_IS_REFVector				_REF
#define VIS_IS_REFInteger				_NOREF
#define VIS_IS_REFString				_REF
#define VIS_IS_REFFloat					_REF
#define VIS_IS_REFBoolean				_NOREF

//=====================================================================================//
//                                   enum OptionType                                   //
//=====================================================================================//
enum OptionType
{
#define VIS_MAKE_ENUM(A)		ot ## A,
	VIS_FOREACH_PARAMETER_TYPE(VIS_MAKE_ENUM)
#undef VIS_MAKE_ENUM

	otTerminator
};

template<unsigned int PT> struct OptionTraits;

#define VIS_CAT2(A,B)	A ## B
#define VIS_CAT(A,B)	VIS_CAT2(A,B)

#define VIS_MAKE_TRAITS_NOREF(PT,T)	\
	template<> struct OptionTraits<ot##PT> { typedef T value_type; typedef value_type reference_type; };
#define VIS_MAKE_TRAITS_REF(PT,T)	\
	template<> struct OptionTraits<ot##PT> { typedef T value_type; typedef const value_type &reference_type; };
#define VIS_MAKE_TRAITS(PT)	\
	VIS_CAT(VIS_MAKE_TRAITS,VIS_IS_REF##PT)(PT,VIS_GET_TYPE##PT)

	VIS_FOREACH_PARAMETER_TYPE(VIS_MAKE_TRAITS)

#undef VIS_MAKE_TRAITS
#undef VIS_MAKE_TRAITS_NOREF
#undef VIS_MAKE_TRAITS_REF
#undef VIS_CAT
#undef VIS_CAT2

}

#endif // !defined(__VIS_OPTION_HELPERS_H_INCLUDED_2074000990995753__)