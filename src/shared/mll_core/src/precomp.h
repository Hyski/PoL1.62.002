#if !defined(__PRECOMP_H_INCLUDED_7552370787048739__)
#define __PRECOMP_H_INCLUDED_7552370787048739__

#define NOWINBASEINTERLOCK
#define WIN32_LEAN_AND_MEAN                                 
#define NOMINMAX

#define __COMPILING_MLL_LIBRARIES__

#include <mll/algebra/_typedefs.h>

#include <mll/geometry/ray3.h>
#include <mll/geometry/aabb3.h>

#include <mll/utils/named_vars.h>
#include <mll/utils/timer.h>

#include <mll/io/manip.h>
#include <mll/io/printf.h>
#include <mll/io/iutils.h>
#include <mll/io/debug_ostream.h>

#include <mll/debug/log.h>
#include <mll/debug/renderer.h>
#include <mll/debug/marker.h>

#include <locale>
#include <algorithm>
#include <string>
#include <fstream>
#include <map>
#include <sstream>

#include <windows.h>
#include <assert.h>

#endif // !defined(__PRECOMP_H_INCLUDED_7552370787048739__)