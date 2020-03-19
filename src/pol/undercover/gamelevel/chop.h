#if !defined(__POLYGON_CHOPPER_INCLUDED__)
#define __POLYGON_CHOPPER_INCLUDED__

class Chopper
{
	static float EPS;

public:
	// �������� ������������� ����������.
	// ������������ ��������:
	//		true - ���� �������� ���� �� ��������� ����� ��������������
	//		false - ���� ������������� ��� ������� ���������
	static bool Chop (const point3 *InPolygon, int InVertsCount,
		const point3 &Normal, const point3 &Origin,
		point3 *OutPolygon, int &OutVertsCount);

private:
	static void AddVertex (point3 *OutPolygon, int &OutVertsCount, 
		bool LastVertex, point3 Pt);
};

#endif