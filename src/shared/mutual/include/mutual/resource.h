#if !defined(__RESOURCE_H_INCLUDED_7552370787048739__)
#define __RESOURCE_H_INCLUDED_7552370787048739__

#include <mll/debug/exception.h>
#include <mutual/resourcemgr.h>
#include <mutual/novtable.h>
#include <mutual/object.h>
#include <mutual/handle.h>
#include <boost/utility.hpp>
#include <boost/format.hpp>

//=====================================================================================//
//                                  resource_failure                                   //
//=====================================================================================//
MLL_MAKE_EXCEPTION(resource_failure,mll::debug::exception);

//=====================================================================================//
//                                  class MlResource                                   //
//=====================================================================================//
class ML_NOVTABLE MlResource : public MlObject, private boost::noncopyable
{
public:

	typedef MlResourceMgr::container_t::iterator handle_t;

private:

	MlResourceMgr::container_t::iterator m_handle;		//	���������� ������� � ������ ���������
	std::string m_name;									//	��������� ������������� ������� (�� ��������)
	bool m_enrolled;									//	���� ����, ��� ������ ��������������� � ������ ���������

protected:
	MlResource();
	MlResource(const std::string& name);
	virtual ~MlResource();
public:
	//	�������� ������������� �������
	const std::string& name(void) const;
	//	���������������� ������ � ������ ���������
	void enroll(void);

public:

	ML_RTTI(MlResource,MlObject);

};

inline MlResource::MlResource() : m_enrolled(false)
{
}

inline MlResource::MlResource(const std::string& name) : m_name(name),m_enrolled(false)
{
}

inline MlResource::~MlResource()
{
	if(m_enrolled) MlResourceMgr::instance()->removeResource(m_handle);
}

//	�������� ������������� �������
inline const std::string& MlResource::name(void) const
{
	return m_name;
}

//	���������������� ������ � ������ ���������
inline void MlResource::enroll(void)
{
	if(m_enrolled) throw resource_failure((boost::format("������ ���� [%1%] � ������ [%2%] ��� ���������������!") % rtti_dynamic().std_type_info().name() % name()).str());
	m_handle = MlResourceMgr::instance()->insertResource(this);
	m_enrolled = true;
}

//=====================================================================================//
//                          struct MlResourceComparePredicate                          //
//=====================================================================================//
//	�������� ��� ���������� ������, ���������� ��� ������� � ��������� ���������������
/*struct MlResourceComparePredicate : public std::binary_function <MlResource*,MlResource*,bool>
{
	bool operator()(const MlResource* value,const std::string& name) { return value->name() < name; }
	bool operator()(const std::string& name,const MlResource* value) { return name < value->name(); }
};*/

#endif // !defined(__RESOURCE_H_INCLUDED_7552370787048739__)