#if !defined(__VIS_H_INCLUDED_9442506872545770__)
#define __VIS_H_INCLUDED_9442506872545770__

#ifndef NOMINMAX
#	define NOMINMAX
#endif

#include <set>
#include <list>
#include <string>
#include <sstream>
#include <hash_map>
#include <algorithm>
#include <functional>

#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

#include <mll/debug/log.h>
#include <mll/debug/exception.h>

#include <mll/geometry/ray3.h>
#include <mll/geometry/obb3.h>
#include <mll/algebra/point2.h>
#include <mll/algebra/point3.h>
#include <mll/algebra/vector2.h>
#include <mll/algebra/vector3.h>
#include <mll/algebra/matrix3.h>
#include <mll/utils/lcrn_tree.h>
#include <mll/utils/animation.h>
#include <mll/algebra/quaternion.h>

#include <mutual/rtti.h>
#include <mutual/object.h>
#include <mutual/handle.h>

#include <Storage/Storage.h>

#if !defined(VIS_BASIC_STATIC)
#	if !defined(__ML_BUILDING_VIS__)
#		define VIS_IMPORT	__declspec(dllimport)
#	else
#		define VIS_IMPORT	__declspec(dllexport)
#	endif
#else
#	define VIS_IMPORT
#endif

#if !defined(VIS_BUFFER_STATIC)
#	if !defined(__ML_BUILDING_VIS_BUFFER__)
#		define VIS_BUFFER_IMPORT	__declspec(dllimport)
#	else
#		define VIS_BUFFER_IMPORT	__declspec(dllexport)
#	endif
#else
#	define VIS_BUFFER_IMPORT
#endif

#if !defined(VIS_MESH_STATIC)
#	if !defined(__ML_BUILDING_VIS_MESH__)
#		define VIS_MESH_IMPORT	__declspec(dllimport)
#	else
#		define VIS_MESH_IMPORT	__declspec(dllexport)
#	endif
#else
#	define VIS_MESH_IMPORT
#endif

#if !defined(VIS_MESH_RENDER_STATIC)
#	if !defined(__ML_BUILDING_VIS_MESH_RENDER__)
#		define VIS_MESH_RENDER_IMPORT	__declspec(dllimport)
#	else
#		define VIS_MESH_RENDER_IMPORT	__declspec(dllexport)
#	endif
#else
#	define VIS_MESH_RENDER_IMPORT
#endif

#ifdef VIS_BASIC_STATIC
#	define VIS_BASIC_PTAG	1
#else
#	define VIS_BASIC_PTAG	0
#endif

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

using mll::geometry::obb3;
using mll::geometry::ray3;
using mll::algebra::point2;
using mll::algebra::point3;
using mll::algebra::vector2;
using mll::algebra::vector3;
using mll::algebra::matrix3;
using mll::utils::animation;
using mll::algebra::quaternion;

MLL_MAKE_EXCEPTION(Error,mll::debug::exception);
	MLL_MAKE_EXCEPTION(WrongType,Error);
	MLL_MAKE_EXCEPTION(OptionNotFound,Error);
	MLL_MAKE_EXCEPTION(AnimError,Error);

class Mesh;
class Option;
class MeshState;

typedef MlObjHandle<Mesh> HMesh;
typedef MlObjHandle<Option> HOption;
typedef MlObjHandle<MeshState> HMeshState;

}

#include "VisNode.h"
#include "VisShape.h"
#include "VisPiece.h"
#include "VisOption.h"
#include "VisChannel.h"
#include "VisMaterial.h"
#include "VisAnimTrack.h"
#include "VisParticles.h"
#include "VisOptionSet.h"
#include "VisPolyGroup.h"
#include "VisChannelBase.h"
#include "VisChannelMerger.h"
#include "VisOptionHelpers.h"

#endif // !defined(__VIS_H_INCLUDED_9442506872545770__)