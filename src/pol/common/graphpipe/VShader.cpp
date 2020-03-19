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
//#define D3D_OVERLOADS

#include "precomp.h"

#include "VShader.h"
#include <xmmintrin.h>
#include "StatesManager.h"
#include <common/texturemgr/texturemgr.h>
#include "graphpipe.h"

#include <undercover/iworld.h>
#include <undercover/globals.h>
#include <undercover/options/options.h>

void invmat(D3DMATRIX &a, const D3DMATRIX &b);

IDirect3DDevice7 *VShader::D3DDev = 0;
LMTexHolder VShader::CachedLM;

void *memcpy_amd(void *dest, const void *src, size_t n);

//=====================================================================================//
//                             void LMTexHolder::Adjust()                              //
//=====================================================================================//
void LMTexHolder::Adjust()
{
	if(adjusted) return;
	LMs.clear();

	LMs.push_back(TextureMgr::Instance()->Texture(":lm"));
	Substitute = TextureMgr::Instance()->Texture("textures.sys/grey.tga");
	adjusted = true;
}

//=====================================================================================//
//                              void LMTexHolder::Clear()                              //
//=====================================================================================//
void LMTexHolder::Clear() 
{
	adjusted = false;

	if(!LMs.empty())
	{
		TextureMgr::Instance()->Release(":lm");
		LMs.clear();
	}

	if(Substitute)
	{
		TextureMgr::Instance()->Release("textures.sys/grey.tga");
		Substitute = NULL;
	}
};

//=====================================================================================//
//                                 VShader::VShader()                                  //
//=====================================================================================//
VShader::VShader()
{
}

//=====================================================================================//
//                                 VShader::VShader()                                  //
//=====================================================================================//
VShader::VShader(const shader_struct &shader)
:	m_traits(shader)
{
	int i,cs;

	bool SkipLightmaps = 		!Options::GetInt("system.video.lightmaps");



	for (i=0;i<MAX_STAGES;i++)
		for (int j=0;j<MAX_ANIM_FRAMES;j++)
			DxTex[i][j]=NULL;

	StageNum=shader.num_stages;
	VertDeform=(SHD_DEFVERTS_ATTRS)shader.deformVertexes;

	//отсечение треугольников по направлению
	if(shader.flags&SHD_CULL_BACK)
		CullMode=D3DCULL_CCW;
	else
		if(shader.flags&SHD_CULL_FRONT)
			CullMode=D3DCULL_CW;
		else CullMode=D3DCULL_NONE;

		for(i=0,cs=0;i<shader.num_stages;i++,cs++)
		{
			//касательно прозрачности...
			MapType[cs]=shader.stage[i].map_type;
			if(SkipLightmaps && MapType[cs]==STG_LIGHTMAP)
			{
				cs--;
				StageNum--;
				continue;
			}

			AlphaBlend[cs]=true;
			if(shader.stage[i].fnBlend.src==BLEND_NONE&&
				shader.stage[i].fnBlend.dst==BLEND_NONE)
				AlphaBlend[cs]=false;
			if(shader.stage[i].fnBlend.src==BLEND_ONE&&
				shader.stage[i].fnBlend.dst==BLEND_ZERO)
				AlphaBlend[cs]=false;
			//AlphaBlend[i]=!(shader.stage[i].fnBlend.src==BLEND_NONE&&shader.stage[i].fnBlend.dst==BLEND_NONE);
			switch(shader.stage[i].fnBlend.dst)
			{
			case BLEND_NONE:DstBlend[cs]=D3DBLEND_ZERO;break;
			case BLEND_ONE:DstBlend[cs]=D3DBLEND_ONE;break;
			case BLEND_ZERO:DstBlend[cs]=D3DBLEND_ZERO;break;
			case BLEND_DST_COLOR:DstBlend[cs]=D3DBLEND_DESTCOLOR;break;
			case BLEND_SRC_COLOR:DstBlend[cs]=D3DBLEND_SRCCOLOR;break;
			case BLEND_SRC_ALPHA:DstBlend[cs]=D3DBLEND_SRCALPHA;break;
			case BLEND_INV_SRC_ALPHA:DstBlend[cs]=D3DBLEND_INVSRCALPHA;break;
			case BLEND_INV_DST_COLOR:DstBlend[cs]=D3DBLEND_INVSRCCOLOR;break;
			case BLEND_INV_SRC_COLOR:DstBlend[cs]=D3DBLEND_INVDESTCOLOR;break;
			}
			switch(shader.stage[i].fnBlend.src)
			{
			case BLEND_NONE:SrcBlend[cs]=D3DBLEND_ZERO;break;
			case BLEND_ONE:SrcBlend[cs]=D3DBLEND_ONE;break;
			case BLEND_ZERO:SrcBlend[cs]=D3DBLEND_ZERO;break;
			case BLEND_DST_COLOR:SrcBlend[cs]=D3DBLEND_DESTCOLOR;break;
			case BLEND_SRC_COLOR:SrcBlend[cs]=D3DBLEND_SRCCOLOR;break;
			case BLEND_SRC_ALPHA:SrcBlend[cs]=D3DBLEND_SRCALPHA;break;
			case BLEND_INV_SRC_ALPHA:SrcBlend[cs]=D3DBLEND_INVSRCALPHA;break;
			case BLEND_INV_DST_COLOR:SrcBlend[cs]=D3DBLEND_INVSRCCOLOR;break;
			case BLEND_INV_SRC_COLOR:SrcBlend[cs]=D3DBLEND_INVDESTCOLOR;break;
			}
			//обработка текстурных координат
			TexGen[cs]=shader.stage[i].tcGen;
			switch(TexGen[cs])
			{
			case TCGEN_LIGHTSPOT:
			case TCGEN_REFLECTION:
			case TCGEN_SPHERE:
				Wrapping[cs]=D3DWRAPCOORD_0|D3DWRAPCOORD_1;
				break;
			default:
			case TCGEN_NONE: Wrapping[cs]=0;
			}

			TModNum[cs]=shader.stage[i].num_tcMods;
			for(int k=0;k<TModNum[cs];k++)
			{
				TexMod[cs][k]=shader.stage[i].tcMod[k];
			}
			RGBGen[cs]=shader.stage[i].rgbGen;
			AlphaGen[cs]=shader.stage[i].alphaGen;
			//сами текстуры
			MapType[cs]=shader.stage[i].map_type;
			AnimFreq[cs]=shader.stage[i].anim_freq;
			TexNum[cs]=(MapType[cs]==STG_LIGHTMAP)?0:shader.stage[i].num_textures;
			for(int j=0;j<TexNum[cs];j++)
			{
				TexMap[cs][j]=shader.stage[i].texture[j];
				if(TexMap[cs][j]=="_absent_")
					MapType[cs]=STG_NULL_MAP;

			}
			switch(MapType[cs])
			{
			case STG_ANIMMAP:
			case STG_LIGHTMAP:
			case STG_CLAMPMAP:
			case STG_NULL_MAP:
				MapCoords[cs]=D3DTADDRESS_CLAMP;
				break;
			case STG_MAP:
				MapCoords[cs]=D3DTADDRESS_WRAP;
			default:
				break;

			}

			ZBias[cs]=shader.stage[0].zBias;
			switch(shader.stage[i].fm_type)
			{
			case FM_POINT:FillMode[cs]=D3DFILL_POINT;break;
			case FM_WIRE: FillMode[cs]=D3DFILL_WIREFRAME;break;
			case FM_SOLID:      default:
				FillMode[cs]=D3DFILL_SOLID;break;
			}
			//FillMode[i]= D3DFILL_WIREFRAME ;//*/D3DFILL_SOLID;
			switch(shader.stage[cs].depth_func)    
			{
			case DF_LEQUAL:
				ZFunc[cs]=D3DCMP_LESSEQUAL;
				break;
			case DF_EQUAL:
				ZFunc[cs]=D3DCMP_EQUAL;
				break;
			case DF_ALWAYS:
				ZFunc[cs]=D3DCMP_ALWAYS;
				break;

			}
			ZWrite[cs]=STG_DEPTHWRITE&shader.stage[i].flags?true:(AlphaBlend[0]||cs?false:true);
			//ZWrite[cs]=STG_DEPTHWRITE&shader.stage[i].flags?true:(AlphaBlend[0]?false:true);

			AlphaTest[cs]=shader.stage[i].af_type!=AF_NONE;
			switch(shader.stage[i].af_type)
			{
			case AF_GT0:
				AlphaFunc[cs]=D3DCMP_GREATER;
				AlphaRef[cs]=0;
				break;
			case AF_GE128:
				AlphaFunc[cs]=D3DCMP_GREATEREQUAL;
				AlphaRef[cs]=128;
				break;
			case AF_LT128:
				AlphaFunc[cs]=D3DCMP_LESS;
				AlphaRef[cs]=128;
				break;
			}
			if(shader.stage[i].flags&STG_NOMIPMAP) NoMipMap[cs]=true;
			else NoMipMap[cs]=false;
			if(shader.stage[i].flags&STG_NOPICMIP) NoPicMip[cs]=true;
			else NoPicMip[cs]=false;
		}
		Transparent=AlphaBlend[0];
		TexturesLinked=false;


	PassNum = CalcPassCount();
}

//=====================================================================================//
//                               void VShader::Release()                               //
//=====================================================================================//
void VShader::Release(IDirect3DDevice7 * /*_D3DDev*/)
{
	//#define PipeLog logFile["graphpipe.log"]
	//   PipeLog("releasing %s\n",ShaderName.c_str());
	for(int i=0;i<StageNum;i++)
	{
		//освобождаем текстуры
		//   PipeLog("XXX  stage %d\n",i);
		if(MapType[i]==STG_LIGHTMAP) continue;
		if(MapType[i]==STG_ANIMMAP)
			for(int j=0;j<=TexNum[i];j++)
			{
				//       PipeLog("XXX animated  %s,%d\n",TexMap[i][j].c_str(),j);
				TextureMgr::Instance()->Release(TexMap[i][j].c_str());
			}
		else
		{
			//      PipeLog("XXX  %s\n",TexMap[i][0].c_str());
			TextureMgr::Instance()->Release(TexMap[i][0].c_str());

		}
	} 
	TexturesLinked = false;
	D3DDev=NULL;
	CachedLM.Clear();
}

//=====================================================================================//
//                              void VShader::AdjustHW()                               //
//=====================================================================================//
void VShader::AdjustHW(IDirect3DDevice7 *_D3DDev)
{
	D3DDev=_D3DDev;
	TexturesLinked=false;
	StatesManager::Clear();
	LinkTextures();
}

//=====================================================================================//
//                            void VShader::LinkTextures()                             //
//=====================================================================================//
void VShader::LinkTextures()
{
	TexturesLinked=true;
	for(int i=0;i<StageNum;i++)
	{
		for (int j=0;j<TexNum[i];j++)
		{
			DxTex[i][j]=GetTexture(TexMap[i][j].c_str(),i);
		}
	}
	//CachedLM.Adjust();
}

//=====================================================================================//
//                                 VShader::~VShader()                                 //
//=====================================================================================//
VShader::~VShader()
{
}

//=====================================================================================//
//                               void memcpy_alignout()                                //
//=====================================================================================//
void memcpy_alignout(void *dest, const void *src, size_t count)
{
	unsigned int pad = (unsigned int )dest & 0xF; // найдем смещение от 16-и байтного выравнивания

	if(pad)
	{
		unsigned int invpad = 0x10 - pad;
		memcpy(dest,src,invpad);
		dest = (char *)dest + invpad;
		src = (const char *)src + invpad;
		count -= invpad;
	}

	unsigned int rem = count & 0x7F;
	unsigned int loopcount = count >> 7;

	register __m128 xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;

	for(unsigned int i = 0; i < loopcount; ++i)
	{
		xmm0 = _mm_loadu_ps((float*)src + 0);
		xmm1 = _mm_loadu_ps((float*)src + 4);
		xmm2 = _mm_loadu_ps((float*)src + 8);
		xmm3 = _mm_loadu_ps((float*)src + 12);
		xmm4 = _mm_loadu_ps((float*)src + 16);
		xmm5 = _mm_loadu_ps((float*)src + 20);
		xmm6 = _mm_loadu_ps((float*)src + 24);
		xmm7 = _mm_loadu_ps((float*)src + 28);
		_mm_store_ps((float*)dest + 0,xmm0);
		_mm_store_ps((float*)dest + 4,xmm1);
		_mm_store_ps((float*)dest + 8,xmm2);
		_mm_store_ps((float*)dest + 12,xmm3);
		_mm_store_ps((float*)dest + 16,xmm4);
		_mm_store_ps((float*)dest + 20,xmm5);
		_mm_store_ps((float*)dest + 24,xmm6);
		_mm_store_ps((float*)dest + 28,xmm7);

		src = (const char *)src + 0x80;
		dest = (char *)dest + 0x80;
	}

	memcpy(dest,src,rem);
}

//=====================================================================================//
//                                void VShader::Chop()                                 //
//=====================================================================================//
void VShader::Chop(const Primi *Prim,IDirect3DDevice7 *D3DDev_,
				   DynamicVB *DynBuf, unsigned int pcount)
{//отрисовать примитив
	if(!D3DDev) D3DDev=D3DDev_;
	if(!TexturesLinked) LinkTextures();

	D3DPRIMITIVETYPE PrimType;
	//float Time=Timer::GetSeconds();
	switch(Prim->Prim)
	{
	case Primi::TRIANGLE:PrimType=D3DPT_TRIANGLELIST;break;
	case Primi::TRIANGLEFAN: PrimType=D3DPT_TRIANGLEFAN;break;
	case Primi::TRIANGLESTRIP: PrimType=D3DPT_TRIANGLESTRIP;break;
	case Primi::LINES: PrimType=D3DPT_LINELIST;break;
	case Primi::LINESSTRIP: PrimType=D3DPT_LINESTRIP;break;
	case Primi::POINTS: PrimType=D3DPT_POINTLIST;break;
	}

	unsigned int StartVertex;
	unsigned int vcount = pcount;
	char *Data=(char*)DynBuf->Lock(vcount,&StartVertex);
	Primi::FVFVertex *vts = reinterpret_cast<Primi::FVFVertex *>(Data);
	if(!Data) return;

	//memcpy_amd(vts,Prim->Verts.begin(),vcount*sizeof(*Prim->Verts.begin()));
	//memcpy(vts,Prim->Verts.begin(),vcount*sizeof(*Prim->Verts.begin()));
	//memcpy_alignout(vts,Prim->Verts.begin(),vcount*sizeof(*Prim->Verts.begin()));

	/// Через этот метод рисуются:
	///  - дальний план
	///  - хексгрид
	///  - персонажи
	///  - тени
	///  - летящие гранаты
	///  - куски взрывающейся модели
	///  - разлетающееся мясо
	///  - трасер прямой и параболический
	///  - разбивающееся стекло
	///  - выпадающие гильзы
	///  - отметины на стенах
	///  - предметы на уровне
	///  - 3D интерфейс
	///  - границы областей видимости

#if 0
	char *datas = (char *)(&*Prim->Verts.begin());
	char *datad = (char *)vts;
	char *datase = datas + sizeof(Primi::FVFVertex)*vcount;
	//for(; datas < datase; datas += 4, datad += 4)
	//{
	//	_asm
	//	{
	//		mov eax, DWORD PTR datas
	//		movnti DWORD PTR datad, eax
	//	}
	//}

	while(((int)(datad))&0xf) *datad++ = *datas++;

#define POL_GRANULE 16
	int dwords = (datase-datas)/POL_GRANULE;

	_asm
	{
		mov ecx, dwords
		mov esi, datas
		mov edi, datad

	start_copy:
		movdqu xmm0, XMMWORD PTR [esi]
		movntdq XMMWORD PTR [edi], xmm0
		//movdqa XMMWORD PTR [edi], xmm0

		add esi,POL_GRANULE
		add edi,POL_GRANULE

		loop start_copy
	}

	memcpy(datad+dwords*POL_GRANULE,datas+dwords*POL_GRANULE,vcount*sizeof(Primi::FVFVertex)-dwords*POL_GRANULE);

#undef POL_GRANULE
#else
    
	std::copy_n(Prim->Verts.begin(), vcount, vts);

#endif

	DynBuf->Unlock();
	for(int k=0;k<StageNum;k++)
	{
		k+=FlushDynBuffer(PrimType,&Prim->Idxs[0],Prim->Idxs.size(),StartVertex,vcount,k,-1,DynBuf);
	} 
}

//=====================================================================================//
//                                void VShader::Chop()                                 //
//=====================================================================================//
void VShader::Chop(const Primi *Prim,IDirect3DDevice7 *D3DDev_, DynamicVB *DynBuf)
{
	Chop(Prim,D3DDev_,DynBuf,Prim->GetVertexCount());
}

//=====================================================================================//
//                                void VShader::Chop()                                 //
//=====================================================================================//
void VShader::Chop(D3DPRIMITIVETYPE PrimType,DynamicVB *DynBuf,
				   unsigned int StartVert,int vertnum,unsigned short *idxs,int idxnum,
				   const std::vector<const D3DMATRIX*> &Worlds)
{//отрисовать примитив
	if(!D3DDev) D3DDev=D3DKernel::GetD3DDevice();
	if(!TexturesLinked) LinkTextures();

	for(int k=0;k<StageNum;k++)
		k+=FlushDynBuffer(PrimType,idxs,idxnum,StartVert,vertnum,k,DynBuf,Worlds);
}

//=====================================================================================//
//                                void VShader::Chop()                                 //
//=====================================================================================//
void VShader::Chop(D3DPRIMITIVETYPE primtype, IDirect3DVertexBuffer7 *vb, unsigned int sv,
				   int vnum, const unsigned short *idxs, int idxnum)
{
	if(!D3DDev) D3DDev=D3DKernel::GetD3DDevice();
	if(!TexturesLinked) LinkTextures();

	for(int k=0; k<StageNum; k++)
	{
		k += FlushBuffer(primtype,idxs,idxnum,sv,vnum,k,vb);
	}
}

//=====================================================================================//
//                             int VShader::FlushBuffer()                              //
//=====================================================================================//
int VShader::FlushBuffer(D3DPRIMITIVETYPE primtype, const unsigned short *idxs,
						 int idxnum, int sv, int vnum, int i, IDirect3DVertexBuffer7 *vb)
{
	int toret = 0;

	//lmNum=-1;
	//if(MapType[i] == STG_LIGHTMAP) return 0;

	toret = ApplyStates(D3DDev,i);

	DWORD col = GetGenCol(RGBGen[i], AlphaGen[i], Timer::GetSeconds());
	//StatesManager::SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR,col);
	StatesManager::SetRenderState(D3DRENDERSTATE_AMBIENT, col);

	if(idxnum)
	{
		D3DDev->DrawIndexedPrimitiveVB(primtype,vb,sv,vnum,(LPWORD)idxs,idxnum,0);
	}
	else
	{
		D3DDev->DrawPrimitiveVB(primtype,vb,sv,vnum,0);
	}

	return toret;
}

//=====================================================================================//
//                     IDirectDrawSurface7 * VShader::GetTexture()                     //
//=====================================================================================//
IDirectDrawSurface7 * VShader::GetTexture(const char *Nam,int stage)
{
	IDirectDrawSurface7 *tex;
	int mip,qual;

	if(NoPicMip[stage]) //нельзя ухудшать качество
	{
		if(NoMipMap[stage]) mip=0; //не нужно генерить мипмапы
		else mip=Options::GetInt("system.video.mipmap");
		qual=0;
	}
	else
	{
		qual=Options::GetInt("system.video.texquality");
		if(NoMipMap[stage]) mip=qual;
		else mip=Options::GetInt("system.video.mipmap");
	}

	tex=TextureMgr::Instance()->Texture(Nam,mip,qual);
	if(!tex)
	{
		//logFile["graphpipe.log"]("не загрузилась текстура %s\n",Nam);
		tex=TextureMgr::Instance()->Texture("textures.sys/notexture.tga");
		/*if(!tex)
		{int t=0;} */
	}
	return tex;
}

//=====================================================================================//
//                              float VShader::GetWave()                               //
//=====================================================================================//
float VShader::GetWave(WAVE_FORM WType, float *args,float Time)
{
	float base,amp,phase,freq,lambda,f;
	base=args[0];
	amp=args[1];

	switch(WType)
	{
	case WF_SIN:
		phase=args[2];
		freq=args[3];
		f=sin(Time*PIm2*freq+phase);
		break;
	case WF_TRIANGLE:
		phase=args[2];
		freq=args[3];
		lambda=1.f/freq,f;
		f=fabs(fmod(Time*freq+phase,lambda)-lambda/2)/(lambda/2)*2-0.5;
		break;
	case WF_SQUARE:
		phase=args[2];
		freq=args[3];
		lambda=1.f/freq,f;
		f=fmod(Time*freq+phase,lambda)>lambda/2?-1:1;
		break;
	case WF_SAWTOOTH:
		phase=args[2];
		freq=args[3];
		lambda=1.f/freq,f;
		f=fmod(Time*freq+phase,lambda)/lambda;
		break;
	case WF_INVSAWTOOTH:
		phase=args[2];
		freq=args[3];
		lambda=1.f/freq,f;
		f=1.0-fmod(Time*freq+phase,lambda)/lambda;
		break;
	default:
	case WF_NONE:
		f=1;
		break;
	}
	return base+amp*f;
}

bool DoMulti=true,DoWire=false;

//=====================================================================================//
//                             bool VShader::SetBlending()                             //
//=====================================================================================//
bool VShader::SetBlending(int i)
{
	if(i>4) return false;
	if(TexGen[i]!=TCGEN_NONE)return false; //fixme: без этого вылетает на некторых видюхах (TNT2)
	if(AlphaBlend[i-1]&&
		(DstBlend[i-1]!=D3DBLEND_INVSRCALPHA|| SrcBlend[i]!=D3DBLEND_SRCALPHA)&&
		(DstBlend[i-1]!=D3DBLEND_ONE||SrcBlend[i-1]!=D3DBLEND_ONE))
		return false;
	if(RGBGen[i].type==CG_WAVE||RGBGen[i].type==CG_VERTEX||
		AlphaGen[i].type==CG_WAVE||AlphaGen[i].type==CG_VERTEX)
		return false;
	unsigned long arg=D3DTOP_DISABLE;
	if(DstBlend[i]==D3DBLEND_ONE&&SrcBlend[i]==D3DBLEND_ONE)
		arg=D3DTOP_ADD;
	else if(DstBlend[i]==D3DBLEND_SRCCOLOR&&SrcBlend[i]==D3DBLEND_DESTCOLOR)
		arg=D3DTOP_MODULATE2X;
	else if(DstBlend[i]==D3DBLEND_INVSRCALPHA&&SrcBlend[i]==D3DBLEND_SRCALPHA)
		arg=D3DTOP_BLENDTEXTUREALPHA;
	else if(DstBlend[i]==D3DBLEND_ZERO&&SrcBlend[i]==D3DBLEND_DESTCOLOR)
		arg=D3DTOP_MODULATE;
	else if(DstBlend[i]==D3DBLEND_SRCCOLOR&&SrcBlend[i]==D3DBLEND_ZERO)
		arg=D3DTOP_MODULATE;
	else return false;

	StatesManager::SetTextureStageState(1,D3DTSS_COLOROP,arg);
	//---test
	StatesManager::SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	StatesManager::SetTextureStageState(1,D3DTSS_COLORARG2,D3DTA_CURRENT);
	StatesManager::SetTextureStageState(1,D3DTSS_COLORARG1,D3DTA_TEXTURE);
	//---

	//FIXME:необходимо правильно настроить альфу
	return true;
}

//=====================================================================================//
//                    bool VShader::MakeBlendingForCalcPassCount()                     //
//=====================================================================================//
bool VShader::MakeBlendingForCalcPassCount(int i) const
{
	if(i>4) return false;
	if(TexGen[i]!=TCGEN_NONE)return false; //fixme: без этого вылетает на некторых видюхах (TNT2)
	if(AlphaBlend[i-1]&&
		(DstBlend[i-1]!=D3DBLEND_INVSRCALPHA|| SrcBlend[i]!=D3DBLEND_SRCALPHA)&&
		(DstBlend[i-1]!=D3DBLEND_ONE||SrcBlend[i-1]!=D3DBLEND_ONE))
		return false;
	if(RGBGen[i].type==CG_WAVE||RGBGen[i].type==CG_VERTEX||
		AlphaGen[i].type==CG_WAVE||AlphaGen[i].type==CG_VERTEX)
		return false;
	unsigned long arg=D3DTOP_DISABLE;
	if(DstBlend[i]==D3DBLEND_ONE&&SrcBlend[i]==D3DBLEND_ONE)
		arg=D3DTOP_ADD;
	else if(DstBlend[i]==D3DBLEND_SRCCOLOR&&SrcBlend[i]==D3DBLEND_DESTCOLOR)
		arg=D3DTOP_MODULATE2X;
	else if(DstBlend[i]==D3DBLEND_INVSRCALPHA&&SrcBlend[i]==D3DBLEND_SRCALPHA)
		arg=D3DTOP_BLENDTEXTUREALPHA;
	else if(DstBlend[i]==D3DBLEND_ZERO&&SrcBlend[i]==D3DBLEND_DESTCOLOR)
		arg=D3DTOP_MODULATE;
	else if(DstBlend[i]==D3DBLEND_SRCCOLOR&&SrcBlend[i]==D3DBLEND_ZERO)
		arg=D3DTOP_MODULATE;
	else return false;

	return true;
}

////=====================================================================================//
////                         void VShader::PushBspTrianglesDyn()                         //
////=====================================================================================//
//void VShader::PushBspTrianglesDyn(int ShdNum, DynamicVB *DynBuf)
//{
//	if(!StageNum)return;
//	if(!D3DDev) D3DDev=D3DKernel::GetD3DDevice();
//	if(!TexturesLinked) LinkTextures();
//	int _Size=Bsp::VisNum[ShdNum];
//	unsigned short *curidx=Bsp::VisTris[ShdNum];
//	int VertNum=Bsp::VertNum;
//	if(!VertNum||!_Size) return;
//	for(int k=0;k<StageNum;k++)
//		k+=FlushDynBuffer(D3DPT_TRIANGLELIST,curidx,_Size, 0,VertNum,k,ShdNum,DynBuf);
//}

////=====================================================================================//
////                       void VShader::PushBspTrianglesDynOld()                        //
////=====================================================================================//
//void VShader::PushBspTrianglesDynOld(int ShdNum, DynamicVB *DynBuf)
//{
//	//return;
//	if(!StageNum)return;
//	if(!D3DDev) D3DDev=D3DKernel::GetD3DDevice();
//	if(!TexturesLinked) LinkTextures();
//
//
//	LevelVertex *Verts=Bsp::Verts;
//	int _Size=Bsp::VisNum[ShdNum];
//	short *vidx=Bsp::VertIdx;
//	unsigned short *curidx=Bsp::VisTris[ShdNum];
//	unsigned int StartVertex,VertNum=0;
//	unsigned int VertsWritten=0;
//	do
//	{
//		int s(_Size-VertsWritten);
//		char *Data;
//		s=s>501?501:s;
//		while(!(Data=(char*)DynBuf->LockSupposed(s,&StartVertex)))
//		{
//			s>>=1;
//			s-=s%3;//while(s%3) s--;
//			if(!s) return;
//		}
//		unsigned short *triidx=Bsp::IdxTris;
//		Bsp::ClearVertIndexes(ShdNum);
//		int idxnum=0;
//		VertNum=0;
//		_asm emms;
//		for(int i=0;i<s;i++)
//		{
//			vidx=&Bsp::VertIdx[*curidx];
//			if(*vidx==-1)
//			{
//				*vidx=VertNum++;
//				LevelVertex *v=&Verts[*curidx];
//#ifdef _NO_ASM
//				memcpy(Data,&Verts[*curidx].Pos,sizeof(point3)); Data+=sizeof(point3);
//				Data+=sizeof(int);
//				memcpy(Data,&Verts[*curidx].uv,sizeof(texcoord)); Data+=sizeof(texcoord);
//				memcpy(Data,&Verts[*curidx].lightmapuv,sizeof(texcoord)); Data+=sizeof(texcoord);
//#else
//				_asm
//				{
//					//emms; 
//					mov eax,v;
//					mov edi,Data;
//					movd mm1,[eax+8]LevelVertex.Pos;
//					movq mm0,[eax  ]LevelVertex.Pos;
//					movq mm2,[eax]LevelVertex.uv;
//					movq mm3,[eax]LevelVertex.lightmapuv;
//					lea eax,[edi+44];
//					movq [edi   ],mm0;
//					movq [edi+16+12],mm2;
//					movq [edi+24+12],mm3;
//					movd [edi+ 8],mm1;
//					//emms;
//					mov Data,eax;
//				} 
//#endif
//			}
//			*triidx++=*vidx;
//			curidx++;
//			idxnum++;
//		}
//		_asm emms;
//		DynBuf->UnlockSupposed(VertNum);
//
//		/*logFile["bsptest.log"]("idxnum:%d ,vertnum:%d, startvertex:%d\n",(int)idxnum,VertNum,StartVertex);
//		for(int p=0;p<idxnum;p++)
//		{
//		logFile["bsptest.log"]("%u ",(unsigned int)Bsp::IdxTris[p]);
//		if(Bsp::IdxTris[p]<0||Bsp::IdxTris[p]>VertNum) logFile["bsptest.log"]("\ntoo bad\n");
//		}
//		logFile["bsptest.log"]("\n");	*/
//
//
//		for(int k=0;k<StageNum;k++)
//			k+=FlushDynBuffer(D3DPT_TRIANGLELIST,Bsp::IdxTris,idxnum, StartVertex,VertNum,
//			k,ShdNum,DynBuf);
//		VertsWritten+=s;
//	}while(VertsWritten<_Size);
//}

//=====================================================================================//
//                           void VShader::SetTexModMatrix()                           //
//=====================================================================================//
void VShader::SetTexModMatrix(int stagenum,int texstage)
{
	int TexModNum=TModNum[stagenum];
	static D3DMATRIX res;
	CreateTexModMatrix(Timer::GetSeconds(),stagenum, &res);

	if(TexModNum)
	{
		StatesManager::SetTextureStageState(texstage,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		StatesManager::SetTransform(
			texstage?D3DTRANSFORMSTATE_TEXTURE1:D3DTRANSFORMSTATE_TEXTURE0,
			&res);
	}
	else
	{
		StatesManager::SetTextureStageState(texstage,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		StatesManager::SetTransform(
			texstage?D3DTRANSFORMSTATE_TEXTURE1:D3DTRANSFORMSTATE_TEXTURE0,
			&res);
	}
}

//=====================================================================================//
//                           void VShader::SetTexGenMatrix()                           //
//=====================================================================================//
void VShader::SetTexGenMatrix(int stagenum,int texstage)
{
	int TexGenType=TexGen[stagenum];
	switch(TexGenType)
	{
	case TCGEN_REFLECTION:
		StatesManager::SetTextureStageState(texstage,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		StatesManager::SetTextureStageState(texstage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
		break;
	case TCGEN_SPHERE:
		StatesManager::SetTextureStageState(texstage,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		StatesManager::SetTextureStageState(texstage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
		break;

	case TCGEN_LIGHTSPOT:
		StatesManager::SetTextureStageState(texstage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT3);
		//StatesManager::SetTextureStageState(texstage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
		StatesManager::SetTextureStageState(texstage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );
		break;
	case TCGEN_NONE:
	default:

		//D3DKernel::GetD3DDevice()->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		StatesManager::SetTextureStageState(texstage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
		break;
	}
}


//=====================================================================================//
//                         void VShader::CreateTexModMatrix()                          //
//=====================================================================================//
void VShader::CreateTexModMatrix(float Time, int stagenum, D3DMATRIX *mat)
{
	static D3DMATRIX res;
	res=*IDENTITYMAT;

	int TexModNum=TModNum[stagenum];
	float u,v;
	for(int m=0;m<TexModNum;m++)
	{
		*mat=*IDENTITYMAT;
		float *args=TexMod[stagenum][m].arg;
		switch(TexMod[stagenum][m].type)
		{
		case TCMOD_TURB:
			{
				float l_scale=1.f/50,l_add=0;	//параметры тумана по дальности
				float h_scale=-1.f/5,h_add=0.6; //параметры тумана по высоте
				l_scale=args[0];
				l_add=args[1];
				h_scale=args[2];
				h_add=args[3];

				D3DMATRIX q = mll::algebra::D3DMATRIX(IWorld::Get()->GetPipe()->GetCam()->GetView());
				invmat(res,q);
				//дальность
				res._11=res._21=0;
				res._41=l_add;res._31=l_scale;


				//высота
				res._12=res._13;res._22=res._23;res._32=res._33;res._42=res._43;

				res._12*=h_scale; res._22*=h_scale; res._32*=h_scale; res._42*=h_scale;
				res._42+=h_add;

			}
			break;
		case TCMOD_SCROLL:
			u=(Time)*args[0];
			v=(Time)*args[1];
			mat->_31 = u; mat->_32 = v;
			D3DMath_MatrixMultiply(res,res,*mat);
			break;
		case TCMOD_ROTATE:
			{
				mat->_31 = -0.5; mat->_32 = -0.5; 
				D3DMath_MatrixMultiply(res,res,*mat);

				D3DUtil_SetRotateZMatrix(*mat,TORAD(Time*args[0]));
				D3DMath_MatrixMultiply(res,res,*mat);

				D3DUtil_SetIdentityMatrix(*mat);
				mat->_31 = 0.5; mat->_32 = 0.5;
				D3DMath_MatrixMultiply(res,res,*mat);
			}
			break;
		case TCMOD_SCALE:
			/*mat->_31 = -0.5; mat->_32 = -0.5;
			D3DMath_MatrixMultiply(res,res,*mat);

			D3DUtil_SetScaleMatrix(*mat,args[0],args[1],1);
			D3DMath_MatrixMultiply(res,res,*mat);

			D3DUtil_SetIdentityMatrix(*mat);
			mat->_31 = 0.5; mat->_32 = 0.5;
			D3DMath_MatrixMultiply(res,res,*mat);*/
			D3DUtil_SetScaleMatrix(*mat,args[0],args[1],1);
			mat->_31 = -0.5*args[0]+0.5; mat->_32 = -0.5*args[1]+0.5;
			D3DMath_MatrixMultiply(res,res,*mat);
			break;
		case TCMOD_STRETCH:
			{
				float f=GetWave(TexMod[stagenum][m].wave_form,TexMod[stagenum][m].arg,Time);
				mat->_31 = -0.5; mat->_32 = -0.5; 
				D3DMath_MatrixMultiply(res,res,*mat);

				D3DUtil_SetScaleMatrix(*mat,f,f,1);
				D3DMath_MatrixMultiply(res,res,*mat);

				D3DUtil_SetIdentityMatrix(*mat);
				mat->_31 = 0.5; mat->_32 = 0.5; 
				D3DMath_MatrixMultiply(res,res,*mat);
				break;
			}
		case TCMOD_EMBOSS:
			{
				/*D3DVECTOR LDir(4*sin(Time*3),4*cos(Time*3),-2);
				LDir=Normalize(LDir);
				u = u+args[0]*LDir.x;
				v = v+args[0]*LDir.y;*/
				u=args[0];
				v=args[1];
				mat->_31 = u; mat->_32 = v;
				D3DMath_MatrixMultiply(res,res,*mat);
				break;
			}
		case TCMOD_NONE:
		default:
			break;
		}
	}
	*mat=res;
}

//=====================================================================================//
//                           void VShader::TuneRGBALPHAgen()                           //
//=====================================================================================//
void VShader::TuneRGBALPHAgen(int i,int texstage)
{
	bool lighting=false;
	switch(RGBGen[i].type)
	{
	case CG_IDENTITY:   case CG_NONE:case CG_LIGHTINGDIFFUSE: 
	default:
		StatesManager::SetTextureStageState(texstage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		break;
	case CG_VERTEX:
		StatesManager::SetTextureStageState(texstage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		StatesManager::SetTextureStageState(texstage, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		break;
	case CG_WAVE:
		StatesManager::SetTextureStageState(texstage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		//StatesManager::SetTextureStageState(texstage, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		StatesManager::SetTextureStageState(texstage, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		lighting=true;
		break;
	}
	switch(AlphaGen[i].type)
	{
	case CG_IDENTITY:   case CG_NONE:case CG_LIGHTINGDIFFUSE: 
	default:
		StatesManager::SetTextureStageState(texstage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		break;
	case CG_VERTEX:
		StatesManager::SetTextureStageState(texstage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		StatesManager::SetTextureStageState(texstage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		break;
	case CG_WAVE:
		StatesManager::SetTextureStageState(texstage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		//StatesManager::SetTextureStageState(texstage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
		StatesManager::SetTextureStageState(texstage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		lighting=true;
		break;
	}
	StatesManager::SetRenderState(D3DRENDERSTATE_LIGHTING,lighting?TRUE:FALSE);
}

//=====================================================================================//
//                            static void UnSetupPixelFog()                            //
//=====================================================================================//
static void UnSetupPixelFog(/*DWORD dwColor, DWORD dwMode*/)
{
	// Disable fog blending.
	StatesManager::SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE); 
}

//=====================================================================================//
//                             static void SetupPixelFog()                             //
//=====================================================================================//
static void SetupPixelFog(/*DWORD dwColor, DWORD dwMode*/)
{
	DWORD dwMode = D3DFOG_LINEAR;
	DWORD dwColor = 0xFFFFFFFF;
	float fStart = 0.0f,    // for linear mode     
		fEnd   = 100.f,
		fDensity = 0.12;  // for exponential modes    
	// Enable fog blending.
	StatesManager::SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE); 
	// Set the fog color.
	StatesManager::SetRenderState(D3DRENDERSTATE_FOGCOLOR, dwColor);    
	// Set fog parameters.
	if(D3DFOG_LINEAR == dwMode)
	{
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, dwMode);
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD *)(&fStart));
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGEND,   *(DWORD *)(&fEnd));
	}  
	else
	{
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD *)(&fStart));
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGEND,   *(DWORD *)(&fEnd));
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, dwMode);
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGDENSITY, *(DWORD *)(&fDensity));
	} 
}

//=====================================================================================//
//                     int VShader::MakeStageForPassCount() const                      //
//=====================================================================================//
int VShader::MakeStageForPassCount(int i) const
{
	if(i+1 == StageNum) return 0;

	//если есть мультитекстурирование
	bool multitexlocked = (MapType[i+1] == STG_LIGHTMAP && LMNum < 0);
	bool blendpassed = (!multitexlocked) && MakeBlendingForCalcPassCount(i+1);

	if(DoMulti && blendpassed && !multitexlocked) return 1;
	return 0;
}

//=====================================================================================//
//                         int VShader::CalcPassCount() const                          //
//=====================================================================================//
int VShader::CalcPassCount() const
{
	int passcount = 0;
	for(int i = 0; i < StageNum; ++i, ++passcount)
	{
		i += MakeStageForPassCount(i);
	}
	return passcount;
}

//=====================================================================================//
//                             int VShader::ApplyStates()                              //
//=====================================================================================//
int VShader::ApplyStates(IDirect3DDevice7 * /*D3DDev*/,int stagenum)
{
	int i=stagenum;

	if(!i) StatesManager::SetRenderState(D3DRENDERSTATE_CULLMODE,CullMode);
	StatesManager::SetRenderState(D3DRENDERSTATE_FILLMODE,DoWire?D3DFILL_WIREFRAME:FillMode[i]);


	//настройка прозрачности
	StatesManager::SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,AlphaBlend[i]?TRUE:FALSE);
	StatesManager::SetRenderState(D3DRENDERSTATE_SRCBLEND,SrcBlend[i]);
	StatesManager::SetRenderState(D3DRENDERSTATE_DESTBLEND,DstBlend[i]);
	StatesManager::SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,AlphaTest[i]?TRUE:FALSE);
	if(AlphaTest[i])
	{
		StatesManager::SetRenderState(D3DRENDERSTATE_ALPHAREF,AlphaRef[i]);
		StatesManager::SetRenderState(D3DRENDERSTATE_ALPHAFUNC,AlphaFunc[i]);
	}

	//работа с Z-буфером
	if(!i)  StatesManager::SetRenderState(D3DRENDERSTATE_ZBIAS,ZBias[i]);
	StatesManager::SetRenderState(D3DRENDERSTATE_ZFUNC,ZFunc[i]);
	if(!i||ZWrite[i]!=ZWrite[i-1])
		StatesManager::SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,ZWrite[i]);

	//тип зацикливания текстуры
	if(!i||MapCoords[i]!=MapCoords[i-1])
		StatesManager::SetTextureStageState(0,D3DTSS_ADDRESS ,MapCoords[i]);

	//настройка модуляции текстура/цвет
	TuneRGBALPHAgen(i,0);

	int toret;
	int Num=0;
	static char t[5]="";
	int _i=i+1;
	//установка текстуры
	if(MapType[i]==STG_ANIMMAP)  Num=((int)(/*Time*!!!*/AnimFreq[i]))%TexNum[i];
	if(MapType[i]!=STG_LIGHTMAP)
	{
		if(MapType[i]!=STG_NULL_MAP)
			StatesManager::SetTexture(0,DxTex[i][Num]);
	}
	else
	{
		IDirectDrawSurface7 *texp = CachedLM.GetLM(0/*LMNum*/);
		if(!texp)		{texp=CachedLM.GetSubstitute();} 
		StatesManager::SetTexture(0,texp);
	}

	//настройка матриц для изменения текстурных координат
	SetTexGenMatrix(i);
	SetTexModMatrix(i);
	if(MapType[i]==STG_LIGHTMAP)
		StatesManager::SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,1);

	//если есть мультитекстурирование
	bool multitexlocked=(MapType[_i]==STG_LIGHTMAP&&LMNum<0);
	bool blendpassed=(!multitexlocked)&&SetBlending(_i);
	if(DoMulti&&(_i<StageNum)&&blendpassed&&!multitexlocked)
	{
		toret=1;
		Num=0;
		if(MapType[_i]==STG_ANIMMAP)  Num=((int)(AnimFreq[_i]))%TexNum[_i];
		if(MapType[_i]!=STG_LIGHTMAP)
		{
			//logFile["graphpipe.log"]("setting texture %s\n",TexMap[_i][Num].c_str());
			if(MapType[_i]!=STG_NULL_MAP)
				StatesManager::SetTexture(1,DxTex[_i][Num]);
			//logFile["graphpipe.log"]("setting texture %s OK\n",TexMap[_i][Num].c_str());
		}
		else
		{
			IDirectDrawSurface7 *texp=CachedLM.GetLM(0 /*LMNum*/);
			if(!texp)		{texp=CachedLM.GetSubstitute();} 
			StatesManager::SetTexture(1,texp);
		}


		SetTexGenMatrix(_i,1);
		SetTexModMatrix(_i,1);

		StatesManager::SetTextureStageState(1,D3DTSS_ADDRESS ,MapCoords[_i]);
		if(MapType[_i]==STG_LIGHTMAP)
			StatesManager::SetTextureStageState(1,D3DTSS_TEXCOORDINDEX,1);
	}
	else  
	{//нет мультитекстурирования
		StatesManager::SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE );
		toret=0;
	}

	//if(i + toret >= StageNum-1 && !Transparent)
	//{
	//	SetupPixelFog();
	//}
	//else
	//{
	//	UnSetupPixelFog();
	//}

	return toret;
}

//=====================================================================================//
//                             DWORD VShader::GetGenCol()                              //
//=====================================================================================//
DWORD VShader::GetGenCol(ColorGen_func &RGBGen,ColorGen_func &AlphaGen,float Time)
{

	DWORD CGcol=0xffffffff;
	switch(RGBGen.type)
	{
	case CG_WAVE:
		{
			float f=VShader::GetWave(RGBGen.wave_form,RGBGen.arg,Time);
			int col=floor(f*255.f);
			col=col<0?0:col>255?255:col;
			CGcol=(col)|(col<<8)|(col<<16);
		} 
		break;
	case CG_VERTEX:
		{
			/*if(Prim->Diffuse)
			CGcol=Prim->Diffuse[vn];*/
		}
		break;
	case CG_NONE: case CG_LIGHTINGDIFFUSE: case CG_IDENTITY:   default:
		//CGcol=0xffffff;
		break;
	}

	switch(AlphaGen.type)
	{
	case CG_WAVE:
		{
			float f=VShader::GetWave(AlphaGen.wave_form,AlphaGen.arg,Time);
			int col=floor(f*255.f);
			col=col<0?0:col>255?0xff000000:(col<<24);
			CGcol|=col;
		} 
		break;
	case CG_VERTEX:
		{
			/*if(Prim->Diffuse)
			Color|=Prim->Diffuse[vn]&0xff000000;*/
			CGcol|=0xff000000;
		}
		break;
	case CG_NONE: case CG_LIGHTINGDIFFUSE: case CG_IDENTITY:   default:
		CGcol|=0xff000000;
		break;
	}
	return CGcol;
}

//=====================================================================================//
//                            int VShader::FlushDynBuffer()                            //
//=====================================================================================//
int VShader::FlushDynBuffer(D3DPRIMITIVETYPE PrimType,const unsigned short *idx,int idxnum,int StartVertex,int VertNum,int i,int lmNum, DynamicVB *DynBuf)
{
	int toret;
	//lmNum=-1;
	if(MapType[i]==STG_LIGHTMAP&&lmNum==-1) return 0;
	LMNum=lmNum;

	toret=ApplyStates(D3DDev,i);

	DWORD col=GetGenCol(RGBGen[i],AlphaGen[i],Timer::GetSeconds());
	//StatesManager::SetRenderState(D3DRENDERSTATE_TEXTUREFACTOR,col);
	StatesManager::SetRenderState(D3DRENDERSTATE_AMBIENT, col);

	if(idxnum)
	{
		D3DDev->DrawIndexedPrimitiveVB(PrimType,
			DynBuf->GetInterface(),StartVertex,VertNum,(LPWORD)idx,idxnum,0);
	}
	else
	{
		D3DDev->DrawPrimitiveVB(PrimType,
			DynBuf->GetInterface(),StartVertex,VertNum,0);
	}

	return toret;
}

//=====================================================================================//
//                            int VShader::FlushDynBuffer()                            //
//=====================================================================================//
int VShader::FlushDynBuffer(D3DPRIMITIVETYPE PrimType,const unsigned short *idx,int idxnum,int StartVertex,int VertNum,int i,DynamicVB *DynBuf, const std::vector<const D3DMATRIX*> &Worlds)
{
	int toret;
	if(MapType[i]==STG_LIGHTMAP) return 0;
	LMNum=-1;
	toret=ApplyStates(D3DDev,i);
	DWORD col=GetGenCol(RGBGen[i],AlphaGen[i],Timer::GetSeconds());
	StatesManager::SetRenderState(D3DRENDERSTATE_AMBIENT, col);
	int wrld_num=Worlds.size();

	if(idxnum)
	{
		for(int i=0;i<wrld_num;i++)
		{
			StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)Worlds[i]);
			D3DDev->DrawIndexedPrimitiveVB(PrimType,
				DynBuf->GetInterface(),StartVertex,VertNum,(LPWORD)idx,idxnum,0);
		}
	}
	else
	{
		for(int i=0;i<wrld_num;i++)
		{
			StatesManager::SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*)Worlds[i]);
			D3DDev->DrawPrimitiveVB(PrimType,
				DynBuf->GetInterface(),StartVertex,VertNum,0);
		}
	}

	return toret;
}
