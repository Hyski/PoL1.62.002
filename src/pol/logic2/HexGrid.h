//
// ����� hex'�� ���������������� �� Max'�
//

#ifndef _PUNCH_HEX_GRID_
#define _PUNCH_HEX_GRID_

class RiskSite;
class BaseEntity;

//
// ���'��� ����
//
class HexGrid{
public:

    //singleton
    static HexGrid* GetInst(){return m_instance;}

    //���� ��� ���������� �����
    class hg_slice{
    public:

        size_t operator()(int x, int y) const;
        size_t operator()(const ipnt2_t pnt) const;
        const ipnt2_t& off2pnt(size_t off);

    private:
        friend class HexGrid;
        hg_slice(size_t x) : m_sizeX(x){}
        
        size_t  m_sizeX;
        ipnt2_t m_pnt;
    };

    //������ �����
    class cell: public point3{
    public:

        cell();
        ~cell();

        enum cost_type{
            NONE  = -1,
            LAND  = -2,
        };
       
        //������ ��������� ��������
        int GetProperty() const{ return m_prop; }

        //������ �� ���������� ����
        int  GetCost() const   { return m_cost; }
        void SetCost(int cost) { m_cost = cost; }

        //��� ����� � �����
        bool IsDefPnt() const  { return m_cost == NONE; }
        bool IsLandPnt() const { return m_cost == LAND; }
  
        //����� ���� ��� ������ �� �������
        BaseEntity* GetEntity() const  { return m_entity; }

        //����������/����� �������� � ������ ����
        void UnlinkEntity() { m_entity = 0; }
        void LinkEntity(BaseEntity* ent)
		{
			assert( m_entity == 0 );
			m_entity = ent;
		}

        //��������� �� ������ � ���� dir
        bool IsPassable(int dir) const { return m_weight[dir] < 80; }
        //������ ��������� ������� � ����� �����������
        int GetWeight(int dir) const { return m_weight[dir]; }
        //���������� ��������� ������� � ����� �����������
        void SetWeight(int dir, int val) { m_weight[dir] = val; }

        //�������� �����
        void LandMark() { m_cost = LAND; }
        void Clear()    { m_cost = NONE; }

        //
        //�������� � ����������� ����������
        //
        
        //����� ������ � ������� ��������?
        bool IsBusRoute() const { return m_buses != 0; }

        //��������/��������� �������
        void MarkBusRoute(unsigned route_id)   { m_buses |= (1 << route_id); }
        void UnmarkBusRoute(unsigned route_id) { m_buses &= ~(1 << route_id); }

        //
        // ������ � ��������� ���������
        //

        //�������� ������ �� RiskSite
        RiskSite* GetRisk() { return m_risk; }
        //���������� ������ �� RiskSite
        void SetRisk(RiskSite* risk) { m_risk = risk; }

    private:

        friend class HexGrid;
      
        int m_cost;      // ��� ������� ��������� ���������
        int m_prop;      // id ������� hex'��
        int m_weight[6]; // ������������

        RiskSite*   m_risk;   //������� ������� ����  
        BaseEntity* m_entity; //�������� ����������� � ���� hex'ce

        unsigned  m_buses;    //�������� ��������� ����������� ����� ��� �����
    };

    //�������� �����
    struct prop {
        std::string  m_zone;    //�������� ����
        unsigned     m_joints;  //������� ����� �� ���������

        bool DynMan,DynHeight;
        std::string ManObj;//������, �� �������� ������� ������������
        bool ManInv;       //������������ ������� ��������
        std::string HeightObj;//������, �� �������� ������� ������
        prop():m_joints(0),DynMan(false),DynHeight(false){}
    };

    virtual ~HexGrid();
    
    //�������� �� ����� � ����
    bool IsOutOfRange(const ipnt2_t& pnt) const;

    //����������� �����
    int GetSizeX() const;
    int GetSizeY() const;
    float GetHexSize() const;

    //�������� �������� ��� ���������� �������
    hg_slice GetSlice() const;

    //�������� ������� � ������� �����
	typedef std::vector<cell>::iterator cell_iterator;
	typedef std::vector<prop>::iterator prop_iterator;

	typedef std::vector<cell>::const_iterator const_cell_iterator;
	typedef std::vector<prop>::const_iterator const_prop_iterator;

    //��� ����������� �������� ������� � �������
    cell_iterator first_cell();
    const_cell_iterator first_cell() const;
    cell_iterator last_cell();
    prop_iterator first_prop();
    prop_iterator last_prop();

    //���������, �� ������� ������ � ������
    cell& Get(const ipnt2_t& pnt);
    const cell& Get(const ipnt2_t& pnt) const;
    const prop& GetProp(const ipnt2_t& pnt);

    //���������� ������ ������
    void SetActiveJoints(unsigned val);
    unsigned GetActiveJoints() const;

protected:

    HexGrid(const char* name);

private:

    void ReadStr(std::istringstream &stream, std::string& str);
    
private:

    int      m_SizeX, m_SizeY;   
    float    m_HexSize;
    unsigned m_active_joints;

	typedef std::vector<cell> Grid_t;
	typedef std::vector<prop> Props_t;

	Grid_t m_grid;
	Props_t m_props;

    static HexGrid* m_instance;
    
public: //Grom
  //��������������� ������ ��� �������� ��������� ������������..
  struct heightfront
    {  //����� � ����������� ��/� ������ � ����������� ���������� �������
    cell *from;
    cell *to;
    int dir;
    heightfront(cell *a,cell *b, int d):from(a),to(b), dir(d){;}
    heightfront():from(NULL),to(NULL),dir(0){;}
    };
  typedef std::vector<ipnt2_t> CellVec;
  typedef std::vector<heightfront> HFrontVec;
  std::map<std::string,CellVec> ManDirect;    //Objstate==0 - �����������
  std::map<std::string,CellVec> ManInvert;    //Objstate==1 - �����������
  std::map<std::string,CellVec> HeightDirect; //�������, �� ������� ������� ������
  std::map<std::string,HFrontVec>  HeightFront; //�������, �� ������� ������� ������
};

//
//~~~~~~~~~~~~~~~~~~~~~~~~inlines~~~~~~~~~~~~~~~~~~~~~~~~
//

//=====================================================================================//
//                  inline unsigned HexGrid::GetActiveJoints() const                   //
//=====================================================================================//
inline unsigned HexGrid::GetActiveJoints() const
{
    return m_active_joints;
}

//=====================================================================================//
//              inline HexGrid::prop_iterator HexGrid::first_prop() const              //
//=====================================================================================//
inline HexGrid::prop_iterator HexGrid::first_prop()
{
    return m_props.begin();
}

//=====================================================================================//
//              inline HexGrid::prop_iterator HexGrid::last_prop() const               //
//=====================================================================================//
inline HexGrid::prop_iterator HexGrid::last_prop()
{
    return m_props.end();
}

//=====================================================================================//
//              inline HexGrid::cell_iterator HexGrid::first_cell() const              //
//=====================================================================================//
inline HexGrid::cell_iterator HexGrid::first_cell() 
{
    return m_grid.begin();
}

//=====================================================================================//
//           inline HexGrid::const_cell_iterator HexGrid::first_cell() const           //
//=====================================================================================//
inline HexGrid::const_cell_iterator HexGrid::first_cell() const
{
    return m_grid.begin();
}

//=====================================================================================//
//              inline HexGrid::cell_iterator HexGrid::last_cell() const               //
//=====================================================================================//
inline HexGrid::cell_iterator HexGrid::last_cell() 
{
    return m_grid.end();
}

//=====================================================================================//
//                        inline int HexGrid::GetSizeX() const                         //
//=====================================================================================//
inline int HexGrid::GetSizeX() const
{
    return m_SizeX;
}

//=====================================================================================//
//                        inline int HexGrid::GetSizeY() const                         //
//=====================================================================================//
inline int HexGrid::GetSizeY() const
{
    return m_SizeY;
}

//=====================================================================================//
//                      inline float HexGrid::GetHexSize() const                       //
//=====================================================================================//
inline float HexGrid::GetHexSize() const
{
    return m_HexSize;
}

//=====================================================================================//
//                        inline HexGrid::cell& HexGrid::Get()                         //
//=====================================================================================//
inline HexGrid::cell& HexGrid::Get(const ipnt2_t& pnt)
{
    return *(first_cell() + GetSlice()(pnt));
}

//=====================================================================================//
//                  inline const HexGrid::cell& HexGrid::Get() const                   //
//=====================================================================================//
inline const HexGrid::cell& HexGrid::Get(const ipnt2_t& pnt) const
{
    return *(first_cell() + GetSlice()(pnt));
}

//=====================================================================================//
//                   inline const HexGrid::prop& HexGrid::GetProp()                    //
//=====================================================================================//
inline const HexGrid::prop& HexGrid::GetProp(const ipnt2_t& pnt)
{
    return *(first_prop() + Get(pnt).m_prop);
}

//=====================================================================================//
//                      inline bool HexGrid::IsOutOfRange() const                      //
//=====================================================================================//
inline bool HexGrid::IsOutOfRange(const ipnt2_t& pnt) const
{   
    return pnt.x < 0 || pnt.x >= m_SizeX || pnt.y < 0 || pnt.y >= m_SizeY;
}

//=====================================================================================//
//                  inline size_t HexGrid::hg_slice::operator() const                  //
//=====================================================================================//
inline size_t HexGrid::hg_slice::operator()(const ipnt2_t pnt) const
{
    return pnt.x + pnt.y*m_sizeX; 
}

//=====================================================================================//
//                  inline size_t HexGrid::hg_slice::operator() const                  //
//=====================================================================================//
inline size_t HexGrid::hg_slice::operator()(int x, int y) const
{
    return x + y*m_sizeX; 
}

//=====================================================================================//
//                 inline const ipnt2_t& HexGrid::hg_slice::off2pnt()                  //
//=====================================================================================//
inline const ipnt2_t& HexGrid::hg_slice::off2pnt(size_t off)
{
    m_pnt.x = off%m_sizeX;
    m_pnt.y = off/m_sizeX;
    return m_pnt;
}

//=====================================================================================//
//                 inline HexGrid::hg_slice HexGrid::GetSlice() const                  //
//=====================================================================================//
inline HexGrid::hg_slice HexGrid::GetSlice() const
{
    return hg_slice(m_SizeX);
}

#endif //_PUNCH_HEX_GRID_