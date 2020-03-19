#pragma once

namespace PoL
{

class ActFactory;

//=====================================================================================//
//                                  class ActTemplate                                  //
//=====================================================================================//
class ActTemplate : public MlRefCounted
{
	friend class ActFactory;

public:
	typedef std::vector<HAct> Acts_t;
	typedef boost::function<HAct (Acts_t &)> ActCreator_t;

private:
	typedef mll::utils::lcrn_tree<ActCreator_t> ActTree_t;
	ActTree_t m_creators;

public:
	typedef ActTree_t::iterator Iterator_t;

public:
	/// Возвращает итератор на запоследний ActCreator_t
	Iterator_t end() { return m_creators.end(); }
	/// Добавляет ActCreator_t в указанную позицию в качестве дочернего узла
	Iterator_t addActCreator(const ActCreator_t &, const Iterator_t &);
	/// Создать акт
	HAct createAct();

private:
	HAct recursiveCreateAct(const Iterator_t &);
};

}