#include "Vis.h"
#include "VisAnimMgr.h"
#include "VisMeshMgr.h"

namespace Vis
{

static AnimMgr *g_instance = 0;

//=====================================================================================//
//                                 AnimMgr::AnimMgr()                                  //
//=====================================================================================//
AnimMgr::AnimMgr()
:	m_appFront(MeshMgr::instance()->getAppFront())
{
	assert( g_instance == 0 );
	g_instance = this;
}

//=====================================================================================//
//                                 AnimMgr::~AnimMgr()                                 //
//=====================================================================================//
AnimMgr::~AnimMgr()
{
	assert( m_anims.empty() );
	assert( g_instance == this );
	g_instance = 0;
}

//=====================================================================================//
//                            bool AnimMgr::hasAnim() const                            //
//=====================================================================================//
bool AnimMgr::hasAnim(const std::string &n) const
{
	return m_anims.find(n) != m_anims.end();
}

//=====================================================================================//
//                            HMeshAnim AnimMgr::getAnim()                             //
//=====================================================================================//
HMeshAnim AnimMgr::getAnim(const std::string &name)
{
	Anims_t::iterator i = m_anims.find(name);
	if(i == m_anims.end())
	{
		using namespace mll::fs;
		const std::string real_name = "animations/"+name+".maani";

		if(!file_exists(real_name))
		{
			std::ostringstream sstr;
			sstr << "AnimMgr: Unable to load anim [" << name << "]"
				<< " from file [" << real_name << "]";
			throw AnimLoadFailureError(sstr.str());
		}

		try
		{
			std::auto_ptr<i_file> f = m_appFront->openFile("animations/"+name+".maani");
			i = m_anims.insert(std::make_pair(name,new MeshAnim(name,f->stream()))).first;
		}
		catch(mll::debug::exception &e)
		{
			std::ostringstream sstr;
			sstr << "AnimMgr: Unable to load anim [" << name << "]"
				<< " (original message: " << e.what() << ")";
			throw AnimLoadFailureError(sstr.str());
		}

		i->second->enroll();
	}
	return i->second;
}

//=====================================================================================//
//                           HMeshAnim AnimMgr::embedAnim()                            //
//=====================================================================================//
HMeshAnim AnimMgr::embedAnim(const std::string &name, std::istream &in)
{
	assert( m_anims.find(name) == m_anims.end() );

	Anims_t::iterator i;
	try
	{
		i = m_anims.insert(std::make_pair(name,new MeshAnim(name,in))).first;
	}
	catch(mll::debug::exception &e)
	{
		std::ostringstream sstr;
		sstr << "AnimMgr: Unable to embed anim [" << name << "]"
				<< " (original message: " << e.what() << ")";
		throw AnimLoadFailureError(sstr.str());
	}

	return i->second;
}

//=====================================================================================//
//                             void AnimMgr::removeAnim()                              //
//=====================================================================================//
void AnimMgr::removeAnim(const std::string &name)
{
	m_anims.erase(name);
}

//=====================================================================================//
//                            AnimMgr *AnimMgr::instance()                             //
//=====================================================================================//
AnimMgr *AnimMgr::instance()
{
	return g_instance;
}

}