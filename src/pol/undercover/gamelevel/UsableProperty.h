#pragma once

#include "ObjectProperty.h"

//=====================================================================================//
//                    class UsableProperty : public ObjectProperty                     //
//=====================================================================================//
class UsableProperty : public ObjectProperty
{
public:
	UsableProperty(DynObjectSet *set);

	/// Переключить состояние объекта
	/** @param tau Текущее игровое время
		@return Возвращает время, в которое завершится проигрывание анимации
		        изменения состояния */
	virtual float SwitchState(float tau) { return tau; }
	/// Установить состояние объекта
	/** @param state Результирующее состояние
		@param tau Текущее игровое время
		@return Возвращает время, в которое завершится проигрывание анимации
		        изменения состояния */
	virtual float SetState(float state, float tau) { return tau; }
	/// Возможно ли переключение состояния?
	virtual bool CanSwitchState() const { return true; }
	/// Обновить состояние объекта в связи с изменением текущего времени
	virtual void Update(float tau) { }

	virtual void OnDestruct() { }

	/// Сохранить объект в файл
	virtual void MakeSave(SavSlot &slot) { }
	virtual void MakeLoad(SavSlot &slot) { }

	static bool CanPass(const ipnt2_t &);
};