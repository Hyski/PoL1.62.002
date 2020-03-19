#pragma once

#include "DynObjectSet.h"

class DynObject;

//=====================================================================================//
//                                class ObjectProperty                                 //
//=====================================================================================//
/// Представляет собой свойство объекта, например "разрушаемый", "ящик", "лифт", и т. д.
class ObjectProperty
{
	DynObjectSet *m_objects;

public:
	ObjectProperty(DynObjectSet *);
	virtual ~ObjectProperty();

	/// Возвращает набор объектов, с которыми работает свойство
	DynObjectSet *GetObjects() { return m_objects; }
	const DynObjectSet *GetObjects() const { return m_objects; }

	/// Сохранить объект в файл
	virtual void MakeSave(SavSlot &slot) = 0;
	virtual void MakeLoad(SavSlot &slot) = 0;
};