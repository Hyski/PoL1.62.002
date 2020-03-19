/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description: �����, ����������� �������� �� ������, �����, � �.�.                                                                 


Author: Alexander Sorokin aka Grom                                                                     
***************************************************************/                



#if !defined(AFX_MARK_H__1B53F7E0_49D0_11D4_A0E0_525405F0AA60__INCLUDED_)
#define AFX_MARK_H__1B53F7E0_49D0_11D4_A0E0_525405F0AA60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////
//
// ������� ����� ��������
//
////////////////////////////////////////////////////////////////////////////
class BaseMark
{
public:
	enum{MAXTRIS=1000};
	BaseMark(const point3& Position, const float Radius, const std::string& Shader);
	virtual ~BaseMark() {FindTris=true;}
	// �������
	virtual void Update(const float CurTime) = 0;// ���������� ���������

	// ������
	bool Dead;// ���� ���� - true - ����� ���������
	bool FindTris;// ���� ���������� ������ ������� ��������� , �� true
	unsigned int Col;// ������� ����
	bool Visible;// ���������
	bool Savable;//������ �� ������ �����������

	point3 Pos;// �������
	float Rad;// ������
	//	int TriNum;// ���������� �������������
	//	point3 Tris[MAXTRIS*3];// ������ ������ �������������
	//	unsigned int Color[MAXTRIS*3];// ����� ������
	//	texcoord UVs[MAXTRIS*3];  //���������� ���������� ������
	Primi prim;
	std::string ShaderName;//��� �������

	unsigned int GetTriangleCount() const { return prim.Verts.size()/3; }
	unsigned int GetVertexCount() const { return prim.Verts.size(); }
};

////////////////////////////////////////////////////////////////////////////
//
// ����� ���������� �������� � ������������� ���������
//
////////////////////////////////////////////////////////////////////////////
class Mark : public BaseMark
{
public:
	// �������

	// �����������
	Mark(const point3& Position, const float Radius, const std::string& Shader) :
	  BaseMark(Position, Radius, Shader), dynamic(false){Savable=true;}
	  // ���������� ���������
	  virtual void Update(const float )
	  {  
		  if(dynamic) FindTris = true;
		  else FindTris = false;
	  }

	  // ����, ������������ �������� �� ��������
	  bool dynamic;
};

////////////////////////////////////////////////////////////////////////////
//
// ����� ��������� ��������
//
////////////////////////////////////////////////////////////////////////////
class Flicker : public Mark
{
public:
	// �������

	// �����������
	Flicker(const point3& Position, const float Radius, const std::string& Shader) :
	  Mark(Position, Radius, Shader){}
	  // ���������� ���������
	  virtual void Update(const float CurTime);
};

////////////////////////////////////////////////////////////////////////////
//
// ����� �������
//
////////////////////////////////////////////////////////////////////////////
class Splash : public BaseMark
{
public:
	// �������

	// �����������
	Splash(const point3& Position, const float Radius, const std::string& Shader,
		const point3& Color, const float BirthTime, const float LifeTime);
	// ���������� ���������
	virtual void Update(const float CurTime);

	// ������
	float birthtime;// ����� ��������
	float lifetime;// ����� �����
	point3 start_color;// ��������� ����
};

////////////////////////////////////////////////////////////////////////////
//
// ����� ��������� ��������
//
////////////////////////////////////////////////////////////////////////////
class DMark: public Splash
{
public:
	// �������

	// �����������
	DMark(const point3& Position, float Radius,
		const std::string& Shader,	const point3& Color,
		float BirthTime, float LifeTime,
		float SwitchTime, unsigned int StartColor) :
	Splash(Position, Radius, Shader, Color, BirthTime, LifeTime)
	{ switchtime = SwitchTime; Col = StartColor; }
	// ���������� ���������
	virtual void Update(const float CurTime);
	// ������
	float switchtime;// ����� ��������� � �������
};

/////////////////////////////////////////////////////////////////////////
// ��� ��� �������
/////////////////////////////////////////////////////////////////////////
class MarksPool
{
public:
	class iterator;
	friend class iterator;
	void clear(); //������� ������
	void Draw();
	void Update(float Time);
	void Add(boost::shared_ptr<BaseMark> Mark){m_Marks.push_back(Mark);}
	void MakeSaveLoad(SavSlot &slot);
	//�������� ��������
	iterator begin();
	iterator end();
	//��������/�����������
	MarksPool(){;};
	~MarksPool();
private:
	typedef std::list< boost::shared_ptr<BaseMark> > MarksSet;
	MarksSet m_Marks;
};
class MarksPool::iterator
{
public:
	iterator& operator++(){if(m_Cur!=m_End)m_Cur++;return *this;};//++i
	iterator operator++(int){iterator i(*this);	if(m_Cur!=m_End)m_Cur++;return i;};						
	bool operator !=(const iterator &a)const {return m_Cur!=a.m_Cur;};
	boost::shared_ptr<BaseMark> operator->()const {return *m_Cur;};
	boost::shared_ptr<BaseMark> operator*()const {return *m_Cur;};
	iterator(const MarksSet::iterator &beg,const MarksSet::iterator &end,const MarksSet::iterator &cur)
		:m_Beg(beg),m_End(end),m_Cur(cur){};
private:
	MarksSet::iterator m_Beg,m_End,m_Cur;
};
inline MarksPool::iterator MarksPool::begin()
{return iterator(m_Marks.begin(),m_Marks.end(),m_Marks.begin());}
inline MarksPool::iterator MarksPool::end()
{return iterator(m_Marks.end(),m_Marks.end(),m_Marks.end());}

#endif // !defined(AFX_MARK_H__1B53F7E0_49D0_11D4_A0E0_525405F0AA60__INCLUDED_)
