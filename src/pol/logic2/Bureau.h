//
// классы для работы с ящиками на уровне
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
//  бюро - в нем находятся все ящики на уровне
class Bureau
{
public:
    ~Bureau();

    //singleton
    static Bureau* GetInst();

    //получить указатель на ящик или 0
    BaseBox* Get(const rid_t& rid);
  
    //уничтожить внутренние структуры
    void Reset();
    //сохранение / загрузка
    void MakeSaveLoad(SavSlot& slot);

    //добавить ящик в список
    void Insert(BaseBox* box);
    //обработать загрузку уровня
    void HandleLevelEntry();
    //обработать уничтожение ящика
    void HandleObjectDamage(const rid_t& rid);

protected:

    Bureau();
    
private:
    typedef std::vector<BaseBox*> box_vec_t;
    box_vec_t m_boxes;

    bool m_fcan_spawn;
};

#endif // _PUNCH_BUREAU_H_