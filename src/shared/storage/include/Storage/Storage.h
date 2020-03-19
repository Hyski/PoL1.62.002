#pragma once

#include <memory>
#include <typeinfo>
#include <mutual/rtti.h>
#include <mutual/object.h>
#include <mutual/handle.h>
#include <boost/utility.hpp>
#include <mll/io/binstream.h>
#include <boost/operators.hpp>
#include <mll/debug/exception.h>

#ifdef STORAGE_STATIC
#	define STORAGE_API
#else
#	ifdef __BUILDING_STORAGE__
#		define STORAGE_API __declspec(dllexport)
#	else
#		define STORAGE_API __declspec(dllimport)
#	endif
#endif

namespace Storage
{

class TypedPtr;
class Registry;
class Repository;
class PersistentBase;
class PersistentClass;

MLL_MAKE_EXCEPTION(Exception,mll::debug::exception);
	MLL_MAKE_EXCEPTION(SaveException,Exception);
	MLL_MAKE_EXCEPTION(LoadException,Exception);
		MLL_MAKE_EXCEPTION(WrongVersionLoadException,LoadException);
		MLL_MAKE_EXCEPTION(WrongTagLoadException,LoadException);
		MLL_MAKE_EXCEPTION(CyclicReferenceLoadException,LoadException);

}

#include "Ptr.h"
#include "Macros.h"
#include "Version.h"
#include "ReadSlot.h"
#include "TypedPtr.h"
#include "Registry.h"
#include "WriteSlot.h"
#include "Repository.h"
#include "PtrJunction.h"
#include "PersistentBase.h"
#include "PersistentClass.h"
#include "RepositoryIterator.h"