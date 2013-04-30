/*******************************************************************
*                           Fast3DLibrary
*   ����������� ���������� ��� ������ � 2D/3D ��������.
*   �� ������������ OpenGL, DirectX, Glide, SDL, ets.
*   �������� �� Win32API. ��� Borland C++ Builder 6.
*
*                           F3DObject.*
*   ���������� �������� ������ 3D-�������, ���������������
*   �������� �������� � ��������. �������� ������ �����
*   ������ ��������� � ��������� ����������.
*******************************************************************/
#ifndef F3DObjectH
#define F3DObjectH

#include <windows.h>
#include "F3DL.h"
#include "F3DProj.h"

// ######## ����� ������� ������� ########

	// ������������ ������� ��������� �� ��������
#define		F3DOF_USE_LIGHT				1
	// ��������� ���� ��������� ��������� ("�������" ���������)  
#define		F3DOF_USE_COLOR_LIGHT		2
	// ��������� ����������� ��������� �����  
#define		F3DOF_USE_LIGHT_DIR			4


	class CF3DObject
	{
		protected:
			// ������� �������
			double	ViewMatrix[16];

			// ������ ������ � �� �����
			TRIPLE	*data;
			U32		data_len;

            // ����� (��������� ������ � �.�.)
			U32		flags;

			// ��������� ��������� ����� (����� ����������� � �������������)
			// ������������, ���� ���������� ����� F3DOF_USE_LIGHT � F3DOF_USE_COLOR_LIGHT
			COLOR	light_color;		// ����, ��-��������� ����� (0xFF, 0xFF, 0xFF)
			double	light_luminance;	// �������, ��-��������� 0.2
			TRIPLE	light_direction;	// �����������, ��-��������� ������ (0, 0, 1)

			// ��������� ����� ��������� pl � ������ ������������,
			// ���� ��� ����� ���� ��������� mat
			COLOR getPlaneColor(const TRIPLE pl[3], COLOR &mat);

			// �������� �� ��������� ��������� (���� �������)
			// ���� ��������� �����, �� ����� �������� �� ����
			//  � ������ ������������. ��� ����� color!=NULL
			//  � ������ ���� ����� ����������� ����� ���������.
			//  �����, � color ����������� ������������ ���������.  
			bool isPlaneVisible(const TRIPLE pl[3], COLOR *color);

		public:
			CF3DObject();
			~CF3DObject();

			// ���������� ��������� ������� �� ������� data
			virtual void Draw()=0;

            // �������������� ���������� ���������
			virtual void Restore();

			// ���������������� ��������
			virtual void Translate(double dx, double dy, double dz);
			virtual void Scale(double dx, double dy, double dz);
			virtual void Rotate(double dx, double dy, double dz);

            // ������ � �������
			virtual inline U32 GetFlags();
			virtual void SetFlags(U32 f);

            // ������ � ���������� �����
			U32 SetLightColor(COLOR &c);
			COLOR GetLightColor();

			U32 SetLumLight(double &lum);
			double GetLumLight();

			U32 SetDirLight(TRIPLE &d);
			TRIPLE GetDirLight();
	};


#endif // F3DObjectH
