#include "precomp.h"

namespace Storage
{

static const Version g_repositoryVersion(0,2);
static const Version g_typeTableVersion(0,1);

static const std::string g_repositoryTag = "MLOS";
static const std::string g_typeTableTag = "TTBL";

namespace RepositoryDetail
{

//=====================================================================================//
//                                   class Allocator                                   //
//=====================================================================================//
class Allocator
{
public:
	void *allocate(size_t s)
	{
		return malloc(s);
	}

	void deallocate(void *a)
	{
		free(a);
	}
};

Allocator g_alloc;

//=====================================================================================//
//                                 struct StorageEntry                                 //
//=====================================================================================//
struct StorageEntry
{
	TypedPtr m_ptr;

	explicit StorageEntry(TypedPtr ptr) : m_ptr(ptr) {}

	bool operator<(const StorageEntry &e) const
	{
		if(m_ptr.getType() == e.m_ptr.getType()) return m_ptr.getObject() < e.m_ptr.getObject();
		return m_ptr.getType() < e.m_ptr.getType();
	}
};

//=====================================================================================//
//                              struct ExternalReference                               //
//=====================================================================================//
struct ExternalReference
{
	const std::type_info *m_type;
	void *m_ptr;

	ExternalReference(const std::type_info *type, void *ptr) : m_type(type), m_ptr(ptr) {}

	bool operator<(const ExternalReference &r) const
	{
		return m_type->before(*r.m_type) != 0;
	}
};

//=====================================================================================//
//                 struct TypeSliceCompare : public rtti_slice_compare                 //
//=====================================================================================//
struct TypeSliceCompare : public rtti_slice_compare
{
public:
	TypeSliceCompare(const class_info &ci) : rtti_slice_compare(ci) {}
	
	bool operator()(const StorageEntry &l, const StorageEntry &r) const
	{
		return rtti_slice_compare::operator()(l.m_ptr.getType(),r.m_ptr.getType());
	}

	bool operator()(const StorageEntry &l, const class_info &r) const
	{
		return rtti_slice_compare::operator()(l.m_ptr.getType(),r);
	}

	bool operator()(const class_info &l, const StorageEntry &r) const
	{
		return rtti_slice_compare::operator()(l,r.m_ptr.getType());
	}
};

//=====================================================================================//
//                               struct ClassInfoCompare                               //
//=====================================================================================//
struct ClassInfoCompare
{
	bool operator()(const class_info &l, const class_info &r) const
	{
		return l.name() < r.name();
	}
};

} // namespace RepositoryDetail

using namespace RepositoryDetail;

//=====================================================================================//
//                                class RepositoryImpl                                 //
//=====================================================================================//
class RepositoryImpl
{
public:
	Allocator m_alloc;
	std::string m_tag;

	typedef std::set<StorageEntry> Entries_t;
	typedef std::vector<Entries_t::iterator> TypeIndex_t;
	typedef std::set<ExternalReference> ExtReferences_t;

	Entries_t m_entries;
	TypeIndex_t m_index;
	ExtReferences_t m_extRefs;

	RepositoryImpl() {}
	RepositoryImpl(const std::string &t) : m_tag(t) {}
	
	void clean()
	{
		while(!m_entries.empty())
		{
			const StorageEntry &e = *m_entries.begin();
			PersistentBase *p = e.m_ptr.getObject();
			delete p;
		}
	}

	void store(mll::io::obinstream &out) const;
	void restore(mll::io::ibinstream &out, Repository *);

	Entries_t::iterator lower_bound(class_info ci)
	{
		return std::lower_bound(m_entries.begin(),m_entries.end(),ci,TypeSliceCompare(ci));
	}

	Entries_t::const_iterator lower_bound(class_info ci) const
	{
		return std::lower_bound(m_entries.begin(),m_entries.end(),ci,TypeSliceCompare(ci));
	}

	Entries_t::iterator upper_bound(class_info ci)
	{
		return std::upper_bound(m_entries.begin(),m_entries.end(),ci,TypeSliceCompare(ci));
	}

	Entries_t::const_iterator upper_bound(class_info ci) const
	{
		return std::upper_bound(m_entries.begin(),m_entries.end(),ci,TypeSliceCompare(ci));
	}

private:
	void restore0(mll::io::ibinstream &out, const Version &, Repository *);
};

//=====================================================================================//
//                               struct TypeIndexCompare                               //
//=====================================================================================//
struct TypeIndexCompare
{
	bool operator()(const RepositoryImpl::Entries_t::iterator &l,
					const RepositoryImpl::Entries_t::iterator &r) const
	{
        return l->m_ptr.getObject() < r->m_ptr.getObject();
	}

	bool operator()(const RepositoryImpl::Entries_t::iterator &l,
					void *ptr) const
	{
        return l->m_ptr.getObject() < ptr;
	}

	bool operator()(void *ptr,
					const RepositoryImpl::Entries_t::iterator &r) const
	{
        return ptr < r->m_ptr.getObject();
	}
};

//=====================================================================================//
//                                   class RPCIBImpl                                   //
//=====================================================================================//
class RPCIBImpl
{
public:
	typedef RepositoryImpl::Entries_t::const_iterator Itor_t;
	Itor_t m_itor;

	RPCIBImpl() {}
	RPCIBImpl(Itor_t i) : m_itor(i) {}
};

//=====================================================================================//
//                                   class RPIBImpl                                    //
//=====================================================================================//
class RPIBImpl
{
public:
	typedef RepositoryImpl::Entries_t::iterator Itor_t;
	Itor_t m_itor;

	RPIBImpl() {}
	RPIBImpl(Itor_t i) : m_itor(i) {}
};

//=====================================================================================//
//                       class RepositoryConstIteratorBase::Impl                       //
//=====================================================================================//
class RepositoryConstIteratorBase::Impl : public RPCIBImpl
{
public:
	Impl() {}
	Impl(Itor_t i) : RPCIBImpl(i) {}
	Impl(const RPIBImpl &i) : RPCIBImpl(i.m_itor) {}
};

//=====================================================================================//
//                         class RepositoryIteratorBase::Impl                          //
//=====================================================================================//
class RepositoryIteratorBase::Impl : public RPIBImpl
{
public:
	Impl() {}
	Impl(Itor_t i) : RPIBImpl(i) {}
};

//=====================================================================================//
//             RepositoryConstIteratorBase::RepositoryConstIteratorBase()              //
//=====================================================================================//
RepositoryConstIteratorBase::RepositoryConstIteratorBase()
:	m_pimpl(new Impl)
{
}

//=====================================================================================//
//             RepositoryConstIteratorBase::~RepositoryConstIteratorBase()             //
//=====================================================================================//
RepositoryConstIteratorBase::~RepositoryConstIteratorBase()
{
	delete m_pimpl;
}

//=====================================================================================//
//             RepositoryConstIteratorBase::RepositoryConstIteratorBase()              //
//=====================================================================================//
RepositoryConstIteratorBase::RepositoryConstIteratorBase(const RepositoryConstIteratorBase &i)
:	m_pimpl(new Impl(*i.m_pimpl))
{
}

//=====================================================================================//
//             RepositoryConstIteratorBase::RepositoryConstIteratorBase()              //
//=====================================================================================//
RepositoryConstIteratorBase::RepositoryConstIteratorBase(const RepositoryIteratorBase &i)
:	m_pimpl(new Impl(*i.m_pimpl))
{
}

//=====================================================================================//
//                      void RepositoryConstIteratorBase::next()                       //
//=====================================================================================//
void RepositoryConstIteratorBase::next()
{
	++m_pimpl->m_itor;
}

//=====================================================================================//
//                  bool RepositoryConstIteratorBase::equals() const                   //
//=====================================================================================//
bool RepositoryConstIteratorBase::equals(const RepositoryConstIteratorBase &i) const
{
	return m_pimpl->m_itor == i.m_pimpl->m_itor;
}

//=====================================================================================//
//          const PersistentBase *RepositoryConstIteratorBase::deref() const           //
//=====================================================================================//
const PersistentBase *RepositoryConstIteratorBase::deref() const
{
	return m_pimpl->m_itor->m_ptr.getObject();
}

//=====================================================================================//
//                  RepositoryIteratorBase::RepositoryIteratorBase()                   //
//=====================================================================================//
RepositoryIteratorBase::~RepositoryIteratorBase()
{
	delete m_pimpl;
}

//=====================================================================================//
//                  RepositoryIteratorBase::RepositoryIteratorBase()                   //
//=====================================================================================//
RepositoryIteratorBase::RepositoryIteratorBase()
:	m_pimpl(new Impl)
{
}

//=====================================================================================//
//                  RepositoryIteratorBase::RepositoryIteratorBase()                   //
//=====================================================================================//
RepositoryIteratorBase::RepositoryIteratorBase(const RepositoryIteratorBase &i)
:	m_pimpl(new Impl(*i.m_pimpl))
{
}

//=====================================================================================//
//                         void RepositoryIteratorBase::next()                         //
//=====================================================================================//
void RepositoryIteratorBase::next()
{
	++m_pimpl->m_itor;
}

//=====================================================================================//
//                     bool RepositoryIteratorBase::equals() const                     //
//=====================================================================================//
bool RepositoryIteratorBase::equals(const RepositoryIteratorBase &i) const
{
	return m_pimpl->m_itor == i.m_pimpl->m_itor;
}

//=====================================================================================//
//                PersistentBase *RepositoryIteratorBase::deref() const                //
//=====================================================================================//
PersistentBase *RepositoryIteratorBase::deref() const
{
	return m_pimpl->m_itor->m_ptr.getObject();
}

//=====================================================================================//
//                                 class Storage::Impl                                 //
//=====================================================================================//
class Repository::Impl : public RepositoryImpl
{
public:
	Impl() {}
	Impl(const std::string &t) : RepositoryImpl(t) {}
};

//=====================================================================================//
//                       class WriteSlotImpl : public WriteSlot                        //
//=====================================================================================//
class WriteSlotImpl : public WriteSlot
{
	const RepositoryImpl *m_rep;
	mll::io::obinstream &m_outerStream;
	std::ostringstream m_stream;
	mll::io::obinstream m_out;

public:
	WriteSlotImpl(const RepositoryImpl *rep, mll::io::obinstream &os)
	:	m_rep(rep),
		m_outerStream(os),
		m_stream(std::ios::binary),
		m_out(m_stream) {}

	virtual mll::io::obinstream &getStream()
	{
		return m_out;
	}

	virtual void storeReference(PersistentBase *pb)
	{
		RepositoryImpl::TypeIndex_t::const_iterator i = 
			std::lower_bound(m_rep->m_index.begin(),m_rep->m_index.end(),pb,TypeIndexCompare());
		unsigned int index = std::distance(m_rep->m_entries.begin(),*i);
		assert( index != m_rep->m_entries.size() );
		m_out << index;
	}

	virtual void storeEmptyReference()
	{
		m_out << unsigned int(0xFFFFFFFF);
	}

	void flush()
	{
		const std::string data = m_stream.str();
		m_outerStream << data.size();
		m_outerStream.stream().write(reinterpret_cast<const char *>(data.c_str()),data.size());
	}
};

//=====================================================================================//
//                                   class ReadData                                    //
//=====================================================================================//
struct ReadData
{
	typedef std::vector<PersistentClass *> TypeSet_t;
	//typedef std::vector<PersistentClass *> Classes_t;
	//typedef std::vector<HPtrJunction> Junctions_t;
	//typedef std::vector<bool> IsLoadingNow_t;
	//typedef std::vector<std::ios::pos_type> ObjPositions_t;

	struct ObjData
	{
		PersistentClass *m_class;
		HPtrJunction m_junct;
		bool m_isLoadingNow;
		std::ios::pos_type m_filepos;

		ObjData() : m_class(0), m_isLoadingNow(false), m_filepos(0) {}
	};

	typedef std::vector<ObjData> ObjsData_t;

	TypeSet_t m_typeSet;
	ObjsData_t m_objsData;
	//Classes_t m_classes;
	//Junctions_t m_juncts;
	//IsLoadingNow_t m_isPartiallyLoaded;
	//ObjPositions_t m_objpos;
};

//=====================================================================================//
//                        class ReadSlotImpl : public ReadSlot                         //
//=====================================================================================//
class ReadSlotImpl : public ReadSlot
{
	unsigned int m_index;
	HPtrJunction m_junct;
	Repository *m_orep;
	RepositoryImpl *m_rep;
	unsigned int m_size;
	std::istringstream m_stream;
	mll::io::ibinstream m_in;
	mll::io::ibinstream &m_infile;
	ReadData &m_readData;

public:
	ReadSlotImpl(unsigned int i, Repository *orep, RepositoryImpl *rep, mll::io::ibinstream &in, ReadData &rd)
	:	m_index(i),
		m_junct(rd.m_objsData[i].m_junct),
		m_orep(orep),
		m_rep(rep),
		m_stream(std::ios::binary),
		m_in(m_stream),
		m_readData(rd),
		m_infile(in)
	{
		if(m_junct->expired())
		{
			if(m_readData.m_objsData[m_index].m_isLoadingNow)
			{
				std::ostringstream sstr;
				sstr << "Storage::Repository: Cyclic strong reference detected";
				throw CyclicReferenceLoadException(sstr.str());
			}

			m_readData.m_objsData[m_index].m_isLoadingNow = true;

			if(m_infile.stream().tellg() != m_readData.m_objsData[m_index].m_filepos)
			{
				m_infile.stream().seekg(m_readData.m_objsData[m_index].m_filepos,std::ios::beg);
			}

			in >> m_size;
			std::string data;
			data.resize(m_size);
			in.stream().read(&data[0],m_size);
			m_stream.str(data);
		}
	}

	~ReadSlotImpl()
	{
		m_readData.m_objsData[m_index].m_isLoadingNow = false;
	}

	bool alreadyRestored() const
	{
		return !m_junct->expired();
	}

	virtual HPtrJunction getPtrJunction() { return m_junct; }
	virtual mll::io::ibinstream &getStream() { return m_in; }
	virtual unsigned int getSize() const { return m_size; }

	virtual HPtrJunction restoreWeakReference()
	{
		unsigned int index;
		m_in >> index;
		if(index == 0xFFFFFFFF) return 0;
		return m_readData.m_objsData[index].m_junct;
	}

	virtual HPtrJunction restoreReference()
	{
		unsigned int index;
		m_in >> index;
		if(index == 0xFFFFFFFF) return 0;
		HPtrJunction ptrj = m_readData.m_objsData[index].m_junct;
		if(ptrj->expired())
		{
			ReadSlotImpl slot(index,m_orep,m_rep,m_infile,m_readData);
			m_readData.m_objsData[index].m_class->restore(m_orep,slot);
		}
		return ptrj;
	}

	virtual void *getExternalReference(const std::type_info &ti)
	{
		RepositoryImpl::ExtReferences_t::iterator i = m_rep->m_extRefs.find(ExternalReference(&ti,0));
		assert( i != m_rep->m_extRefs.end() );
		return i->m_ptr;
	}
};

//=====================================================================================//
//                         void RepositoryImpl::store() const                          //
//=====================================================================================//
void RepositoryImpl::store(mll::io::obinstream &out) const
{
	// Сохраним все данные так, чтобы наш новый формат был максимально похож на старый.
	// Обязательно существует возможность все загрузить из старого формата
	Utils::storeTag(out,m_tag);
	Utils::storeVersion(out,g_repositoryVersion);

	// Сохраним список типов также, как и в предыдущей версии
	typedef std::set<class_info,ClassInfoCompare> TypeSet_t;
	TypeSet_t m_typeSet;

	for(Entries_t::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i)
	{
		m_typeSet.insert(i->m_ptr.getType());
	}

	out << m_typeSet.size();

	for(TypeSet_t::iterator i = m_typeSet.begin(); i != m_typeSet.end(); ++i)
	{
		out << i->name();
	}

	// Сохраним объекты
	out << m_entries.size();
	for(Entries_t::const_iterator i = m_entries.begin(); i != m_entries.end();)
	{
		class_info cls = i->m_ptr.getType();
		unsigned int index = std::distance(m_typeSet.begin(),m_typeSet.find(cls));
		unsigned int count = 0;
		while(i != m_entries.end() && cls == i->m_ptr.getType()) { ++i; ++count; }
		out << index << count;
	}

	for(Entries_t::const_iterator i = m_entries.begin(); i != m_entries.end(); ++i)
	{
		WriteSlotImpl slot(this,out);
		i->m_ptr.getObject()->store(slot);
		slot.flush();
	}
}

//=====================================================================================//
//                           void RepositoryImpl::restore0()                           //
//=====================================================================================//
void RepositoryImpl::restore0(mll::io::ibinstream &in, const Version &v, Repository *r)
{
	// Загрузим список типов
	ReadData readData;

	if(v.getMinor() <= 1)
	{
		const std::string tag = Utils::restoreTag(in);
		if(g_typeTableTag != tag)
		{
			std::ostringstream sstr;
			sstr << "Incompatible type table tag (expected [" << g_typeTableTag
				<< "], got from file [" << tag << "])";
			throw WrongTagLoadException(sstr.str());
		}

		Version ver = Utils::restoreVersion(in);
		if(!ver.isCompatible(g_typeTableVersion))
		{
			std::ostringstream sstr;
			sstr << "Incompatible type table version (expected ["
				<< g_typeTableVersion.getMajor() << "." << g_typeTableVersion.getMinor()
				<< "], got from file ["
				<< ver.getMajor() << "." << ver.getMinor()
				<< "])";
			throw WrongVersionLoadException(sstr.str());
		}
	}

	unsigned int tcount;
	in >> tcount;
	readData.m_typeSet.resize(tcount,0);

	for(unsigned int i = 0; i < tcount; ++i)
	{
		std::string cls;
		in >> cls;
		readData.m_typeSet[i] = Registry::instance()->getClass(cls);
	}

	unsigned int obj_count;
	in >> obj_count;
	readData.m_objsData.resize(obj_count);
	//readData.m_juncts.resize(obj_count);
	//readData.m_classes.resize(obj_count,0);

	// Загрузим объекты
	if(v.getMinor() <= 1)
	{
		for(unsigned int i = 0; i < obj_count; ++i)
		{
			readData.m_objsData[i].m_junct = new PtrJunction;
		}

		for(unsigned int i = 0; i < obj_count; ++i)
		{
			unsigned int index;
			in >> index;
			readData.m_objsData[i].m_class = readData.m_typeSet[index];
		}
	}
	else
	{
		for(unsigned int i = 0; i < obj_count; ++i)
		{
			readData.m_objsData[i].m_junct = new PtrJunction;
		}

		for(unsigned int i = 0, j = 0; i < tcount; ++i)
		{
			unsigned int index;
			unsigned int count;
			in >> index >> count;
			for(unsigned int k = j; k < j+count; ++k)
			{
				readData.m_objsData[k].m_class = readData.m_typeSet[index];
			}
			//std::fill(readData.m_classes.begin()+j,readData.m_classes.begin()+j+count,readData.m_typeSet[index]);
			j += count;
		}
	}

	for(unsigned int i = 0; i < obj_count; ++i)
	{
		readData.m_objsData[i].m_filepos = in.stream().tellg();
		unsigned int size;
		in >> size;
		in.stream().seekg(size,std::ios::cur);
	}

	std::ios::pos_type lastpos = in.stream().tellg();

	for(unsigned int i = 0; i < obj_count; ++i)
	{
		ReadSlotImpl slot(i,r,this,in,readData);
		if(!slot.alreadyRestored())
		{
			readData.m_objsData[i].m_class->restore(r,slot);
		}
	}

	in.stream().seekg(lastpos);
}

//=====================================================================================//
//                           void RepositoryImpl::restore()                            //
//=====================================================================================//
void RepositoryImpl::restore(mll::io::ibinstream &in, Repository *r)
{
	clean();

	const std::string tag = Utils::restoreTag(in);
	if(m_tag != tag)
	{
		std::ostringstream sstr;
		sstr << "Incompatible repository tag (expected [" << m_tag
			<< "], got from file [" << tag << "])";
		throw WrongTagLoadException(sstr.str());
	}

	Version ver = Utils::restoreVersion(in);
	switch(ver.getMajor())
	{
		case 0:	restore0(in,ver,r); break;
		default:
		{
			std::ostringstream sstr;
			sstr << "Unknown repository major version " << "[" << ver.getMajor() << "]";
			throw WrongVersionLoadException(sstr.str());
		}
	}
}

//=====================================================================================//
//                                 Storage::Storage()                                  //
//=====================================================================================//
Repository::Repository()
:	m_pimpl(new Impl(g_repositoryTag))
{
}

//=====================================================================================//
//                              Repository::Repository()                               //
//=====================================================================================//
Repository::Repository(const std::string &tag)
:	m_pimpl(new Impl(tag))
{
}

//=====================================================================================//
//                                 Storage::~Storage()                                 //
//=====================================================================================//
Repository::~Repository()
{
	m_pimpl->clean();
}

//=====================================================================================//
//                              void Repository::clear()                               //
//=====================================================================================//
void Repository::clear()
{
	std::string tag = m_pimpl->m_tag;
	m_pimpl.reset(new Impl(tag));
}

//=====================================================================================//
//                              void Storage::addObject()                              //
//=====================================================================================//
void Repository::addObject(const TypedPtr &tp)
{
	Impl::Entries_t::iterator i = m_pimpl->m_entries.insert(StorageEntry(tp)).first;
	Impl::TypeIndex_t::iterator j = std::lower_bound(m_pimpl->m_index.begin(),m_pimpl->m_index.end(),
			i, TypeIndexCompare());
	m_pimpl->m_index.insert(j,i);
}

//=====================================================================================//
//                              void Storage::addObject()                              //
//=====================================================================================//
void Repository::removeObject(void *ptr)
{
	Impl::TypeIndex_t::iterator i = std::lower_bound(m_pimpl->m_index.begin(),m_pimpl->m_index.end(),
			ptr,TypeIndexCompare());
	m_pimpl->m_entries.erase(*i);
	m_pimpl->m_index.erase(i);
}

//=====================================================================================//
//                           void Repository::store() const                            //
//=====================================================================================//
void Repository::store(mll::io::obinstream &out) const
{
	m_pimpl->store(out);
}

//=====================================================================================//
//                             void Repository::restore()                              //
//=====================================================================================//
void Repository::restore(mll::io::ibinstream &in)
{
	m_pimpl->restore(in,this);
}

//=====================================================================================//
//                              void *Storage::allocate()                              //
//=====================================================================================//
void *Repository::allocate(size_t s)
{
	size_t ns = s + sizeof(Repository *);
	Repository **result = reinterpret_cast<Repository **>(m_pimpl->m_alloc.allocate(ns));
	*result = this;
	return result+1;
}

//=====================================================================================//
//                           void *Storage::allocateStatic()                           //
//=====================================================================================//
void *Repository::allocateStatic(size_t s)
{
	size_t ns = s + sizeof(Repository *);
	Repository **result = reinterpret_cast<Repository **>(g_alloc.allocate(ns));
	*result = 0;
	return result+1;
}

//=====================================================================================//
//                          void Storage::deallocateStatic()                           //
//=====================================================================================//
void Repository::deallocateStatic(void *p)
{
	Repository **storage = reinterpret_cast<Repository **>(p);
	if(*(storage-1))
	{
		(*(storage-1))->removeObject(p);
		(*(storage-1))->m_pimpl->m_alloc.deallocate(storage-1);
	}
	else
	{
		g_alloc.deallocate(storage-1);
	}
}

//=====================================================================================//
//                     RepositoryIteratorBase Repository::begin()                      //
//=====================================================================================//
RepositoryIteratorBase Repository::begin(class_info ci)
{
	RepositoryIteratorBase result;
	result.m_pimpl->m_itor = m_pimpl->lower_bound(ci);
	return result;
}

//=====================================================================================//
//                RepositoryConstIteratorBase Repository::begin() const                //
//=====================================================================================//
RepositoryConstIteratorBase Repository::begin(class_info ci) const
{
	RepositoryConstIteratorBase result;
	result.m_pimpl->m_itor = m_pimpl->lower_bound(ci);
	return result;
}

//=====================================================================================//
//                      RepositoryIteratorBase Repository::end()                       //
//=====================================================================================//
RepositoryIteratorBase Repository::end(class_info ci)
{
	RepositoryIteratorBase result;
	result.m_pimpl->m_itor = m_pimpl->upper_bound(ci);
	return result;
}

//=====================================================================================//
//                 RepositoryConstIteratorBase Repository::end() const                 //
//=====================================================================================//
RepositoryConstIteratorBase Repository::end(class_info ci) const
{
	RepositoryConstIteratorBase result;
	result.m_pimpl->m_itor = m_pimpl->upper_bound(ci);
	return result;
}

//=====================================================================================//
//                        std::pair<> Repository::getObjects()                         //
//=====================================================================================//
std::pair<RepositoryIteratorBase,RepositoryIteratorBase> Repository::getObjects(class_info ci)
{
	RepositoryIteratorBase first;
	RepositoryIteratorBase second;
	first.m_pimpl->m_itor = m_pimpl->lower_bound(ci);
	second.m_pimpl->m_itor = m_pimpl->upper_bound(ci);
	return std::make_pair(first,second);
}

//=====================================================================================//
//                     std::pair<> Repository::getObjects() const                      //
//=====================================================================================//
std::pair<RepositoryConstIteratorBase,RepositoryConstIteratorBase> Repository::getObjects(class_info ci) const
{
	RepositoryConstIteratorBase first;
	RepositoryConstIteratorBase second;
	first.m_pimpl->m_itor = m_pimpl->lower_bound(ci);
	second.m_pimpl->m_itor = m_pimpl->upper_bound(ci);
	return std::make_pair(first,second);
}

//=====================================================================================//
//                           void Repository::addReference()                           //
//=====================================================================================//
void Repository::addReference(void *ptr, const std::type_info &ti)
{
	m_pimpl->m_extRefs.insert(ExternalReference(&ti,ptr));
}

//=====================================================================================//
//                         void Repository::removeReference()                          //
//=====================================================================================//
void Repository::removeReference(void *ptr, const std::type_info &ti)
{
	m_pimpl->m_extRefs.erase(ExternalReference(&ti,ptr));
}

}