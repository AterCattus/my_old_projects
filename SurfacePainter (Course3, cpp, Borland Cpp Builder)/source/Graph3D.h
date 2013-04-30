#ifndef Graph3DH
#define Graph3DH

#include <vcl.h>
#include <windows.h>
#include <math.h>

const FLOAT_16 = (16*sizeof(float));

// ��������� �����������
const int SurfX = 60; // �� ����� ������ 3,
const int SurfZ = 60; // ����� �������� 
const float stepX = 2.0/(SurfX-1);
const float stepZ = 2.0/(SurfZ-1);

struct FPOINT
{
	float x, y, z;
};

struct IPOINT
{
	int x, y;
};

// ��� ������� ��������� ����� �����������
typedef float(*SURF_FUNC)(float x, float z);

inline FPOINT FPoint(float x, float y, float z);
inline IPOINT IPoint(int x, int y);

#define CopyMatrix(s, d) CopyMemory((void*)(d), (void*)(s), FLOAT_16)

	class CGraph3D
	{
		protected:
				// ���� �����������
			FPOINT surface[SurfX][SurfZ];
			
				// ������� ������� ����
			float ViewMatrix[16];

				// �������� ������ � ������ ������� ������
			int W2, H2;

				// ��������� � ����������� �������
			FPOINT cam_pos, cam_ang;

            	// ����� ������
			float focus;

				// �� ��� ��������
			TCanvas *Canvas;

				// ������������� ����� �� ��������� ������� ��������� �� ��������� ������
			void ProjectPoint(FPOINT *p1, int &x, int &y);
				// ������� ��� -> ���
			void Global2Local(FPOINT *s, FPOINT *d);
				// ������������ ������ r = m1*m2
			void MulMatrix(const float m1[16], const float m2[16], float r[16]);
				// ��������� �������
			void _Translate_(FPOINT &d);
				// ��������� �������
			void _Rotate_(FPOINT &d);
				// ������������ ��������� �������
			void MakeSingle(float m[16]);
				// ������� ������� ��������� ����� ����� � �������� ����������
			void Point3D_To_ScreenCoords(const FPOINT *p, FPOINT *loc, int &x, int &y);
				// ����� �������������� �����
			void HLine(int y, int x1, int x2, DWORD color);
				// ���������� ������� � ������������
			void Normal(FPOINT *p1, FPOINT *p2, FPOINT *p3, FPOINT *n);
				// ����������� ����� ������������ � ����������� �� ���� ����� ��� �������� � ������������ �������
			DWORD TrColor(FPOINT *p1, FPOINT *p2, FPOINT *p3);
				// ����� ������������
			void Triangle(IPOINT *p1, IPOINT *p2, IPOINT *p3, DWORD color);
				// ������� �������� ������� ����������� ��� �������� ����������� �������
			void ChangeIndexies_Painter(int &x, int &z);

		public:
			CGraph3D(TCanvas *c);
			~CGraph3D();

				// ����� ���� ���������� � ���������
			void ReInit();
				// ��������� �������� ������� ������
			void ChangeBounds(int w, int h);

				// ������������ �����������
			void InitSurface(SURF_FUNC func);
				// ����� �����������
			void DrawSurface();

				// ������� ����������� �� ��������� ����������
			void Translate(float x, float y, float z);
				// ������� ����������� �� ��������� ����������
			void Rotate(float x, float y, float z);
	};

#endif // Graph3DH
