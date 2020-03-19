#if !defined(__VIS_OPTION_H_INCLUDED_4716722768415123__)
#define __VIS_OPTION_H_INCLUDED_4716722768415123__

#include "VisOptionHelpers.h"

//=====================================================================================//
//                                   namespace Masce                                   //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//       class ML_NOVTABLE Option : private boost::noncopyable, public MlObject        //
//=====================================================================================//
class ML_NOVTABLE Option : private boost::noncopyable, public MlObject
{
	OptionType m_type;

	virtual void store(mll::io::obinstream &) = 0;
	virtual void restore(mll::io::ibinstream &) = 0;

public:
	ML_RTTI(Option,MlObject);

	Option(OptionType t) : m_type(t) {}
	virtual ~Option() {}

	virtual HOption clone() const = 0;
	OptionType getType() const { return m_type; }

#define VIS_GEN_GET_SET(A)	\
	virtual void set##A(OptionTraits<ot##A>::reference_type) { throw WrongType(); }	\
	virtual OptionTraits<ot##A>::reference_type get##A() const { throw WrongType(); }	
	VIS_FOREACH_PARAMETER_TYPE(VIS_GEN_GET_SET)
#undef VIS_GEN_GET_SET

	static HOption restoreOption(mll::io::ibinstream &in);
	static void storeOption(HOption, mll::io::obinstream &);

	virtual void showSelf(std::ostream &) const = 0;

	virtual bool compare(const Option &) const = 0;
};

//=====================================================================================//
//                        class ConcreteOption : public Option                         //
//=====================================================================================//
template<unsigned int I>
class ConcreteOption : public Option
{
public:
	typedef OptionTraits<I> traits;
	typedef typename traits::value_type value_type;
	typedef typename traits::reference_type reference_type;
	typedef value_type &non_const_reference_type;

public:
	ML_RTTI(ConcreteOption,Option);

	ConcreteOption() : Option((OptionType)I) {}
	ConcreteOption(reference_type v) : Option((OptionType)I), m_value(v) {}

	virtual HOption clone() const { return new ConcreteOption(m_value); }
	non_const_reference_type value() { return m_value; }
	reference_type value() const { return m_value; }

	ConcreteOption &operator=(reference_type v) { m_value = v; return *this; }

#define VIS_GEN_GET_SET(A)	\
	virtual void set##A(OptionTraits<ot##A>::reference_type) { throw WrongType(); }	\
	virtual OptionTraits<ot##A>::reference_type get##A() const { throw WrongType(); }	
	VIS_FOREACH_PARAMETER_TYPE(VIS_GEN_GET_SET)
#undef VIS_GEN_GET_SET

	virtual bool compare(const Option &opt) const
	{
		if(rtti_dynamic() != opt.rtti_dynamic()) return false;
		const ConcreteOption &co = static_cast<const ConcreteOption &>(opt);
		return m_value == co.m_value;
	}

private:
	value_type m_value;

	virtual void store(mll::io::obinstream &out) { out << m_value; }
	virtual void restore(mll::io::ibinstream &in) { in >> m_value; }
	virtual void showSelf(std::ostream &out) const { out << m_value; }
};

#define VIS_MAKE_PARAMETER(A)	\
	template<> void ConcreteOption<ot##A>::set##A(traits::reference_type v) { m_value = v; }	\
	template<> OptionTraits<ot##A>::reference_type ConcreteOption<ot##A>::get##A() const { return m_value; }	\
	typedef ConcreteOption<ot##A> Option##A;	\
	typedef MlObjHandle< Option##A > HOption##A;
	VIS_FOREACH_PARAMETER_TYPE(VIS_MAKE_PARAMETER)
#undef VIS_MAKE_PARAMETER

}

#endif // !defined(__VIS_OPTION_H_INCLUDED_4716722768415123__)