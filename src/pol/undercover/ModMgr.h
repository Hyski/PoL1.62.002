//=====================================================================================//
//                                                                                     //
//                                  Paradise Cracked                                   //
//                                                                                     //
//                             Copyright by MiST land 2002                             //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Description:                                                                      //
//                                                                                     //
//   -------------------------------------------------------------------------------   //
//   Author: Pavel A.Duvanov                                                           //
//   Date:   29.11.2002                                                                //
//                                                                                     //
//=====================================================================================//
#ifndef __MOD_MGR_H_INCLUDED_5211432865184138__
#define __MOD_MGR_H_INCLUDED_5211432865184138__

//=====================================================================================//
//                                      class Mod                                      //
//=====================================================================================//
class Mod
{
private:

	std::string m_author;			//	��� ������
	std::string m_name;				//	������������ ����
	std::string m_desc;				//	�������� ����
	std::string m_id;				//	������������� ����
	bool m_isValid;					//	���� ���������� ����

public:
	Mod(const std::string& id);
	~Mod();
public:
	//	�������� �� ����������� ����
	bool isValid(void)  const;
	//	�������� ��� ������
	const std::string& author(void) const;
	//	�������� ������������ ����
	const std::string& name(void) const;
	//	�������� �������� ����
	const std::string& desc(void) const;
	//	�������� ������������� ����
	const std::string& id(void) const;

};

//	�������� �� ����������� ����
inline bool Mod::isValid(void) const
{
	return m_isValid;
}
//	�������� ��� ������
inline const std::string& Mod::author(void) const
{
	return m_author;
}
//	�������� ������������ ����
inline const std::string& Mod::name(void) const
{
	return m_name;
}
//	�������� �������� ����
inline const std::string& Mod::desc(void) const
{
	return m_desc;
}
//	�������� ������������� ����
inline const std::string& Mod::id(void) const
{
	return m_id;
}

//=====================================================================================//
//                                    class ModMgr                                     //
//=====================================================================================//
class ModMgr
{
public:

	static const std::string m_modPath;			//	���� �� �������� � ������
	static const std::string m_modInfo;			//	������������ ������� � ��������� ����

	typedef std::vector<Mod*> mods_t;

private:

	mods_t m_mods;								//	������ ��������� �����

public:
	ModMgr();
	~ModMgr();
public:
	//	�������� ������ �������� �����
	const mods_t& mods(void) const;

};

//	�������� ������ �������� �����
inline const ModMgr::mods_t& ModMgr::mods(void) const
{
	return m_mods;
}


#endif //__MOD_MGR_H_INCLUDED_5211432865184138__
