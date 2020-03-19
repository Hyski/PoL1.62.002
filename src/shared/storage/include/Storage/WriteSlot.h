#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                                   class WriteSlot                                   //
//=====================================================================================//
class WriteSlot
{
public:
	virtual mll::io::obinstream &getStream() = 0;
	template<typename T> void storeReference(const Ptr<T> &);
	template<typename T> void storeReference(const AutoPtr<T> &);

private:
	virtual void storeEmptyReference() = 0;
	virtual void storeReference(PersistentBase *p) = 0;
};

//=====================================================================================//
//                          void WriteSlot::storeReference()                           //
//=====================================================================================//
template<typename T>
void WriteSlot::storeReference(const Ptr<T> &p)
{
	if(p.expired()) storeEmptyReference();
	else storeReference(p.get());
}

//=====================================================================================//
//                          void WriteSlot::storeReference()                           //
//=====================================================================================//
template<typename T>
void WriteSlot::storeReference(const AutoPtr<T> &p)
{
	if(p.expired()) storeEmptyReference();
	else storeReference(p.get());
}

}