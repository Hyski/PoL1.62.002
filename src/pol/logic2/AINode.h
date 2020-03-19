#pragma once

//=====================================================================================//
//                                    class AINode                                     //
//=====================================================================================//
/// Все AI представлено в виде дерева из AINode
class AINode
{
public:
    virtual ~AINode() {}

    DCTOR_ABS_DEF(AINode)

    //в этой функции узел должен подумать
    virtual float Think(state_type* st) = 0;
    //сохранение/востановление AI
    virtual void MakeSaveLoad(SavSlot& st) = 0;
	// расчет полной сложности узла
	virtual float getComplexity() const = 0;

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return 0; }
	// пошлем уведомление об убийстве детей с заданным ID и заставить их
	// сделать тоже самое со своими детьми
	virtual void deleteChildren(eid_t ID) { }
	// нужно ли удалять узел
	virtual bool need2Delete() const { return false; }
	// умри!
	virtual bool die() { return false; }
};
