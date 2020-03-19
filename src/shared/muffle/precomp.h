#if !defined(__PRECOMP_H_INCLUDED_5992692519605117__)
#define __PRECOMP_H_INCLUDED_5992692519605117__

#if !defined(NOWINBASEINTERLOCK)
//#define NOWINBASEINTERLOCK
#endif


#define NOMINMAX
#define __MUFFLE_EXPORTS__
#define _WIN32_WINNT 0x0400

#include <new>
#include <list>
#include <memory>
#include <string>
#include <sstream>
#include <exception>
#include <assert.h>
#include <hash_map>
#include <map>
#include <algorithm>
#include <vector>
#include <deque>
#include <strstream>
#include <set>
#include <iomanip>
#include <time.h>

//#ifdef _DEBUG
//    #include <crtdbg.h>
//    #define PUNCH_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
//    #define new PUNCH_NEW
//#endif //_DEBUG

#include <dsound.h>
#include <atlcomcli.h>

const unsigned int MaxHandles = MAXIMUM_WAIT_OBJECTS;

#include <mll/algebra/point3.h>
#include <mll/algebra/vector3.h>

#include <Muffle/ISound.h>
#include <Muffle/VFS.h>

#include <mll/io/binstream.h>
#include <mll/io/manip.h>
#include <mll/debug/static_assert.h>
#include <mll/debug/reporter.h>
#include <mll/debug/log.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/preprocessor/cat.hpp>

using mll::io::ibinstream;

#if 0&&defined(_DEBUG)
#define KERNEL_LOG(List)	MLL_MAKE_DEBUG_LOG("sound/kernel.log",List)
#define NOFILE_LOG(List)	MLL_MAKE_LOG("sound/file_not_found.log",List)
#else
#define KERNEL_LOG(List)
#define NOFILE_LOG(List)
#endif

//=====================================================================================//
//                                 bool is_lowercase()                                 //
//=====================================================================================//
inline bool is_lowercase(const std::string &str)
{
	for(std::string::const_iterator i = str.begin(); i != str.end(); ++i)
	{
		if(isalpha(*i) && !islower(*i)) return false;
	}
	return true;
}

namespace Muffle
{

class Stream;
struct Format;

}

namespace Muffle2
{

typedef Muffle::snd_vector vec;
using Muffle::Stream;
using Muffle::Format;
using Muffle::HScript;

//=====================================================================================//
//                             inline long normToDecibel()                             //
//=====================================================================================//
inline long normToDecibel(float vol)
{
	const float minimal = 0.05f; // 0.05f == powf(expf(1.0f),-9.6f);
	if(vol <= minimal) return -10000L;
	if(vol >= 1.0f) return 0L;
	return static_cast<long>(1000.0f*logf(vol));
}

}

#define MUFFLE_SYNCHRONIZED_THIS	Muffle2::Lock BOOST_PP_CAT(scopeguard,__LINE__) = lock()
#define MUFFLE_SYNCHRONIZED(O)		Muffle2::Lock BOOST_PP_CAT(scopeguard,__LINE__) = (O)->lock()

#include "safety.h"
#include "guid.h"
#include "CriticalSection.h"
#include "Stream.h"

#endif // !defined(__PRECOMP_H_INCLUDED_5992692519605117__)