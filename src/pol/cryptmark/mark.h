#pragma once

namespace SecurityMark
{
/// �������� ����� ���������� ��� ������ Mark
MLL_MAKE_EXCEPTION(MarkException,mll::debug::exception);
	/// ����������: �� ������ ������ (� �����)
	MLL_MAKE_EXCEPTION( CanNotFindMark_MarkException, MarkException);
	/// ����������: �� ������� ����� ��� ������� (� �����)
	MLL_MAKE_EXCEPTION( CanNotFindPlaceForMark_MarkException, MarkException);
	/// ����������: ���� �� ����������
	MLL_MAKE_EXCEPTION( FileNotMarked_MarkException, MarkException);

/// ����� - ������������ ��� ���������� ������
class Mark
{
private:
/// ������ �����, ��� ���������� ������/������ ������
	static const int FILE_BLOCK  = 512;

public:
/// ������������ ����� ������� ��� ��������� ���������� (�� ��������� - 256)
	static const int MAX_MARK_LEN = 128;

/// ������������ ����� ������ � GUID'��
	static const int MAX_GUID_STR_LEN = 39;

/// ������������ ����� ������ � ������ ����������
	static const int MAX_COMP_STR_LEN = MAX_COMPUTERNAME_LENGTH+1;

/// �����(����) - ������������� ������� � �����
	static const std::string MARK_KEY;

/// ��������� ������
	static const std::string MARK_DEF;

/// ������ ��� ���������� ������� xor'�� - 128 ����
	static const std::string CRYPT_STR;

///@{ ��������� ��� �������� �������
	struct MarkData
	{
		/// ���������������� �����������
		/** ������������ ����� ������ - MAX_MARK_LEN */
		char m_label [MAX_MARK_LEN    ];

		/// ������ � GUID'��
		/** ������������ ����� ������ - MAX_GUID_STR_LEN */
		char m_guid  [MAX_GUID_STR_LEN];

		/// ��� ���������� �� ������� ���� �������
		/** ������������ ����� ������ - MAX_COMP_STR_LEN */
		char m_comp  [MAX_COMP_STR_LEN];

		/// ���� �������� �������
		time_t m_date;
	};
/*@}*/

/// ����������� - ��������� ������ ������������ ��� �������� ����� � �����
	Mark(std::istream & fin);

/// ����������� - ��������� ������ ������������ ��� ������������� ������� ���������������� � ��������� �����������
	Mark(const std::string & mrk);

/// ������� ������ ������� � �������� ����(fout)
	/** ������� ���������:
	  * - fin - ������� �����
	  * - fout - �������� �����
	  * - stream - <true> ���� ������� � �������� ������ - ���� � ����� ���� */
	void setMark(std::istream & fin,std::ostream & fout,bool stream);

/// ������� �������� ��������� � ��������
	const MarkData & getMark() const;

private:
/// ���������/����������� �������
	void crypt();

/// ����� � ����� (fin) �������� �������
	/** ������������ ��������:
	    - 0 - ����� �������
	    - 1 - ����� �� ������� */
	bool findMarkPos(std::istream & fin);

/// ��������� ������ � GUID'��
	static std::string genGuidString();

/// ��������� �������� ������� � �����
	std::streampos	m_FileMarkPos;

/// ���������� � ��������
	MarkData	m_Mark;	
};

}