//
// ������ ��� ������ � ������� �� ������
//

#ifndef _PUNCH_BUREAU_H_
#define _PUNCH_BUREAU_H_

class BoxInfo;
class DataBox;
class StoreBox;
class BaseThing;
class BaseBox;

//=====================================================================================//
//                                    class Bureau                                     //
//=====================================================================================//
//  ���� - � ��� ��������� ��� ����� �� ������
class Bureau
{
public:
    ~Bureau();

    //singleton
    static Bureau* GetInst();

    //�������� ��������� �� ���� ��� 0
    BaseBox* Get(const rid_t& rid);
  
    //���������� ���������� ���������
    void Reset();
    //���������� / ��������
    void MakeSaveLoad(SavSlot& slot);

    //�������� ���� � ������
    void Insert(BaseBox* box);
    //���������� �������� ������
    void HandleLevelEntry();
    //���������� ����������� �����
    void HandleObjectDamage(const rid_t& rid);

protected:

    Bureau();
    
private:
    typedef std::vector<BaseBox*> box_vec_t;
    box_vec_t m_boxes;

    bool m_fcan_spawn;
};

#endif // _PUNCH_BUREAU_H_