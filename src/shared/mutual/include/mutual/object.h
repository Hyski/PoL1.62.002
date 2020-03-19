#if !defined(__OBJECT_H_INCLUDED_7552370787048739__)
#define __OBJECT_H_INCLUDED_7552370787048739__

#include <mutual/novtable.h>
#include <mutual/rtti.h>

//=====================================================================================//
//                                 class MlRefCounted                                  //
//=====================================================================================//
//! ������� ����� ��� �������� �� ��������� ������
/*! ����� ������������� ����� ����������� ������� �������� ������.
 *  ���� ����� ������������ � ������� ����������� ��������.
 */
class MlRefCounted
{
protected:
	virtual ~MlRefCounted() {}

private:
	mutable unsigned m_refCount;

	// ������� ��������� ����������, ����� ����������� ����� �� ��� �� ��������,
	//   �� �� ����� �������������� �� (www.cuj.com, Conversations: Virtually Yours)
	virtual void load() const { }
	//! ���������� ������
	/*! ��� ���������� ��������� ������ ���� ���������� ��� �������.
	 *  ��������� ��-��������� ����������� � ������ <tt>delete this;</tt>.
	 *  ����������� ������ ����� �������������� ��������� ���� �������, �� �������
	 *  �� ��� �� �����. */
	virtual void unload() const { delete this; }

public:
	MlRefCounted() : m_refCount(0) {}
	MlRefCounted(const MlRefCounted &) : m_refCount(0) {}

	//! \brief �������������� ������� ������
	/*! \return ����� �������� �������� ������ */
	inline void addRef() const { if(!m_refCount++) load(); }

	//! \brief �������������� ������� ������
	/*! � ������, ���� ������� ������ ��������� ����, �� ����� �������
	 *  ������� \link MlObject::unload() \endlink
	 *  \return ����� �������� �������� ������ */
	inline void release() const { if(!--m_refCount) unload(); }

	//! \brief ���������� ������� ������
	inline unsigned int getRefCount() const { return m_refCount; }

	MlRefCounted &operator=(const MlRefCounted &rc) { return *this; }
};

//=====================================================================================//
//                                   class MlObject                                    //
//=====================================================================================//
//! ����� ��������� ��� ������� � ����������� � ���� � ��������� ������
class ML_NOVTABLE MlObject : public MlRefCounted
{
public:
	ML_RTTI_ROOT(MlObject);

protected:
	MlObject() {}
	virtual ~MlObject() {}
};

#endif // !defined(__OBJECT_H_INCLUDED_7552370787048739__)