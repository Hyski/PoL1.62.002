#pragma once

#include "Shadow.h"

class DIBData;

namespace Shadows
{

//=====================================================================================//
//                                   struct sc_node                                    //
//=====================================================================================//
struct sc_node
{
	point3		*ptr;
	float		x, y;
	int			xi, yi;
};

//=====================================================================================//
//                                   struct sc_line                                    //
//=====================================================================================//
struct sc_line 
{
	enum {MAXNBRS = 4};
	int			bIdx, lIdx; // ������� ������ �����
	int			tIdx[MAXNBRS];	// ������� �������� �����
	sc_node		*bi, *li;	// ��������� ������ �����
	sc_node		*ti[MAXNBRS];	// ��������� �������� �����
	short int	ct;		// ���-�� ���������
	short int	nbrCount; // ���-�� �������
};

class ShadowAccum;

//=====================================================================================//
//                                 class ComplexShadow                                 //
//=====================================================================================//
class ComplexShadow : public BaseShadow // ������� ����
{
	friend ShadowUtility;
	friend ShadowAccum;

public:
	ComplexShadow(RealEntity *);
	ComplexShadow(BaseShadow *);
	~ComplexShadow();

	void Update(float);							// �������� ����
	bool PoLNeedUpdate(float);

	bool IsComplex() const { return true; }

	// ������� ��� ����
	void AddObject(const TexObject *);			// �������� ������
	void AddObject(SimpleTexturedObject *);		// �������� ������
	void Clear();								// �������� ���������� ��� ���� ��������

	void Draw(GraphPipe *);						// ���������� ����
	void UpdateOnModeChange();

	void LockSurface();
	void UnlockSurface();

private:
	void Calc();						// ����������� �����
	void BlitLinez();					// ���������� ����� � ������ � �������� �����
	void BlitBuffer(unsigned char *, long pitch);	// ���������� ����� � bitmap � �������� �����

	//static DWORD WINAPI Worker(LPVOID);

private:
	DIBData *dbdTexture;	// ���������� ��������

	unsigned int TEXTX;	// Shadow texture width
	unsigned int TEXTY;	// Shadow texture height
	unsigned char TXS;	// TEXTX = 2**TXS

	float d;

	LPDIRECTDRAWSURFACE7 shdwSurface;	// ����������� ��� ����

	short int *buf;		// ����� ��� ������������� ������

	typedef std::vector<sc_node> Nodes_t;
	typedef std::vector<sc_line> Lines_t;

	Nodes_t m_nodes;
	Lines_t m_lines;

	bool bLinez;

	unsigned char *m_lockData;
	long m_lockPitch;
};

}