#if !defined(__VIS_NODE_H_INCLUDED_4479592412517698__)
#define __VIS_NODE_H_INCLUDED_4479592412517698__

#include "VisOptionSet.h"

//=====================================================================================//
//                                   namespace Masce                                   //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4251 4275)

//=====================================================================================//
//                                     class Node                                      //
//=====================================================================================//
class VIS_IMPORT Node : public Storage::PersistentBase
{
	std::string m_name;
	Storage::Ptr<Node> m_parent;

	matrix3 m_preRotate, m_preTranslate, m_postTransform;
	quaternion m_rotation;
	vector3 m_translation;

	OptionSet m_options;

public:
	ML_RTTI_CUSTOM_SINGLE(Node,"[Vis.Node]");
	ML_PERSISTENT2(Node,VIS_BASIC_PTAG);

	Node();
	Node(const Node &);
	Node(Storage::ReadSlot &);
	Node(const std::string &, Node *parent, const OptionSet & = OptionSet());
	~Node();

	void setName(const std::string &name) { m_name = name; }

	bool hasParent() const { return !m_parent.expired(); }
	Node *getParent() const { return hasParent()?m_parent.get():0; }
	const std::string &getName() const { return m_name; }
	const OptionSet &getOptions() const { return m_options; }

	const matrix3 &getPreRotate() const { return m_preRotate; }
	const matrix3 &getPreTranslate() const { return m_preTranslate; }
	const quaternion &getRotation() const { return m_rotation; }
	const vector3 &getTranslation() const { return m_translation; }
	const matrix3 &getPostTransform() const { return m_postTransform; }

	void setPreRotate(const matrix3 &m) { m_preRotate = m; }
	void setPreTranslate(const matrix3 &m) { m_preTranslate = m; }
	void setRotation(const quaternion &m) { m_rotation = m; }
	void setTranslation(const vector3 &m) { m_translation = m; }
	void setPostTransform(const matrix3 &m) { m_postTransform = m; }

	matrix3 getTransformation() const;
	matrix3 getTransformation(const quaternion &rot, const vector3 &trans) const;
	matrix3 getTransformationPure(const quaternion &rot, const vector3 &trans) const;

	void store(Storage::WriteSlot &) const;
};

#pragma warning(pop)

ML_PERSISTENT_HIMPL2(Node,VIS_BASIC_PTAG);

}

#endif // !defined(__VIS_NODE_H_INCLUDED_4479592412517698__)