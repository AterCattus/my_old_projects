Attribute VB_Name = "DBConst"
' ���������� ������ �������� �� MsgBox
Public Const resBad = 0     '   �����, ��������� ����
Public Const resOk = 1      '   ��
Public Const resNo = 2      '   ���
Public Const resCancel = 3  '   ������

' ��������� ����� ������
Public Const ccInteger As Byte = 0
Public Const ccString As Byte = 1

' ����� ������� ������� � ��
    ' ��������� ������ ��� �����
Public Const flPasswordNeed As Byte = 1
    ' ��������� ������ �� ������ ��� ������
Public Const flReadOnlyEnable As Byte = 2
    ' ��������������� ������
Public Const flCoded As Byte = 4

' ��� ��������
Type TDiagElem
    Text As String
    Val As Integer
    Color As Long
End Type

' ����� ������ ������
Public Sub ProtectedMsg()
 Call MsgForm.ErrorMsg("������������ ���� ��� ���������� ��������!")
End Sub

' ���� ������� ������
Public Sub SoundClick()
    Call sndPlaySound("Data\Click.wav", SND_ASYNC + SND_FILENAME + SND_LOOP + SND_APPLICATION)
End Sub

Public Function IsInteger(ByVal str$) As Boolean
 Dim Arr(1 To 4) As String * 1
 Arr(1) = "e": Arr(2) = "E": Arr(3) = ",": Arr(4) = "."
 IsInteger = True
 If IsNumeric(str) Then
  For i% = LBound(Arr) To UBound(Arr)
   If (InStr(1, str, Arr(i)) > 0) Then
    IsInteger = False
    Exit For
   End If
  Next i
 Else
  IsInteger = False
 End If
End Function

Public Sub ButEnabled(Pict As Image, Lbl As Label, enbl As Boolean)
    If enbl Then
        Pict.Picture = MainForm.ButtonImageList.ListImages(1).Picture
        Lbl.MousePointer = 1
    Else
        Pict.Picture = MainForm.ButtonImageList.ListImages(2).Picture
        Lbl.MousePointer = 12
    End If
    Lbl.Tag = CInt(enbl)
End Sub
