#if !defined(AFX_VSHADER_H__1F984AC1_EAB9_11D3_81EA_0060520A50A9__INCLUDED_)
#define AFX_VSHADER_H__1F984AC1_EAB9_11D3_81EA_0060520A50A9__INCLUDED_
/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description: реализация шейдера для использования в качестве
материала на объектах. Основная функция для шейдеров                                                               

Шейдер реализует мультитекстурирование и такие
эффекты, как движение, масштабирование текстур
и т.д.

Author: Grom 
Creation: 12 апреля 2000
***************************************************************/                

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DynamicVB.h"
#include "camera.h"
#include "../shader/shader.h"

//=====================================================================================//
//                                  class LMTexHolder                                  //
//=====================================================================================//
class LMTexHolder
{
private:
	std::vector<IDirectDrawSurface7*> LMs;
	IDirectDrawSurface7* Substitute;
	bool adjusted;
public:
	IDirectDrawSurface7* GetLM(unsigned int Num)
	{
		if(!adjusted) Adjust();
		return LMs.empty() ? NULL : LMs[0]; 
	};
	IDirectDrawSurface7* GetSubstitute(){return Substitute; };
	void Adjust();
	void Clear();
	~LMTexHolder() {Clear();};
	LMTexHolder():Substitute(NULL),adjusted(false) {};
};

class VShader  
{
	//========= внутренний буфер и что с ним связано ==================
protected:
	//Primi InHouseBuf;  //сам буфер
	//int CurIndex;      //индекс до которого буфер заполнен 
public:
	std::string FileName,ShaderName;//имя файла из которого загружен и имя шейдера
	//подготовка буфера для записи не больше VertNum вершин 
	//    Primi PrepareFill(int VertNum);
	//указывает сколько вершин действительно было записано в массив
	// void FilledSize(int VertNum) { CurIndex+=VertNum;  }
	//======================================================
public://Для вывода примитивов
	void Chop(const Primi *Prim,IDirect3DDevice7 *D3DDev_, DynamicVB *DynBuf);
	void Chop(const Primi *Prim,IDirect3DDevice7 *D3DDev_, DynamicVB *DynBuf,
		      unsigned int pcount);
	void Chop(D3DPRIMITIVETYPE PrimType,DynamicVB *DynBuf,
		unsigned int StartVert,int vertnum,unsigned short *idxs,int idxnum,
		const std::vector<const D3DMATRIX*> &Worlds);

	void Chop(D3DPRIMITIVETYPE primtype, IDirect3DVertexBuffer7 *vb, unsigned int sv, int vnum, const unsigned short *idxs, int idxnum);

	void PrecacheTextures() { LinkTextures(); }

protected: //низкий уровень используется при выводе примитива
	static float GetWave(WAVE_FORM WType, float *args,float Time);
	static DWORD GetGenCol(ColorGen_func &RGBGen,ColorGen_func &AlphaGen,float Time);
	bool NoDataChanges(int stagenum);
	void TuneRGBALPHAgen(int stage,int texstage);
	void LinkTextures();
	bool MakeBlendingForCalcPassCount(int) const;
	int MakeStageForPassCount(int) const;
	int CalcPassCount() const;

public:
	static LMTexHolder CachedLM; //Кэшированные текстуры карт освещенности
	int StageNum; //количество налагаемых текстур
	int PassNum;
	SHD_DEFVERTS_ATTRS VertDeform;
	D3DCULL CullMode;

	D3DCMPFUNC ZFunc[MAX_STAGES]; //функция сравнения для z-буфера
	bool NoMipMap[MAX_STAGES];
	bool NoPicMip[MAX_STAGES];
	bool ZWrite[MAX_STAGES];  //будем ли писать в буффер
	int ZBias[MAX_STAGES];// смещение в z-буфере

	bool AlphaBlend[MAX_STAGES]; //разрешение смешения цветов для прозрачности
	D3DBLEND SrcBlend[MAX_STAGES];
	D3DBLEND DstBlend[MAX_STAGES];
	bool AlphaTest[MAX_STAGES];//разрешение проверки отрисовки пикселов в зависимости от альфа-канала
	DWORD AlphaRef[MAX_STAGES];//с этим значением будет идти сравнение
	D3DCMPFUNC AlphaFunc[MAX_STAGES];//функция по которой происходит сравнение

	D3DFILLMODE FillMode[MAX_STAGES];//debug:режим отрисовки многоугольников (точки.линии.треугольники)

	int TexNum[MAX_STAGES];//количество текстур в анимации в stage #...
	STG_MAP_TYPE MapType[MAX_STAGES];
	float AnimFreq[MAX_STAGES];
	std::string TexMap[MAX_STAGES][MAX_ANIM_FRAMES];
	D3DTEXTUREADDRESS MapCoords[MAX_STAGES];
	unsigned Wrapping[MAX_STAGES];
	//генератор и модификатор текстурных координат
	TCGEN_TYPE TexGen[MAX_STAGES];
	int TModNum[MAX_STAGES];//количество модификаций
	tcMod_func TexMod[MAX_STAGES][MAX_TCMOD_FUNCS];
	ColorGen_func   RGBGen[MAX_STAGES];
	ColorGen_func   AlphaGen[MAX_STAGES];

protected://это тестирование VB
	static IDirect3DDevice7 *D3DDev;
	IDirectDrawSurface7 *DxTex[MAX_STAGES][MAX_ANIM_FRAMES];
	bool SetBlending(int i);
	int ApplyStates(IDirect3DDevice7 *D3DDev,int stagenum);
	int FlushDynBuffer(D3DPRIMITIVETYPE PrimType,const unsigned short *idx,int idxnum,int StartVertex,int VertNum,int i,int lmNum, DynamicVB *DynBuf);
	int FlushBuffer(D3DPRIMITIVETYPE PrimType, const unsigned short *idx, int idxnum, int StartVertex, int VertNum, int i, IDirect3DVertexBuffer7 *);
	int FlushDynBuffer(D3DPRIMITIVETYPE PrimType,const unsigned short *idx,int idxnum,int StartVertex,int VertNum,int i,DynamicVB *DynBuf, const std::vector<const D3DMATRIX*> &Worlds);
	int LMNum;//номер карты освещения
	bool TexturesLinked;//правильно ли заполнен массив с указателями на текстуры
public:
	bool Transparent;
public:
	void CreateTexModMatrix(float Time, int stagenum, D3DMATRIX *mat);
	void SetTexModMatrix(int stagenum, int texstage=0);
	void SetTexGenMatrix(int stagenum, int texstage=0);

	void PushBspTrianglesDyn(int ShdNum, DynamicVB *DynBuf);
	void PushBspTrianglesDynOld(int ShdNum, DynamicVB *DynBuf);

	VShader(const shader_struct &shader);
	IDirectDrawSurface7* GetTexture(const char *Nam, int stage);
	void Release(IDirect3DDevice7 *D3DDev);
	void AdjustHW(IDirect3DDevice7 *D3DDev);

	const shader_struct &GetTraits() const { return m_traits; }

	VShader();
	virtual ~VShader();

private:
	shader_struct m_traits;
};

#endif // !defined(AFX_VSHADER_H__1F984AC1_EAB9_11D3_81EA_0060520A50A9__INCLUDED_)
