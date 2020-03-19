#pragma once

#include "Storage.h"
#include "RepositoryIterator.h"

namespace Storage
{

//=====================================================================================//
//                                   class Repository                                  //
//=====================================================================================//
/// Класс - хранилище объектов
class STORAGE_API Repository : boost::noncopyable
{
	class Impl;
	std::auto_ptr<Impl> m_pimpl;

public:
	template<typename T> struct Iterator { typedef RepositoryIterator<T> type; };
	template<typename T> struct ConstIterator { typedef RepositoryConstIterator<T> type; };
	template<typename T> struct Range { typedef std::pair<typename Iterator<T>::type,typename Iterator<T>::type> type; };
	template<typename T> struct ConstRange { typedef std::pair<typename ConstIterator<T>::type,typename ConstIterator<T>::type> type; };
	typedef Iterator<PersistentBase>::type AllIterator;
	typedef ConstIterator<PersistentBase>::type AllConstIterator;
	typedef Range<PersistentBase>::type AllRange;
	typedef ConstRange<PersistentBase>::type AllConstRange;

	/// Конструктор хранилища
	Repository();
	/// Конструктор хранилища
	Repository(const std::string &tag);
	/// Деструктор хранилища
	~Repository();

	/// Получить версию хранилища
	const Version &getVersion() const;

	/// Сохранить содержимое хранилища
	void store(mll::io::obinstream &) const;
	/// Восстановить содержимое хранилища
	void restore(mll::io::ibinstream &);

	/// Получить диапазон объектов типа T
	template<typename T> typename Iterator<T>::type begin();
	template<typename T> typename Iterator<T>::type end();

	/// Получить диапазон константных объектов типа T
	template<typename T> typename ConstIterator<T>::type begin() const;
	template<typename T> typename ConstIterator<T>::type end() const;

	/// Получить сразу два итератора на диапазон объектов
	template<typename T> typename Range<T>::type getObjects();
	template<typename T> typename ConstRange<T>::type getObjects() const;

	/// Получить диапазон объектов типа T
	AllIterator allBegin();
	AllIterator allEnd();
	AllConstIterator allBegin() const;
	AllConstIterator allEnd() const;

	AllRange getAllObjects();
	AllConstRange getAllObjects() const;

	template<typename T> void addReference(T *);
	template<typename T> void removeReference(T *);

	void clear();

	/// Создать объект в хранилище
	template<typename T> T *construct();
	template<typename T, typename P1> T *construct(P1);
	template<typename T, typename P1, typename P2> T *construct(P1,P2);
	template<typename T, typename P1, typename P2, typename P3> T *construct(P1,P2,P3);
	template<typename T, typename P1, typename P2, typename P3, typename P4> T *construct(P1,P2,P3,P4);

private:
	template<typename T> T *addObject(T *);
	void addObject(const TypedPtr &);
	void removeObject(void *);

	RepositoryIteratorBase begin(class_info);
	RepositoryConstIteratorBase begin(class_info) const;
	RepositoryIteratorBase end(class_info);
	RepositoryConstIteratorBase end(class_info) const;
	std::pair<RepositoryIteratorBase,RepositoryIteratorBase> getObjects(class_info);
	std::pair<RepositoryConstIteratorBase,RepositoryConstIteratorBase> getObjects(class_info) const;

	void addReference(void *, const std::type_info &);
	void removeReference(void *, const std::type_info &);

private:
	friend class PersistentBase;

	void *allocate(size_t);
	static void *allocateStatic(size_t);
	static void deallocateStatic(void *);
};

//=====================================================================================//
//                               T *Storage::addObject()                               //
//=====================================================================================//
template<typename T>
T *Repository::addObject(T *obj)
{
	addObject(TypedPtr(obj));
	return obj;
}

//=====================================================================================//
//                               T *Storage::construct()                               //
//=====================================================================================//
template<typename T>
T *Repository::construct()
{
	return addObject(new(this) T);
}

//=====================================================================================//
//                               T *Storage::construct()                               //
//=====================================================================================//
template<typename T, typename P1>
T *Repository::construct(P1 p1)
{
	T *result = addObject(new(this) T(p1));
	// Если выпал следующий ассерт, то, скорее всего, в классе не указан макрос ML_RTTI*
	assert( result->rtti_dynamic().std_type_info() == T::rtti_static().std_type_info() );
	return result;
}

//=====================================================================================//
//                               T *Storage::construct()                               //
//=====================================================================================//
template<typename T, typename P1, typename P2>
T *Repository::construct(P1 p1, P2 p2)
{
	T *result = addObject(new(this) T(p1,p2));
	// Если выпал следующий ассерт, то, скорее всего, в классе не указан макрос ML_RTTI*
	assert( result->rtti_dynamic().std_type_info() == T::rtti_static().std_type_info() );
	return result;
}

//=====================================================================================//
//                             T *Repository::construct()                              //
//=====================================================================================//
template<typename T, typename P1, typename P2, typename P3>
T *Repository::construct(P1 p1, P2 p2, P3 p3)
{
	T *result = addObject(new(this) T(p1,p2,p3));
	// Если выпал следующий ассерт, то, скорее всего, в классе не указан макрос ML_RTTI*
	assert( result->rtti_dynamic().std_type_info() == T::rtti_static().std_type_info() );
	return result;
}

//=====================================================================================//
//                             T *Repository::construct()                              //
//=====================================================================================//
template<typename T, typename P1, typename P2, typename P3, typename P4>
T *Repository::construct(P1 p1, P2 p2, P3 p3, P4 p4)
{
	T *result = addObject(new(this) T(p1,p2,p3,p4));
	// Если выпал следующий ассерт, то, скорее всего, в классе не указан макрос ML_RTTI*
	assert( result->rtti_dynamic().std_type_info() == T::rtti_static().std_type_info() );
	return result;
}

//=====================================================================================//
//                       inline void Repository::addReference()                        //
//=====================================================================================//
template<typename T>
inline void Repository::addReference(T *ptr)
{
	addReference(reinterpret_cast<void*>(ptr),typeid(T));
}

//=====================================================================================//
//                      inline void Repository::removeReference()                      //
//=====================================================================================//
template<typename T>
inline void Repository::removeReference(T *ptr)
{
	removeReference(reinterpret_cast<void*>(ptr),typeid(T));
}

//=====================================================================================//
//              inline Repository::Iterator<T>::type Repository::begin()               //
//=====================================================================================//
template<typename T>
inline typename Repository::Iterator<T>::type Repository::begin()
{
	return begin(T::rtti_static());
}

//=====================================================================================//
//               inline Repository::Iterator<T>::type Repository::end()                //
//=====================================================================================//
template<typename T>
inline typename Repository::Iterator<T>::type Repository::end()
{
	return end(T::rtti_static());
}

//=====================================================================================//
//         inline Repository::ConstIterator<T>::type Repository::begin() const         //
//=====================================================================================//
template<typename T>
inline typename Repository::ConstIterator<T>::type Repository::begin() const
{
	return begin(T::rtti_static());
}

//=====================================================================================//
//          inline Repository::ConstIterator<T>::type Repository::end() const          //
//=====================================================================================//
template<typename T>
inline typename Repository::ConstIterator<T>::type Repository::end() const
{
	return end(T::rtti_static());
}

//=====================================================================================//
//                inline Repository::AllIterator Repository::allBegin()                //
//=====================================================================================//
inline Repository::AllIterator Repository::allBegin()
{
	return begin(class_info(0));
}

//=====================================================================================//
//                 inline Repository::AllIterator Repository::allEnd()                 //
//=====================================================================================//
inline Repository::AllIterator Repository::allEnd()
{
    return end(class_info(0));
}

//=====================================================================================//
//          inline Repository::AllConstIterator Repository::allBegin() const           //
//=====================================================================================//
inline Repository::AllConstIterator Repository::allBegin() const
{
	return begin(class_info(0));
}

//=====================================================================================//
//           inline Repository::AllConstIterator Repository::allEnd() const            //
//=====================================================================================//
inline Repository::AllConstIterator Repository::allEnd() const
{
	return end(class_info(0));
}

//=====================================================================================//
//         inline typename Repository::Range<T>::type Repository::getObjects()         //
//=====================================================================================//
template<typename T>
inline typename Repository::Range<T>::type Repository::getObjects()
{
	typedef std::pair<RepositoryIteratorBase,RepositoryIteratorBase> Pair_t;
	Pair_t pair = getObjects(T::rtti_static());
	return typename Range<T>::type(pair.first,pair.second);
}

//=====================================================================================//
//   inline typename Repository::ConstRange<T>::type Repository::getObjects() const    //
//=====================================================================================//
template<typename T>
inline typename Repository::ConstRange<T>::type Repository::getObjects() const
{
	typedef std::pair<RepositoryConstIteratorBase,RepositoryConstIteratorBase> Pair_t;
	Pair_t pair = getObjects(T::rtti_static());
	return typename ConstRange<T>::type(pair.first,pair.second);
}

//=====================================================================================//
//                  Repository::AllRange Repository::getAllObjects()                   //
//=====================================================================================//
inline Repository::AllRange Repository::getAllObjects()
{
	return getObjects(class_info(0));
}

//=====================================================================================//
//             Repository::AllConstRange Repository::getAllObjects() const             //
//=====================================================================================//
inline Repository::AllConstRange Repository::getAllObjects() const
{
	return getObjects(class_info(0));
}

}