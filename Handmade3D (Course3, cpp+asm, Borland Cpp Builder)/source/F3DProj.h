/*******************************************************************
*                           Fast3DLibrary
*   ����������� ���������� ��� ������ � 2D/3D ��������.
*   �� ������������ OpenGL, DirectX, Glide, SDL, ets.
*   �������� �� Win32API. ��� Borland C++ Builder 6.
*
*                            F3DProj.*
*   ������, ������������� �� ������� �������� ����������������
*   ���������� (�������� � ���������, �������������, ets.)
*******************************************************************/
#ifndef F3DProjH
#define F3DProjH

#include <windows.h>
#include "F3DL.h"

// �������� ���������� ������
#define camera_focus 5

const MAT_WIDTH		= 4;
const MAT_HEIGHT	= 4;
const MAT_SIZE		= MAT_WIDTH * MAT_HEIGHT;

//###########################################################################################################
//####################################### ������� ���������� ################################################

		// ������� ��������� �������
	U32 f3d_MakeSingle(double m[MAT_SIZE]);

		// ����������� �������
	U32 f3d_CopyMatrix(const double s[MAT_SIZE], double d[MAT_SIZE]);

		// ������������ ������: AB = A*B
	U32 f3d_MulMatrix(const double A[MAT_SIZE], const double B[MAT_SIZE], double AB[MAT_SIZE]);

		// ������������� ����� P � ����� R (R.z == P.z)
	U32 f3d_Project(const TRIPLE *P, TRIPLE *R);

		// �������������� ��������� ����� P �� ���������� ������� ��������� (�.�.) � ��������� R �� ������ ������� ���� VM
	U32 f3d_Localize(const TRIPLE *P, const double VM[MAT_SIZE], TRIPLE *R);

    	// ������� �.�. VM � ����� �.�. VMNew
	U32 f3d_Translate(const double VM[MAT_SIZE], double VMNew[MAT_SIZE], TRIPLE d);

    	// ��������������� �.�. VM � �.�. VMNew � �������������� d
	U32 f3d_Scale(const double VM[MAT_SIZE], double VMNew[MAT_SIZE], TRIPLE d);

    	// ������� �.�. VM �� ���� d � ����� �.�. VMNew
	U32 f3d_Rotate(const double VM[MAT_SIZE], double VMNew[MAT_SIZE], TRIPLE d);

    	// ���������� ������� ����� ��������� ������������ ��������
	U32 f3d_Normal(const TRIPLE *a, const TRIPLE *b, bool normalize, TRIPLE *n);

    	// ���������� ���� ����� ����� ���������
	U32 f3d_Angle(const TRIPLE *a, const TRIPLE *b, double *angle);

#endif // F3DProjH
