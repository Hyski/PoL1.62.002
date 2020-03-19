#ifndef _PRECOMP_H_
#define _PRECOMP_H_

//#include <ProjectConfig/config/mlconfig.h>
//#define ML_STATIC	// ƒл€ статической линковки с mutual

//	дл€ совместимости с DirectX 8.x
#define DIRECTINPUT_VERSION	0x0700
#define _WIN32_WINNT 0x0500

#ifndef D3D_OVERLOADS
#define D3D_OVERLOADS
#endif

#include <new>
#include <fstream>

#include <map>
#include <set>
#include <list>
#include <deque>
#include <queue>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <hash_map>
#include <algorithm>

#include <cstdio>

#ifndef _WINDOWS_
    //дл€ убыстрени€ компил€ции выбросим
    //лишнее из windows.h
    #define WIN32_LEAN_AND_MEAN                                 
    #include <windows.h>
	// ”берем эти ужасные макросы, объ€вленные где-то в windows.h
	#ifdef max
	#undef max
	#endif
	#ifdef min
	#undef min
	#endif
#endif

#include <mll/io/manip.h>
#include <mll/utils/table.h>
#include <mll/algebra/d3d7.h>
#include <mll/geometry/ray3.h>
#include <mll/geometry/obb3.h>
#include <mll/algebra/point2.h>
#include <mll/algebra/point3.h>
#include <mll/geometry/plane.h>
#include <mll/algebra/matrix3.h>
#include <mll/algebra/vector3.h>
#include <mll/algebra/quaternion.h>

#include <mutual/object.h>
#include <mutual/handle.h>

#include <time.h>
#include <dinput.h>
//подключим отладочную кучу
#ifdef _DEBUG
    #include <crtdbg.h>
    #define PUNCH_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new PUNCH_NEW
#endif //_DEBUG


#include <atlcomcli.h>

#include <boost/utility.hpp>
#include <boost/tokenizer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "Exception/Exception.h"

//--- Grom ---
#include "utils/stackguard.h"
//------------

#include "DataMgr/DataMgr.h"
#include "saveload/saveload.h"

#include "3d/geometry.h"
#include "Utils/Utils.h"
#include "utils/d3dutil.h"

#include "Log/Log.h"
#include "Timer/Timer.h"
#include "DebugInfo/DebugInfo.h"

//#include "D3DApp/input/input.h"
#include "D3DAPP/d3dkernel/d3dkernel.h"

#include <mll/debug/log.h>
#include <mll/io/debug_ostream.h>

#if !defined(_HOME_VERSION)
//#define _HOME_VERSION
#endif

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

//#define NO_BLOOD //For German users :-(
//#define USE_SECUROM_TRIGGERS

//#include <undercover/MlVersion.h>
//#include "../DataFromFile.h"

#endif	//_PRECOMP_H_	