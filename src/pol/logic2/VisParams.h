#pragma once

//=====================================================================================//
//                                enum VisUpdateEvent_t                                //
//=====================================================================================//
enum VisUpdateEvent_t
{
    UE_NONE,
    
    UE_POS_CHANGE,    //изменение позиции
    UE_FOS_CHANGE,    //изменение поля видимости
    
    UE_INS_MARKER,    //добавление нового маркера
    UE_DEL_MARKER,    //удаление маркера
    
    UE_INS_SPECTATOR, //добавление наблюдателя
    UE_DEL_SPECTATOR, //удаление наблюдателя
    
    UE_MARKER_ON,     //маркер включился
    UE_MARKER_OFF,    //маркер выключился

	UE_POL_UNLIM_MOVES_UPDATE
};

//=====================================================================================//
//                                   class VisParams                                   //
//=====================================================================================//
class VisParams
{
	bool m_visible;

public:
	VisParams() : m_visible(false) {}
	VisParams(bool visible) : m_visible(visible) {}

	bool IsVisible() const { return m_visible; }
	void SetVisible(bool visible) { m_visible = visible; }

	bool operator ==(const VisParams &p) const
	{
		return m_visible == p.m_visible;
	}

	bool operator !=(const VisParams &p) const
	{
		return !(*this == p);
	}

	static const VisParams vpVisible, vpInvisible;
};
