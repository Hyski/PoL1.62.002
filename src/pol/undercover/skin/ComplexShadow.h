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
	int			bIdx, lIdx; // Индексы концов линии
	int			tIdx[MAXNBRS];	// Индексы соседних точек
	sc_node		*bi, *li;	// Указатели концов линии
	sc_node		*ti[MAXNBRS];	// Указатели соседних точек
	short int	ct;		// кол-во отрисовок
	short int	nbrCount; // кол-во соседей
};

class ShadowAccum;

//=====================================================================================//
//                                 class ComplexShadow                                 //
//=====================================================================================//
class ComplexShadow : public BaseShadow // сложная тень
{
	friend ShadowUtility;
	friend ShadowAccum;

public:
	ComplexShadow(RealEntity *);
	ComplexShadow(BaseShadow *);
	~ComplexShadow();

	void Update(float);							// Обновить тень
	bool PoLNeedUpdate(float);

	bool IsComplex() const { return true; }

	// Объекты для тени
	void AddObject(const TexObject *);			// Добавить объект
	void AddObject(SimpleTexturedObject *);		// Добавить объект
	void Clear();								// Очистить информацию обо всех объектах

	void Draw(GraphPipe *);						// Отрисовать тень
	void UpdateOnModeChange();

	void LockSurface();
	void UnlockSurface();

private:
	void Calc();						// Пересчитать линии
	void BlitLinez();					// Нарисовать линии в буфере и очистить линии
	void BlitBuffer(unsigned char *, long pitch);	// Отобразить буфер в bitmap и очистить буфер

	//static DWORD WINAPI Worker(LPVOID);

private:
	DIBData *dbdTexture;	// Содержимое текстуры

	unsigned int TEXTX;	// Shadow texture width
	unsigned int TEXTY;	// Shadow texture height
	unsigned char TXS;	// TEXTX = 2**TXS

	float d;

	LPDIRECTDRAWSURFACE7 shdwSurface;	// Поверхность для тени

	short int *buf;		// буфер для промежуточных данных

	typedef std::vector<sc_node> Nodes_t;
	typedef std::vector<sc_line> Lines_t;

	Nodes_t m_nodes;
	Lines_t m_lines;

	bool bLinez;

	unsigned char *m_lockData;
	long m_lockPitch;
};

}