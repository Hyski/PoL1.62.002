#pragma once

#define DIRECTINPUT_VERSION	0x0700
#define _WIN32_WINNT 0x0500

#define D3D_OVERLOADS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <new>

#ifdef _DEBUG
    #include <crtdbg.h>
    #define PUNCH_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new PUNCH_NEW
#endif //_DEBUG

#include <d3d.h>
#include <dinput.h>

#include <mll/algebra/d3d7.h>
#include <mll/algebra/point2.h>
#include <mll/algebra/point3.h>
#include <mll/algebra/quaternion.h>

#include <mll/io/manip.h>
#include <mll/geometry/ray3.h>
#include <mll/geometry/obb3.h>
#include <mll/geometry/plane.h>
#include <mll/debug/log.h>
#include <mll/io/debug_ostream.h>

#include <atlcomcli.h>

#include <map>
#include <set>
#include <list>
#include <queue>
#include <string>
#include <memory>
#include <vector>
#include <iomanip>
#include <hash_map>
#include <hash_set>
#include <algorithm>

#include <mll/utils/table.h>

#include <common/3d/quat.h>
#include <common/3d/geometry.h>
#include <common/datamgr/datamgr.h>
#include <common/saveload/saveload.h>

#include <common/log/log.h>
#include <common/utils/utils.h>
#include <common/utils/d3dutil.h>
#include <common/exception/exception.h>

#include <common/timer/timer.h>
#include <common/utils/vfilewpr.h>
#include <common/debuginfo/debuginfo.h>

#include <common/d3dapp/d3dkernel/d3dkernel.h>

#include "MlVersion.h"

#include <mutual/object.h>
#include <mutual/handle.h>

#include <boost/utility.hpp>
#include <boost/tokenizer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <windows.h>

#ifdef _HOME_VERSION
#	define	POL_LOG(L)	MLL_MAKE_LOG("power_of_law.log",L)
#else
#	define	POL_LOG(L)
#endif

#ifdef _HOME_VERSION
#	define	POL_DEBUG_OUT(L)	MLL_DEBUG_OUT(L)
#else
#	define	POL_DEBUG_OUT(L)
#endif

#define	POL_MAKE_STAMP	__FILE__ << "(" << __LINE__ << ") : "
