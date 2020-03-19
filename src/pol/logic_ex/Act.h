#pragma once

namespace PoL
{

//=====================================================================================//
//                           class Act : public MlRefCounted                           //
//=====================================================================================//
/// Один акт скриптовой сцены
class Act : public MlRefCounted
{
public:
	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() = 0;
	/// Начался ли акт
	virtual bool isStarted() const = 0;
	/// Начать акт
	virtual void start() = 0;
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau) = 0;
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const = 0;
	/// Может ли действие закончиться
	virtual bool isEndable() const { return true; }
	/// Содержит ли акт выигрыщ или проигрыш
	virtual bool hasEndGame() const { return false; }
};

}