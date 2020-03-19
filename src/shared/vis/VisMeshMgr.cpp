#include "Vis.h"
#include "VisMeshMgr.h"

namespace Vis
{

static MeshMgr *g_instance = 0;

//=====================================================================================//
//                                 MeshMgr::MeshMgr()                                  //
//=====================================================================================//
MeshMgr::MeshMgr(AppFront *front)
:	m_appFront(front)
{
	assert( g_instance == 0 );
	g_instance = this;
}

//=====================================================================================//
//                                 MeshMgr::~MeshMgr()                                 //
//=====================================================================================//
MeshMgr::~MeshMgr()
{
	assert( g_instance == this );
	g_instance = 0;
}

//=====================================================================================//
//                            bool MeshMgr::hasMesh() const                            //
//=====================================================================================//
bool MeshMgr::hasMesh(const std::string &n) const
{
	return m_meshes.find(n) != m_meshes.end();
}

//=====================================================================================//
//                              HMesh MeshMgr::getMesh()                               //
//=====================================================================================//
HMesh MeshMgr::getMesh(const std::string &name)
{
	Meshes_t::iterator i = m_meshes.find(name);
	if(i == m_meshes.end())
	{
		try
		{
			i = m_meshes.insert(std::make_pair(name,new Mesh(name,m_appFront,0))).first;
		}
		catch(mll::debug::exception &e)
		{
			std::ostringstream sstr;
			sstr << "MeshMgr: Unable to load mesh [" << name << "]"
				<< " (original message: " << e.what() << ")";
			throw MeshLoadFailureError(sstr.str());
		}

		i->second->enroll();
	}
	return i->second;
}

//=====================================================================================//
//                             HMesh MeshMgr::embedMesh()                              //
//=====================================================================================//
HMesh MeshMgr::embedMesh(const std::string &name, std::istream &in, HObject ssl)
{
	assert( m_meshes.find(name) == m_meshes.end() );

	Meshes_t::iterator i;
	try
	{
		i = m_meshes.insert(std::make_pair(name,new Mesh(name,in,m_appFront,ssl))).first;
	}
	catch(mll::debug::exception &e)
	{
		std::ostringstream sstr;
		sstr << "MeshMgr: Unable to embed mesh [" << name << "]"
				<< " (original message: " << e.what() << ")";
		throw MeshLoadFailureError(sstr.str());
	}

	i->second->enroll();
	return i->second;
}

//=====================================================================================//
//                             void MeshMgr::removeMesh()                              //
//=====================================================================================//
void MeshMgr::removeMesh(const std::string &name)
{
	m_meshes.erase(name);
}

//=====================================================================================//
//                            MeshMgr *MeshMgr::instance()                             //
//=====================================================================================//
MeshMgr *MeshMgr::instance()
{
	return g_instance;
}

}