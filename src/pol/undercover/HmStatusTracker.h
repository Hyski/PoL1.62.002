#pragma once

namespace PoL
{

//=====================================================================================//
//                                class HmStatusTracker                                //
//=====================================================================================//
class HmStatusTracker
{
	int m_left, m_right, m_top, m_bottom;
	bool m_oldvis;

public:
	HmStatusTracker();

	/// Возвращает true, если необходимо проявить активность
	bool needActivities();
};

}
