#ifndef _UTILS_H_
#define _UTILS_H_

#define RELEASE(object) {if(object) {object->Release(); object = NULL;}}
#define DESTROY(object) {if(object) {delete object; object = NULL;}}
#define FREE(object)    {if(object) {delete[] object; object = NULL;}}

// ��� ����������� ����������� �������
// !!! ����������� ������� �.�. �������� � ����� ������������� !!!
#define DIMOF(x) (sizeof(x)/sizeof(x[0]))

#ifdef PI
#undef PI
#endif

//��������� � pi
const float PI   = 3.1415927f;
const float PIm2 = 6.2831853f;
const float PId2 = 1.5707963f;
const float PId3 = 1.0471976f;
const float PId4 = 0.7853982f;

//�������� 
const float epsilon = 0.001f;

//��������� � cos, sin, tg
const float COS30   = 0.86602540f;
const float COS30m2 = 1.73205081f;
const float COS60   = 0.5f;
const float SIN60   = 0.86602540f;
const float TAN30   = 0.57735027f;

//���, ������� & etc.
const point3 AXISX(1,0,0);    //��� �
const point3 AXISY(0,1,0);    //��� Y
const point3 AXISZ(0,0,1);    //��� Z
const point3 NULLVEC(0,0,0);  //������� ������

struct _D3DMATRIX;
extern const _D3DMATRIX *IDENTITYMAT;

#define TORAD(x) (PI*(x)/180)
#define TODEG(x) (180*(x)/PI)
#define TODXVECTOR(x)  (*((D3DVECTOR*)(&x)))


//	-------------------- ���������� �������� � �������� ��������� -----------
template <class T>
inline T saturate(const T& value,const T& min,const T& max)
{
	if(value<min)
		return min;
	if(value>max)
		return max;
	return value;
}
//	-------------------------------------------------------------------------
float __fastcall FastAbs(float a);
float __fastcall FastSin(float a);
float __fastcall FastCos(float a);
float __stdcall FastSqrt(float a);
float __stdcall FastMul(float a,float b);
float __stdcall FastDiv(float a,float b);
float __stdcall FPUDist(float pnt1,float pnt2);

float frand(); //���������� ��������������� ����� � ��������� 0.f .. 1.f
BOOL MungeFPCW(WORD *pwOldCW );
void RestoreFPCW(WORD wSave);
void LoadString(FILE *f, std::string *s);
void SaveString(FILE *f, const std::string &s);
void LoadPascalString(FILE *f, std::string *s,bool lower=true);
void SavePascalString(FILE *f, const std::string &s);

void print(char *Template,...);
void status(char *Template,...);
#pragma warning(disable:4035)
inline int GetBit(unsigned Bit)
  {
    _asm
      {
      xor ecx,ecx
      bsf eax,Bit
      cmove eax,ecx
      };
  };
inline SavSlot& operator << (SavSlot& st, const point3& val)
{
    st<<val.x<<val.y<<val.z;    
    return st;
}

inline SavSlot& operator >> (SavSlot& st, point3& val)
{
    st>>val.x>>val.y>>val.z;    
    return st;
}
	 
class ProgressBar
{
private:
	int StringLen;
	int tick_cnt;
public:
	ProgressBar():StringLen(0),tick_cnt(0){};
	void print(char *Template,...)
	{
		while(StringLen){::print("\b \b");--StringLen;}
  char r[255];
  va_list ap;
  va_start(ap,Template);
  vsprintf(r, Template, ap);
  ::print("%s",r);
	StringLen=strlen(r);
	};
	void flush(){StringLen=0;};
	void clear(){while(StringLen){::print("\b \b");--StringLen;}};
  void nl(){::print("\n");StringLen=0;};
  void tick();
};

//�� ������ ��������� ��� �������
std::string KillSpaces(const std::string &data);

//
// "����������" ������ printf
//
std::string mlprintf(const char* str, ...);

// ������� ��� ��������� ���������, ��������� � ������ �����������
// ��� ������� ������ �� ��������� � ���� ����� ���������� ������� �� ������,
//   � ������� ���� ������������� ���������
#if !defined(__SMART_MESSAGE_MACROS__)
#define MACRO_TO_STR(str)			#str
#define	MACRO_TO_STR2(str)			MACRO_TO_STR(str)
#define	MACRO_PLACE_STAMP			__FILE__"("MACRO_TO_STR2(__LINE__)")"
#define SMART_MESSAGE(msg)			message(MACRO_PLACE_STAMP" : "msg)
#define	SMART_WARNING(msg)			SMART_MESSAGE("warning [Paradise Cracked]: "msg)
#define	SMART_ERROR(msg)			SMART_MESSAGE("error [Paradise Cracked]: "msg)
#define __SMART_MESSAGE_MACROS__
#endif

#endif	//_UTILS_H_
