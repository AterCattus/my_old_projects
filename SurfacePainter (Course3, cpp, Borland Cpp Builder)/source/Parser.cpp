#include "Parser.h"

//######################################################################################################################
int Prior(EToken tok)
{
	switch (tok)
	{
		case tAdd	: return 1;
		case tSub	: return 1;
		case tMul	: return 2;
		case tDiv	: return 2;
		case tMod	: return 2;

		case tSin	: return 3; // ?
		case tCos	: return 3; // ?
		case tTan	: return 3; // ?

		default		: return 0; // ����� ������ ������ "����������� ���������"
	}
}

//######################################################################################################################
int VarCount(EToken tok)
{
	switch (tok)
	{
		case tAdd	: return 2;
		case tSub	: return 2;
		case tMul	: return 2;
		case tDiv	: return 2;
		case tMod	: return 2;

		case tSin	: return 1;
		case tCos	: return 1;
		case tTan	: return 1;

		default		: return 0;
	}
}

//######################################################################################################################
SInfo Info(EToken tok, float f)
{
	SInfo info;
	info.token = tok;
	info.data = f;

	return info;
}

//######################################################################################################################
CStack::CStack()
{
	top = NULL;
}

//----------------------------------------------------------------------------------------------------------------------
CStack::CStack(const CStack &v)
{
	SItem *cur = v.top;

	top = NULL;

	while (cur)
	{
		add_last(cur->info);
		cur = cur->next;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CStack::~CStack()
{
	while (!empty()) pop();
}

//----------------------------------------------------------------------------------------------------------------------
void CStack::push(SInfo i)
{
	SItem *cur = new SItem;
	cur->info = i;
	cur->next = top;
	top = cur;
}

//----------------------------------------------------------------------------------------------------------------------
void CStack::add_last(SInfo i)
{
	SItem *cur = top, *prev=NULL;
	while (cur)
	{
		prev = cur;
		cur = cur->next;
	}

	cur = new SItem;
	cur->info = i;
	cur->next = NULL;

	((prev) ? (prev->next) : top) = cur;
}

//----------------------------------------------------------------------------------------------------------------------
SInfo CStack::pop()
{
	if (empty()) return Info(tNothing, 0);

	SInfo val = top->info;
	SItem *cur = top;
	top = top->next;
	delete cur;

	return val;
}


//----------------------------------------------------------------------------------------------------------------------
SInfo CStack::read()
{
	if (!empty()) return top->info;

	return Info(tNothing, 0);
}

//----------------------------------------------------------------------------------------------------------------------
bool CStack::empty()
{
	return (top==NULL); // ��� ����������, ��� ������ "return top;"
}

//######################################################################################################################
CParser::CParser()
{
	vars = NULL;
	ops = NULL;
}

//----------------------------------------------------------------------------------------------------------------------
CParser::~CParser()
{
	DoneCurEval();
}

//----------------------------------------------------------------------------------------------------------------------
EToken CParser::getNextToken(char *buf, int &pos)
{
	char ch;

	while ( (ch = buf[pos]) == ' ') pos++;

	if (isalpha(ch)) // � ������ ������: 'x', 'z', 'sin', 'cos', 'tan'
	{
		switch (ch) // �����, �� ������� �)
		{
			case 'x':
			{
				pos++;
				return tX;
			}

			case 'z':
			{
            	pos++;
				return tZ;
			}

			default: // � ��������� ���������� 3�-���������� (+PI, ������ ������)
			{
				char tmp[4];
				strncpy(tmp, &buf[pos], 3);
				tmp[3] = 0; // ?

				if (isalpha(buf[pos+3])) return tNothing;

				if (!strcmp(tmp, "sin"))
				{
					pos += 3;
					return tSin;
				}
				if (!strcmp(tmp, "cos"))
				{
					pos += 3;
					return tCos;
				}
				if (!strcmp(tmp, "tan"))
				{
					pos += 3;
					return tTan;
				}

				// ������ ������� ����������� � PI

				if (isalpha(buf[pos+2])) return tNothing;

				if (!strncmp(tmp, "pi", 2))
				{
					pos += 2;
					data = M_PI;
					return tConst;
				}

			} // default
		} // switch

		return tNothing; // ������

	}
	else
	if (isdigit(ch)) // � ������ �����: ������ �����
	{
		char *endptr;
		data = strtod(&buf[pos], &endptr);
		pos += (endptr-&buf[pos]);

		return tConst;

	}
	else
	{
		pos++;

		switch (ch)
		{
			case '+': return tAdd;
			case '-': return tSub;
			case '*': return tMul;
			case '/': return tDiv;
			case '%': return tMod;

			case '(': return tBeg;
			case ')': return tEnd;

			default : return tNothing;
		}
	}

    return tNothing;
}

//----------------------------------------------------------------------------------------------------------------------
bool CParser::callStacks(CStack *op, CStack *var)
{
	float op1, op2;
	SInfo info;
	
	do {
		info = op->pop();

		switch (info.token) {
			case -1:
				return true; // ���� ���������� ����

			case tBeg:
				return true; // ����� �� ����� �����

			case tAdd:
				op1 = var->pop().data;
				op2 = var->pop().data;
				var->push(Info(tConst, op1+op2));
				break;

			case tSub:
				op1 = var->pop().data;
				op2 = var->pop().data;
				var->push(Info(tConst, op2-op1));
				break;

			case tMul:
				op1 = var->pop().data;
				op2 = var->pop().data;
				var->push(Info(tConst, op2*op1));
				break;

			case tDiv:
				op1 = var->pop().data;
				op2 = var->pop().data;
				if (!op1) return false; // ������� �� 0
				var->push(Info(tConst, op2/op1));
				break;

			case tMod:
				op1 = var->pop().data;
				op2 = var->pop().data;
				if (!op1) return false; // ������� �� 0
				var->push(Info(tConst, int(op2)%int(op1)));
				break;

			case tSin:
				op1 = var->pop().data;
				var->push(Info(tConst, sin(op1)));
				break;

			case tCos:
				op1 = var->pop().data;
				var->push(Info(tConst, cos(op1)));
				break;

			case tTan:
				op1 = var->pop().data;
				var->push(Info(tConst, tan(op1)));
				break;

			default:
				return false; // ������

		} // switch

	} while (1);
}

//----------------------------------------------------------------------------------------------------------------------
void CParser::InitNewEval(float x, float z)
{
	DoneCurEval();

	// VARIABLES
	vars = new CStack();

	SItem *cur = variables.top;
	while (cur)
	{
		if (cur->info.token == tX)
		{
			vars->push(Info(tConst, x));
		}
		else
		if (cur->info.token == tZ)
		{
			vars->push(Info(tConst, z));
		}
		else
			vars->push(cur->info);

		cur = cur->next;

	} // while (cur)


	// OPERATORS
	ops = new CStack();

	cur = operators.top;
	while (cur)
	{
		ops->push(cur->info);
		cur = cur->next;
	}

}

//----------------------------------------------------------------------------------------------------------------------
void CParser::DoneCurEval()
{
	if (vars) delete vars;
	vars = NULL;
	if (ops) delete ops;
	ops = NULL;
}

//----------------------------------------------------------------------------------------------------------------------
bool CParser::Calc()
{ // ���������� �������� ��� �������� ���������� ����������
  // ��������� ��� �����, �� ���, ��-�����, �������� 
	int pos = 0;
	EToken tok;

	while (pos<len)
	{
		tok = getNextToken(str, pos);
		if (tok == tNothing) return false; // ����� ������ �������

		if (tok >= tAdd) // ������� ��������
		{
			operators.push(Info(tok, 0));
			continue;
		}

		if (tok == tConst) // ������� ���������
		{
			variables.push(Info(tok, data));
			continue;
		}

		if ((tok == tBeg) || (tok == tEnd)) // ������ ��� ����� �����
		{
			if (tok == tEnd)
			{
				SortBlock(&operators, &variables);
				variables.push(Info(tok, 0)); // ��� � �� �����m�� � ����������� ������� � ����� ���������
			}

			operators.push(Info(tok, 0));

			continue;
		}

		if ((tok == tX) || (tok == tZ)) // ����������
		{
			variables.push(Info(tok, 0));
		}

	} // while
}

//----------------------------------------------------------------------------------------------------------------------
void swap_tokens(SItem *t1, SItem *t2)
{ // ����� �������

	SInfo i = t1->info;
	t1->info = t2->info;
	t2->info = i;
}

SItem* PtrPlusNum(SItem *ptr, int num)
{ // ��������� ���������, ������� "ptr += num"

	if (!ptr || (num<0)) return NULL;

	SItem *c = ptr;
	while (num-- && c) c = c->next;

	return c;
}

void move(SItem *from, int count, SItem *to, SItem *&top)
{ // ����������� ����� ������� � ����� �� ������ ����� � ������

	if (!from || (count<=0)) return;

	SItem *last = PtrPlusNum(from, count);

	SItem *first = from->next;
	from->next = last->next;

	if (to)
	{
		to->next = first;
		last->next = to->next;
	}
	else
	{
		last->next = top;
		top = first;
	}
}

#define PRIOR(i) Prior((i)->info.token)
#define TOKEN(i) (i)->info.token

void CParser::SortBlock(CStack *op, CStack *var)
{
	SItem *ptrOp, *ptrVar, *ptrVarPrev;
	bool moved;

	if (!op) return;

	do
	{
		moved = false;
		ptrOp = op->top; ptrVar = var->top; ptrVarPrev = NULL;

		while (ptrOp->next) // �� ����� �����
		{
			if (TOKEN(ptrOp) == tBeg) break; // ����� �����

		/* 1 */
			if (PRIOR(ptrOp) >= PRIOR(ptrOp->next))
			{
				ptrOp = ptrOp->next;
				ptrVarPrev = ptrVar;
				ptrVar = PtrPlusNum(ptrOp, VarCount(TOKEN(ptrOp)));
				continue;
			}

		/* 2 */
			swap_tokens(ptrOp, ptrOp->next);

		/* 3 */
			int pos = VarCount(TOKEN(ptrOp))-1;
			pos = max(pos, 1)-1;
			move(	PtrPlusNum(ptrVar, pos),		// ������
					VarCount(TOKEN(ptrOp->next)),	// �����
					ptrVarPrev,					 	// ����
					var->top );

		/* 4 */
			moved = true;
			break;

		} // while (ptrOp->next)

	} while (moved); // ���� �� ����� ������������
}

//----------------------------------------------------------------------------------------------------------------------
bool CParser::Parse(char *path)
{
	ifstream f(path, ios::in);
	if (!f) return false;

	f.getline(str, 100);
	f.close();

	len = strlen(str);

	MessageBox(NULL, str, "������� �����������", MB_OK | MB_ICONINFORMATION);

	return Calc();
}

//----------------------------------------------------------------------------------------------------------------------
bool CParser::Eval(float x, float z, float &y)
{
    // ������������� ����� ��� ������� �������� �����
	InitNewEval(x, z);

	CStack var_cut;
	CStack op_cut;

	SItem *cur_var = vars->top;
	SItem *cur_op = ops->top;

	while (cur_op)
	{
		if (cur_op->info.token == tEnd)
		{
			// �������� ��������� ����, �������� ���
			if (!callStacks(&op_cut, &var_cut))
			{
				MessageBox(NULL, "������ ����������", NULL, MB_OK | MB_ICONERROR);
				DoneCurEval();
				return false;
			}
			if (cur_var) cur_var = cur_var->next;
			cur_op = cur_op->next;
			continue;
		}

		if (cur_var && (cur_var->info.token != tEnd)) var_cut.push(cur_var->info);
		op_cut.push(cur_op->info);

		cur_op = cur_op->next;
		if (cur_var && (cur_var->info.token != tEnd)) cur_var = cur_var->next;
	}

    // ���� �������� �������������� ���������, �������� �������� ������
	if (!op_cut.empty())
	{
		if (!callStacks(&op_cut, &var_cut))
		{
			MessageBox(NULL, "������ ����������", NULL, MB_OK | MB_ICONERROR);
			DoneCurEval();
			return false;
		}
	}

	// ��������, ��� ������� ���� ������ ������������ ���������� (e.g. "1.25") � ������ �� �����������,
	// ����� ������ ������� �������� ���� ��������� � ���������
	if (!var_cut.empty())	y = var_cut.top->info.data;
	else					y = vars->top->info.data;

	DoneCurEval();

	return true;
}
