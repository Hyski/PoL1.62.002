#include "logicdefs.h"

#include "Graphic.h"
#include "PathShowDrv.h"

//=====================================================================================//
//                             PathShowDrv::PathShowDrv()                              //
//=====================================================================================//
PathShowDrv::PathShowDrv(const pnt_vec_t& path, Driver* drv)
:	m_path(path),
	DecoratorDrv(drv),
	m_firstTime(true)
{
	if(m_path.size())
	{
		ShowPath();
	}
}

//=====================================================================================//
//                             PathShowDrv::~PathShowDrv()                             //
//=====================================================================================//
PathShowDrv::~PathShowDrv()
{ 
	GraphGrid::GetInst()->Show(GraphGrid::HT_PATH, false);
}

//=====================================================================================//
//                      activity_command PathShowDrv::CalcStep()                       //
//=====================================================================================//
activity_command PathShowDrv::CalcStep(Activity* activity, activity_command user_cmd)
{
	if(!m_firstTime)
	{
		if(m_path.size())
		{
			m_path.pop_back();
			ShowPath();
		}
	}

	m_firstTime = false;

	return DecoratorDrv::CalcStep(activity, user_cmd);
}    

//=====================================================================================//
//                               bool PathShowDrv::Run()                               //
//=====================================================================================//
bool PathShowDrv::Run(Activity* activity, bool result)
{
	if(!result) GraphGrid::GetInst()->Show(GraphGrid::HT_PATH, false);
	return DecoratorDrv::Run(activity, result);
}

//=====================================================================================//
//                            void PathShowDrv::ShowPath()                             //
//=====================================================================================//
void PathShowDrv::ShowPath()
{
	GraphGrid::GetInst()->SetHexes(GraphGrid::HT_PATH, m_path);
	GraphGrid::GetInst()->Show(GraphGrid::HT_PATH, true);
}
