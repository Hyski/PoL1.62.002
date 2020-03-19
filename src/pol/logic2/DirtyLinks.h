//
// ������� ����� - ����� �� ���. ����� ����������
//

#ifndef _PUNCH_DIRTYLINKS_H_
#define _PUNCH_DIRTYLINKS_H_

struct damage_s;

class GameLevel;
class GraphPipe;
class LevelCase;
class BaseEntity;
class DynObjectSet;

//
// ��������� �� ���������� ������� 
// (��� ����� �������� ������ ����� GameLevel)
//
class BusRoute{
public:

    virtual ~BusRoute(){}

    //����������� ����� ������� ������������
    DCTOR_ABS_DEF(BusRoute)
    //������� ����������/��������
    virtual void MakeSaveLoad(SavSlot& st) = 0;

    enum point_type{
        PT_STOP,    //��������� 
        PT_PATH,    //������� �����
        PT_DEST,    //�������� ���������
        PT_LAST,    //��������� ����� ����
    };

    struct pos_s{
        ipnt2_t m_pos;
        float   m_angle;
    };

    //������ ��� ������� �����
    virtual point_type GetPntType(const ipnt2_t& cur_pos) = 0;
    //������� � ������ ��������
    virtual void GenLoc(bool first, pos_s* pos) = 0;
    //������ ���� ��� ��������
    virtual void GetPath(const ipnt2_t& cur_pos, pnt_vec_t* path) = 0;
};

//
// �������� �� ����� �� ��� ����� �����
//
class CheckPoints{
public:

    virtual ~CheckPoints(){}

    //����������� ����� ������� ������������
    DCTOR_ABS_DEF(CheckPoints)
    //������� ����������/��������
    virtual void MakeSaveLoad(SavSlot& st) = 0;

    struct pos_s{
        ipnt2_t m_pos;
        float   m_angle;
        bool    m_fspecial;

        pos_s() : m_fspecial(false){}
    };

    //������ �����, ����������� �������� ��� ������������
    virtual void GetPos(pos_s* pos) const = 0;
    //��� ��������� ����� � ������?
    virtual bool IsEnd() const = 0;

    //������� � ��������� �����
    virtual void Next() = 0;

    enum first_type{
        FT_USUAL,
        FT_SPAWN,
    };

    //������� � ������ �����
    virtual void First(first_type type = FT_USUAL) = 0;
};

//
// ���� ������������� ����� Win32 API
//
class Win32File : public AbstractFile{
public:

    Win32File(const std::string& fname, unsigned create, unsigned access = GENERIC_READ|GENERIC_WRITE) :   
      m_hfile(CreateFile(fname.c_str(), access, FILE_SHARE_READ, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL)){}

    ~Win32File() { CloseHandle(m_hfile); }
        
    long GetPos(){ return SetFilePointer(m_hfile, 0, NULL, FILE_CURRENT); }

    void Seek(long offset, int origin)
    {
        switch(origin){
        case SEEK_CUR:
            SetFilePointer(m_hfile, offset, NULL, FILE_CURRENT);
            break;

        case SEEK_END:
            SetFilePointer(m_hfile, offset, NULL, FILE_END);
            break;

        case SEEK_SET:
            SetFilePointer(m_hfile, offset, NULL, FILE_BEGIN);
            break;
        }
    }

    size_t Read(void* ptr, size_t size)
    {
        DWORD bytes_read;
        ReadFile(m_hfile, ptr, size, &bytes_read, NULL);
        return bytes_read;
    }

    size_t Write(const void* ptr, size_t size)
    {
        DWORD bytes_written;
        WriteFile(m_hfile, ptr, size, &bytes_written, NULL);
        return bytes_written;
    }

private:

    HANDLE m_hfile;
};

//
// ����� ��� �������� ������ ��������� � ���.�������
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ��������:  ��� �������� �� ������ �������
//            ������ ������ ������ ����������� ������
//          
class LevelFile : public AbstractFile {
public:

    enum open_mode{
        OM_READ,     //������ ������ (������ "r")
        OM_WRITE,    //������ ������ (������ "w")
    };

    LevelFile(AbstractFile* file, const std::string& level, open_mode om);
    ~LevelFile();

    //��������/������������ ������
    void MakeSaveLoad(SavSlot& slot);

    //������ ������� ��������� ��������� � �����
    long GetPos();
    //���������� ��������� ����� �� ����� �������
    void Seek(long offset, int origin);

    //������ / ������ �� �����
    size_t Read(void* ptr, size_t size);
    size_t Write(const void* ptr, size_t size);

private:

    typedef std::map<std::string,size_t> index_map_t;

    //������/������ ������� (�������� ��������� ��������� � ������� ������/������)
    void ReadIndexMap(AbstractFile* file, index_map_t& indexes);
    void WriteIndexMap(AbstractFile* file, index_map_t& indexes);
    
private:

    enum { BUFF_SIZE = 256 };

    long        m_pos;
    open_mode   m_mode;
    data_buff_t m_data;
    std::string m_level;

    AbstractFile* m_file;    
};

//=====================================================================================//
//                                    class BoxInfo                                    //
//=====================================================================================//
// ������� ������ ��������� �� �������� �����
class BoxInfo
{
public:
    BoxInfo(const rid_t& rid = rid_t());

	bool IsStorage() const;
	//��� ���� ������
	bool IsDatabase() const;
	
	//������ ���������� ����� �����
	const std::string& GetName() const;
	//������ �������� ������ ���������
	const std::string& GetItems() const;
	
	int GetWeight() const;
	int GetRespawn() const;
	int GetExperience() const;

private:
    const DynObjectSet *m_case;
};

//
// ������������ ����� ��� �������� ���� ������ � ������� ��������� ���������
//
class BoxSpawner{
public:

    virtual ~BoxSpawner(){}

    //������� �������������� �������
    virtual void CreateDataBox(const rid_t& rid) = 0;
    //������� ���� � ����������
    virtual void CreateStoreBox(const rid_t& rid) = 0;
};

//
// ������� �����
//
namespace DirtyLinks{
	bool IsGoodEntity(BaseEntity *ent);
    //�������� ������ �� GraphPipe
    GraphPipe* GetGraphPipe();
    //�������� ������ �� GameLevel
    GameLevel* GetGameLevel();

    //������ ��� �������� ������
    const std::string& GetLevelSysName();
    std::string GetLevelRawSysName();
    std::string GetLevelFullName(const std::string& sys_name);

    //�������� ��������� ������
    std::string GetStrRes(const rid_t& rid); 

    //�������� ��������� ������� 
    bool GetObjState(const rid_t& rid);
    //���������� ��������� �������
    void SetObjState(const rid_t& rid, bool state);
    //�������� �������� ����� ��� use �� �������
    rid_t GetKey4UseObj(const rid_t& obj_rid);
    //�������� wisdom ��� use �� �������
    int GetWisdom4UseObj(const rid_t& obj_rid);
    //��� ����?
    bool IsElevatorObj(const rid_t& obj_rid);

    //���������� ������ �� ������
    void EraseLevelObject(const rid_t& rid);
    //������ ��� ����� �� ������
    void EnumerateAllLevelCases(BoxSpawner* spawner);
    //��������� ����� HexGrid'� ��� �������� ������
    void CalcObjectHexes(const rid_t& rid, pnt_vec_t* pnts);

    //����� �� ������ �c����������
    bool CanUseObject(const rid_t& obj_rid, std::string* hint);        

    //�������� ����� �� ������, ������ �� �����. ����
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //�������:
    //  true    -   �����
    //  flase   -   ���������
    //
    //bool GetLevelPnt(point3* lev_pnt);

    //��������/��������� ����� ������
    void EnableRoofs(bool flag);

    // ��������� �������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //  ��� �������� ������ ������ ����������
    //  �������� �������� ������.
    // 
    void LoadLevel(const std::string& level);

    enum button_type{
        BT_OK     =   1 << 0,
        BT_CANCEL =   1 << 1,
    };

    //���������� �������� bar'a ��� ��������
    void SetLoadBarPos(float val);

    //���a�� ���������� ������ �������
    point3 GetObjCenter(const rid_t& rid); 

    enum message_type{
        MT_USUAL,   //������� ���������
        MT_DENIAL,  //����� � ��� - ����
    };

    //������� ����� �������
    void Print(const std::string& str, message_type type = MT_USUAL);

    //������ ������ �������� � �. pnt.x, pnt.y
    float GetLandHeight(const point3& pnt);
    //���������� BBox
    void DrawBBox(const BBox& box, unsigned color);
    //����������� ��������� �������
    float SwitchObjState(const std::string& obj_name);

    //������� �������� � �������� ����
    void SendTurnEvent(player_type player, bool fbegin);

    //������ ������ �� ����� � ����� ��������� ������
    bool IsInCameraCone(const point3& point);
    //��������� ������ � �������
    void MoveCamera(const point3& pos, float time = 0.2f);

    //������ ������� ������
    point3 GetCamPos();
    //������� ����� �� ������ � �����������
    point3 PickCam(float x, float y);

    enum opt_type{

        OT_SHOW_PATH,         //���������� ���� ��� unit'�
        OT_SHOW_FRONT,        //���������� ������ ���� ������������
        OT_SHOW_JOINTS,       //���������� ���� ������      
        OT_SHOW_LANDS,        //���������� ����� ������� �� �������
        OT_SHOW_FOS,          //��������� ���������� FOS 
        OT_SHOW_BANNER,       //��������� ���������� Banner

        OT_MOUSE_UPDATE_TIME, //����� ���������� ����
        OT_ENABLE_THEMES,     //��������� ����������� ����������� ���

        OT_ENABLE_HELPER,     //���������� helper?

        OT_HIDDEN_MOVEMENT_ENEMY_RELATED_MOVE,      //���������� ����������� ���������� ����������
        OT_HIDDEN_MOVEMENT_ENEMY_RELATED_SHOOT,     //���������� �������� ���������� ����������
        OT_HIDDEN_MOVEMENT_FRIEND_RELATED_MOVE,     //���������� ����������� ������������� ����������
        OT_HIDDEN_MOVEMENT_FRIEND_RELATED_SHOOT,    //���������� �������� ������������� ����������
        OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_MOVE,    //���������� ����������� ����������� ����������
        OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_SHOOT,   //���������� �������� ����������� ����������

        OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_MOVE,          //���������� ����������� ������ �������
        OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_SHOOT,         //���������� �������� ������ �������
        OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_MOVE,    //���������� ����������� ������ �������
        OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_SHOOT,   //���������� �������� ������ �������

        MAX_OPTIONS,
    };

    //������ � �������
    int GetIntOpt(opt_type type);
    float GetFloatOpt(opt_type type);

    //�������� �������� ��������� ������
    bool GetCmdLineArg(const std::string& arg_name, std::string* value);

    //���������/��������� ��������� ������ 
    void EnableLevelRender(bool flag);
    //��������� ��������� ���������� �������
    void EnableCameraControl(bool flag);
    //���������/�������� ���� �� ������
    void EnableLevelAndEffectSound(bool flag);

    //���������� quick Save/quick Load
    void MakeQuickSaveLoad(GenericStorage::storage_mode mode);

    //������� ���������� �������
    BusRoute* CreateBusRoute(const std::string& sys_name);
    //������� ����� CheckPoints
    CheckPoints* CreateCheckPoints(const std::string& sys_name);
    //������� ����� ����� ��� �������� � �������� ����������
    CheckPoints* CreateCirclePoints(BaseEntity* entity, int radius);
    CheckPoints* CreateCirclePoints(const ipnt2_t& center, int radius);
}

#endif // _PUNCH_DIRTYLINKS_H_