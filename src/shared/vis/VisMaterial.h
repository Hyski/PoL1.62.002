#if !defined(__VIS_MATERIAL_H_INCLUDED_4889377598778046__)
#define __VIS_MATERIAL_H_INCLUDED_4889377598778046__

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4251 4275)

//=====================================================================================//
//                                   class Material                                    //
//=====================================================================================//
class VIS_IMPORT Material : public Storage::PersistentBase
{
	std::string m_name;
	std::string m_body;
	OptionSet m_options;

public:
	ML_RTTI_CUSTOM_SINGLE(Material,"[Vis.Material]");
	ML_PERSISTENT2(Material,VIS_BASIC_PTAG);

	Material();
	Material(const Material &);
	Material(Storage::ReadSlot &);
	Material(const std::string &, const OptionSet & = OptionSet());
	~Material();

	const std::string &getName() const { return m_name; }
	const std::string &getBody() const { return m_body; }
	void setBody(const std::string &body) { m_body = body; }

	const OptionSet &getOptions() const { return m_options; }

	void store(Storage::WriteSlot &) const;
};

#pragma warning(pop)

ML_PERSISTENT_HIMPL2(Material,VIS_BASIC_PTAG);

}

#endif // !defined(__VIS_MATERIAL_H_INCLUDED_4889377598778046__)