#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                                   class ReadSlot                                    //
//=====================================================================================//
class ReadSlot
{
public:
	virtual HPtrJunction getPtrJunction() = 0;
	virtual mll::io::ibinstream &getStream() = 0;
	virtual unsigned int getSize() const = 0;
	template<typename T> Ptr<T> restoreWeakReference();
	template<typename T> Ptr<T> restoreReference();

	template<typename T> T *getExternalReference();

private:
	virtual HPtrJunction restoreWeakReference() = 0;
	virtual HPtrJunction restoreReference() = 0;
	virtual void *getExternalReference(const std::type_info &) = 0;
};

//=====================================================================================//
//                          ReadSlot::restoreWeakReference()                           //
//=====================================================================================//
template<typename T>
Ptr<T> ReadSlot::restoreWeakReference()
{
	return Ptr<T>(restoreWeakReference());
}

//=====================================================================================//
//                            ReadSlot::restoreReference()                             //
//=====================================================================================//
template<typename T>
Ptr<T> ReadSlot::restoreReference()
{
	return Ptr<T>(restoreReference());
}

//=====================================================================================//
//                         T *ReadSlot::getExternalReference()                         //
//=====================================================================================//
template<typename T>
T *ReadSlot::getExternalReference()
{
	return reinterpret_cast<T *>(getExternalReference(typeid(T)));
}

}