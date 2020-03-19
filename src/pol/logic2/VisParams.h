#pragma once

//=====================================================================================//
//                                enum VisUpdateEvent_t                                //
//=====================================================================================//
enum VisUpdateEvent_t
{
    UE_NONE,
    
    UE_POS_CHANGE,    //��������� �������
    UE_FOS_CHANGE,    //��������� ���� ���������
    
    UE_INS_MARKER,    //���������� ������ �������
    UE_DEL_MARKER,    //�������� �������
    
    UE_INS_SPECTATOR, //���������� �����������
    UE_DEL_SPECTATOR, //�������� �����������
    
    UE_MARKER_ON,     //������ ���������
    UE_MARKER_OFF,    //������ ����������

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
