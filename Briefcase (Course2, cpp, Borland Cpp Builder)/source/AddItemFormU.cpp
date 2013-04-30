#include <vcl.h>

#pragma warn -lvc

#include "AddItemFormU.h"
#include "ParamFormU.h"

#pragma resource "*.dfm"
TAddItemForm *AddItemForm;
__fastcall TAddItemForm::TAddItemForm(TComponent* Owner) : TForm(Owner) {}

//---------------------------------------------------------------------------
void __fastcall TAddItemForm::FormShow(TObject *Sender)
{
	NameEdit->Text = "";
    ProfitEdit->Text = "";
    CostEdit->Text = "";

	BitBtn1->SetFocus();
    NameEdit->SetFocus();  
}
//---------------------------------------------------------------------------

/*
  �������������� ������ � ����� �����.
  ��������� - ���� ����������� ��������������.
  � val - ���������. ���� �������� NULL,
  �� ���������� ������ �������� �� ����������� ��������.
*/
bool AnsiString2Int(AnsiString &str, int *val)
{
    char *tmp = new char[strlen(str.c_str())+1];
    strcpy(tmp, str.c_str());
    int i = atoi(tmp);

    if (i == 0)
        if (strcmp(tmp, "0"))
        {
            MessageBox(0, ("������ � ������ ����� [" + str + "]!").c_str(), "������", MB_OK | MB_ICONERROR);
            return false;
        }

    if (val)
    	*val = i;

    return true;
}

void __fastcall TAddItemForm::BitBtn2Click(TObject *Sender)
{
	int cost, profit;

    NameEdit->Text = NameEdit->Text.Trim();
    
	if (NameEdit->Text != "")
    {
    	if (AnsiString2Int(CostEdit->Text, &cost))
        {
	    	if (AnsiString2Int(ProfitEdit->Text, &profit))
    	    {
            	ModalResult = mrOk;
                return; 
        	} else
            	ProfitEdit->SetFocus();            
        } else
        	CostEdit->SetFocus();    	
    } else
    {
        MessageBox(0, "��������� ��������� ����!", "���������", MB_OK | MB_ICONINFORMATION);
    	NameEdit->SetFocus();
    }
}
//---------------------------------------------------------------------------
void __fastcall TAddItemForm::FormPaint(TObject *Sender)
{
	LinearFill(AddItemForm);
}
//---------------------------------------------------------------------------

