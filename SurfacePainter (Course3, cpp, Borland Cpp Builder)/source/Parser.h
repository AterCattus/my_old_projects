#ifndef ParserH
#define ParserH

#include <fstream.h>
#include <windows.h>
#include <ctype.h>
#include <math.h>

    // ��������� ������ ������ �������
	enum EToken
	{
        tNothing = -1,

		tBeg,		// "("
		tEnd,		// ")", ������� �� ��������� � ����� ����������

		tX,			// "x"
		tZ,			// "z"

		tConst,		// FPC

		tAdd,		// +
		tSub,		// -
		tMul,		// *
		tDiv,		// /
		tMod,		// %

		tSin,		// sin
		tCos,		// cos
		tTan		// tan
	};

    // ���������-��������� ������
	struct SInfo
	{
		// ���
		EToken	token;
		// ��������� �� ����������� ������
		float	data;
	};

	SInfo Info(EToken tok, float f);

	// ���� ������� ������ �������
	struct SItem
	{
		SInfo	info;
		SItem	*next;
	};

// #### ����������� ���������� ������ ####
	int Prior(EToken tok);

// #### ����������� ���-�� ��������, ����������� ��� ��������� � ����� ��������� ####
// { ���������� ��� ���������� ����� ���������� � ������ �� ����������� }   
	int VarCount(EToken tok);


// #### ���� (��� ������������ :) ####
	class CStack
	{
		private:
			SItem	*top;

		public:
			CStack();
			CStack(const CStack &v);
			~CStack();
			void push(SInfo i);
			void add_last(SInfo i);
			SInfo pop();
			SInfo read();
			bool empty();

            friend class CParser;
	};


// #### ������ � ���������� ��������� ####
	class CParser
	{
		private:
			char str[100];
			int len;

			// ��������� ����� ��������� � ����������
			CStack	*vars;
			CStack	*ops;

            // ��������� ������� ������
			CStack	variables;
			CStack	operators;

			// ��� ���������� getNextToken ���������, ����� ����������� �� ��������
			float	data;

			// ��������� ���������� ������ �� ������ �������
			EToken getNextToken(char *buf, int &pos);

			// ���������� ������ ����� ����������
			// ���� � ����� ops: "+(*+(/--+", �� ���������� ������ "/--+"
			// � ��������� "(" �������� �� �����
			bool callStacks(CStack *op, CStack *var);

			// ����������� ����������� 'variables' � 'operators' �
			// 'vars' � 'ops' �������������� � ������ �������� 'X' � 'Z'
			void InitNewEval(float x, float z);

			// �������� �� ������ vars � ops
			void DoneCurEval();

            // ���������� ���������� ��������
			bool Calc();

            // ���������� ���������� � ��������������� ��������� ��� ����� �����������
			void SortBlock(CStack *op, CStack *var);

		public:
			CParser();
			~CParser();

			// �������� �������
			bool Parse(char *path);
			// ���������� Y �� �������� X,Z
			bool Eval(float x, float z, float &y);
	};

#endif // ParserH
