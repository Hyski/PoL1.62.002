//
//так игра видит модуль искусств. интеллекта
//

#ifndef _PUNCH_GAMELOGIC_H_
#define _PUNCH_GAMELOGIC_H_

class GenericStorage;

//
// интерфейс AI игры
//
class GameLogic
{
public:
    //инициализация/деинициализация
    static void Init();
    static void Shut();

    //начало обычной игры
    static void BegNewGame();               
    //начало сетевой игры
    static void BegNetGame();

    //отрисовка чего - либо
    static void Draw();
    //AI думает в этом tick'е
    static void Think();

    //загрузка/сохранение игры
    static void MakeSaveLoad(GenericStorage& st);  
};

#endif // _PUNCH_GAMELOGIC_H_