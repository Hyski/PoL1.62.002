#pragma once

#include "Ptr2.h"

namespace Muffle2
{

class SoundBuffer;
typedef Ptr<SoundBuffer> HSoundBuffer;

//=====================================================================================//
//                                   struct LockData                                   //
//=====================================================================================//
struct LockData
{
	short *data1;
	size_t size1;
	short *data2;
	size_t size2;

	LockData() : data1(0), size1(0), data2(0), size2(0) {}
};

//=====================================================================================//
//                                    struct Locker                                    //
//=====================================================================================//
struct Locker : LockData
{
	HSoundBuffer buffer;

	Locker() {}
	Locker(HSoundBuffer buf) : buffer(buf) {}
	Locker(Locker &locker) : LockData(locker), buffer(locker.buffer) { locker.buffer.reset(); }
	~Locker();

	Locker &operator=(Locker &locker)
	{
		Locker temp(locker);
		swap(*this,temp);
		return *this;
	}

	static void swap(Locker &l1, Locker &l2)
	{
		std::swap(l1.data1,l2.data1);
		std::swap(l1.data2,l2.data2);
		std::swap(l1.size1,l2.size1);
		std::swap(l1.size2,l2.size2);
		l1.buffer.swap(l2.buffer);
	}
};

}