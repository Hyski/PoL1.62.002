#include "Vis.h"

namespace Vis
{

ML_PERSISTENT_IMPL2(Node,VIS_BASIC_PTAG);

//=====================================================================================//
//                                    Node::Node()                                     //
//=====================================================================================//
Node::Node()
:	m_name("**wrong_node_name**"),
	m_parent(0),
	m_preRotate(matrix3::identity()),
	m_preTranslate(matrix3::identity()),
	m_rotation(quaternion::identity()),
	m_translation(mll::algebra::null_vector),
	m_postTransform(matrix3::identity())
{
}

//=====================================================================================//
//                                    Node::Node()                                     //
//=====================================================================================//
Node::Node(const Node &n)
:	m_name(n.getName()),
	m_parent(n.getParent()),
	m_options(n.getOptions()),
	m_preRotate(n.m_preRotate),
	m_preTranslate(n.m_preTranslate),
	m_rotation(n.m_rotation),
	m_translation(n.m_translation),
	m_postTransform(n.m_postTransform)
{
}

//=====================================================================================//
//                                    Node::Node()                                     //
//=====================================================================================//
Node::Node(Storage::ReadSlot &slot)
:	Storage::PersistentBase(slot)
{
	slot.getStream() >> m_name;
	m_parent = slot.restoreWeakReference<Node>();
	m_options.restore(slot.getStream());
	slot.getStream() >> m_preRotate;
	slot.getStream() >> m_preTranslate;
	slot.getStream() >> m_rotation;
	slot.getStream() >> m_translation;
	slot.getStream() >> m_postTransform;
}

//=====================================================================================//
//                                    Node::Node()                                     //
//=====================================================================================//
Node::Node(const std::string &name, Node *parent, const OptionSet &options)
:	m_name(name),
	m_parent(parent),
	m_options(options),
	m_preRotate(matrix3::identity()),
	m_preTranslate(matrix3::identity()),
	m_rotation(quaternion::identity()),
	m_translation(mll::algebra::null_vector),
	m_postTransform(matrix3::identity())
{
}

//=====================================================================================//
//                                    Node::~Node()                                    //
//=====================================================================================//
Node::~Node()
{
}

//=====================================================================================//
//                                 void Node::store()                                  //
//=====================================================================================//
void Node::store(Storage::WriteSlot &slot) const
{
	slot.getStream() << m_name;
	slot.storeReference(m_parent);
	m_options.store(slot.getStream());
	slot.getStream() << m_preRotate;
	slot.getStream() << m_preTranslate;
	slot.getStream() << m_rotation;
	slot.getStream() << m_translation;
	slot.getStream() << m_postTransform;
}

//=====================================================================================//
//                       matrix3 Node::getTransformation() const                       //
//=====================================================================================//
matrix3 Node::getTransformation() const
{
	return m_preRotate
		* m_rotation.as_matrix()
		* m_preTranslate
		* mll::algebra::translate(m_translation)
		* m_postTransform;
}

//=====================================================================================//
//                       matrix3 Node::getTransformation() const                       //
//=====================================================================================//
matrix3 Node::getTransformation(const quaternion &rot, const vector3 &trans) const
{
	return m_preRotate
		* (m_rotation * rot).as_matrix()
		* m_preTranslate
		* mll::algebra::translate(m_translation + trans)
		* m_postTransform;
}

//=====================================================================================//
//                     matrix3 Node::getTransformationPure() const                     //
//=====================================================================================//
matrix3 Node::getTransformationPure(const quaternion &rot, const vector3 &trans) const
{
	return m_preRotate
		* rot.as_matrix()
		* m_preTranslate
		* mll::algebra::translate(trans)
		* m_postTransform;
}

}