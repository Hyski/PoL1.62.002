#include "Vis.h"

namespace Vis
{

ML_PERSISTENT_IMPL2(Material,VIS_BASIC_PTAG);

//=====================================================================================//
//                                Material::Material()                                 //
//=====================================================================================//
Material::Material()
:	m_name("**wrong_material_name**"),
	m_body("&^%$#?Wrong material Body;;;;")
{
}

//=====================================================================================//
//                                Material::Material()                                 //
//=====================================================================================//
Material::Material(const std::string &name, const OptionSet &options)
:	m_name(name),
	m_options(options)
{
}

//=====================================================================================//
//                                Material::Material()                                 //
//=====================================================================================//
Material::Material(Storage::ReadSlot &slot)
:	Storage::PersistentBase(slot)
{
	slot.getStream() >> m_name;
	slot.getStream() >> m_body;
	m_options.restore(slot.getStream());
}

//=====================================================================================//
//                                Material::Material()                                 //
//=====================================================================================//
Material::Material(const Material &mat)
:	m_name(mat.getName()),
	m_body(mat.getBody()),
	m_options(mat.getOptions())
{
}

//=====================================================================================//
//                                Material::~Material()                                //
//=====================================================================================//
Material::~Material()
{
}

//=====================================================================================//
//                               void Material::store()                                //
//=====================================================================================//
void Material::store(Storage::WriteSlot &slot) const
{
	slot.getStream() << getName();
	slot.getStream() << getBody();
	m_options.store(slot.getStream());
}

}