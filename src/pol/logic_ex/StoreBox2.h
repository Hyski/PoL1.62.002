#pragma once

namespace PoL
{

//=====================================================================================//
//                                      class Box                                      //
//=====================================================================================//
class StoreBox2
{
	typedef BaseThing *HThing;
	typedef std::list<HThing> Things_t;

	std::string m_fullId;
	Things_t m_things;
	float m_maxWeight;
	float m_curWeight;

public:
	typedef Things_t::const_iterator Iterator_t;

public:
	StoreBox2();
	StoreBox2(const std::string &fullId, float maxWeight);
	~StoreBox2();

	/// ���������� ������ ��������� ���
	const std::string &getFullId() const { return m_fullId; }
	/// ���������� ������������ ��� ��������� � �����
	float getMaxWeight() const { return m_maxWeight; }
	/// ���������� ������� ��� ��������� � �����
	float getCurWeight() const { return m_curWeight; }

	Iterator_t thingsBegin() const;
	Iterator_t thingsEnd() const;

	/// �������� ������� � ����
	void add(BaseThing *);
	/// ������ ������� �� �����
	void remove(BaseThing *);

	/// ��������� ��������� ������� � �����
	void store(mll::io::obinstream &) const;
	/// ������������ ��������� ������� �� ������
	void restore(mll::io::ibinstream &);

	bool operator<(const StoreBox2 &box) const { return m_fullId < box.m_fullId; }

private:
	void updateCurWeight();
};

typedef boost::shared_ptr<StoreBox2> HStoreBox2;

}
