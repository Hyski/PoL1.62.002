// SimpleTexturedObject.h: interface for the SimpleTexturedObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLETEXTUREDOBJECT_H__6768B6C7_51FF_4B3E_8364_6EDFA0894213__INCLUDED_)
#define AFX_SIMPLETEXTUREDOBJECT_H__6768B6C7_51FF_4B3E_8364_6EDFA0894213__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class SavSlot;
class OptSlot;

//=====================================================================================//
//                             class SimpleTexturedObject                              //
//=====================================================================================//
class SimpleTexturedObject //Кусок кожи, имеющий один материал
{
public:
	void FreeAll();
	void Alloc(int _PntNum, int _IdxNum)
	{
		prim.Verts.resize(_PntNum);
		prim.Idxs.resize(_IdxNum);
	}
	
	std::string MaterialName;     //имя материала для этого куска

	//    int PntNum;             //количество вершин/нормалей/... в этом куске
	//    int IdxNum;             //количество индексов
	//    point3 *Points;
	//    point3 *Normals;
	//    texcoord *uv;
	Primi prim;
	// 
	//private:
	//short int *indexes;     //индекы, создающие грани
public:
	// создадим функции для доступа к индексам - на чтение и на чтение/запись
	short int * GetIndexesFull();
	const short int * const GetIndexesRO();
	typedef short int * short_int_ptr;


	//void Load(FILE *f);
	//void Save(FILE *f);
	bool MakeSaveLoad(SavSlot &slot);
	void Load(OptSlot &slot);
	void Save(OptSlot &slot);

	SimpleTexturedObject &operator=(const SimpleTexturedObject &a)
	{
		if (this==&a) return *this;
		FreeAll();
		MaterialName=a.MaterialName;
		prim = a.prim;
		return *this;
	}
	virtual ~SimpleTexturedObject()
	{
		FreeAll();
	}
	SimpleTexturedObject()
	{
		prim.Prim = Primi::TRIANGLE;
		prim.Contents = Primi::NEEDTRANSFORM;
	}

	SimpleTexturedObject(const SimpleTexturedObject &a)
	{
		prim.Prim = Primi::TRIANGLE;
		prim.Contents = Primi::NEEDTRANSFORM;
		*this=a;
	}

};

//=====================================================================================//
//                                   class TexObject                                   //
//=====================================================================================//
class TexObject
{
protected:
	typedef SimpleTexturedObject STObject;

public:
	STObject *Parts[500];//максимум 500 кусков у объектов
	int PartNum;
public:
	TexObject() : PartNum(0)
	{
	}

	virtual ~TexObject()
	{
		Release();
	}

	//void Load(FILE *f);
	//void Save(FILE *f);
	bool MakeSaveLoad(SavSlot &slot);
	void Load(OptSlot &slot);
	void Save(OptSlot &slot);
	bool TraceRay(const ray &r, float *Pos, point3 *Norm) const;

	//освобождение занимаемой памяти
	void Release()
	{
		for(int i = 0; i < PartNum; ++i)
		{
			delete Parts[i];
			Parts[i] = 0;
		}

		PartNum = 0;
	}

	TexObject &operator =(const TexObject &a);

	// by Flif !!!
	TexObject(const TexObject &a) : PartNum(0) { *this = a; }
	BBox GetBBox() const;
};


#endif // !defined(AFX_SIMPLETEXTUREDOBJECT_H__6768B6C7_51FF_4B3E_8364_6EDFA0894213__INCLUDED_)
