#if !defined(__NOVTABLE_H_INCLUDED_5992692519605117__)
#define __NOVTABLE_H_INCLUDED_5992692519605117__

// ������� ������, ��� �������� ������ �� vtable ������������ �������
//   ���������: ���������� ������� ������������� ������������� �������,
//              ���������� ������ ����

#if defined(_MSC_VER) || defined(__ICL)
#define ML_NOVTABLE		__declspec(novtable)
#else
#define ML_NOVTABLE
#endif

#endif // !defined(__NOVTABLE_H_INCLUDED_5992692519605117__)