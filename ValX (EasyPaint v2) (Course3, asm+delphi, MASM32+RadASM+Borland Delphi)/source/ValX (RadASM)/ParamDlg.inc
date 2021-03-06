;##################################################################
;    ...::: Valkiria :::...
;
; ����     : ParamDlg.inc
; �������� : ���������� ���� ���������� ������ �����������
; ������   : 23.03
;
;##################################################################
.const
	IDC_STC1 equ 2001
	IDC_STC2 equ 2002
	IDC_EDT1 equ 2003
	IDC_EDT2 equ 2004
	IDC_CHK1 equ 2005
	IDC_BTN1 equ 2006
	IDC_BTN2 equ 2007

	cDlgName db "ParamsDlg",0
	cDlgClass db "PARAMSDLGCLASS",0
	
	; ������������ ������� �����������
	MAX_IMAGE_SIZE_X	equ		2000
	MAX_IMAGE_SIZE_Y	equ		2000
	
.data
	hDlg 		DWORD 	0
	CheckBoxVal	DWORD	0
	 
.code

ShowSizeError proc
	.data
		bad_size db "������������ ������� ������ �����������!", 13,
		            "��������� ������� ������ � ��������� 1..2000.", 0
	.code
	invoke	MessageBox, hDlg, addr bad_size, SADD("������"), MB_OK or MB_ICONERROR
	ret
ShowSizeError endp

check_buff MACRO
; �������� �� ������������ ������ � [eax] �� ����������� ����������� ����� �����
	LOCAL	@@loop, @@end
	push	eax
	mov		dh,	0 ; ���-�� ����
@@loop:
	mov		dl,	byte ptr [eax] ; ��������� �����
	cmp		dl, 0 ; ����� ������?
	je		@@end
		
		; �������� ��������
		cmp		dl,		'0'
		jnb		@F
			invoke	ShowSizeError
			ret
		@@:
		
		cmp		dl,		'9'
		jna		@F
			invoke	ShowSizeError
			ret
		@@:
		
		inc		dh
		cmp		dh,		4
		jna		@F
			invoke	ShowSizeError
			ret
		@@:
		
		inc		eax
		
		jmp		@@loop
	
@@end:
	pop		eax	
ENDM

DlgWndProc proc hWnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
	LOCAL buff[8] : BYTE

	.IF (uMsg==WM_SHOWWINDOW)
		; ������
		invoke	ltoa, Rect.right, addr buff
		invoke	SetDlgItemText, hWnd, IDC_EDT1, addr buff
		; ������
		invoke	ltoa, Rect.bottom, addr buff
		invoke	SetDlgItemText, hWnd, IDC_EDT2, addr buff
	
		invoke	GetDlgItem, hWnd, IDC_EDT1
		invoke	SetFocus, eax
		
	.ELSEIF (uMsg==WM_CLOSE)
		invoke	EndDialog, hWnd, NULL
		mov		hDlg, NULL
		
	.ELSEIF (uMsg==WM_COMMAND)
		mov	eax, wParam
		mov	edx, eax
		shr	edx, 16
		.IF (dx==BN_CLICKED)
			.IF (eax==IDC_BTN2)
				invoke	SendMessage, hWnd, WM_CLOSE, NULL, NULL
				
			.ELSEIF (eax==IDC_CHK1)
				not		CheckBoxVal
				
			.ELSEIF (eax==IDC_BTN1)
				
				push	CheckBoxVal
				
			; HEIGHT
				lea		eax,	buff
				push	eax
				invoke	GetDlgItemText, hWnd, IDC_EDT2, eax, 6
				pop		eax
				check_buff
				invoke	ustr2dw, addr buff
				; >>>> �������� ���������
				cmp		eax,	MAX_IMAGE_SIZE_Y
				jg		@@ERROR1
				test	eax,	eax
				jnz		@F
					@@ERROR1:
					; ������� ������� ������ �����������
					invoke	ShowSizeError
					pop		eax ; ��������� � ���� CheckBoxVal
					mov		eax, FALSE
					ret
				@@:
				; <<<< �������� ���������
				push	eax
				
			; WIDTH
				lea		eax,	buff
				push	eax
				invoke	GetDlgItemText, hWnd, IDC_EDT1, addr buff, 6
				pop		eax
				check_buff
				invoke	ustr2dw, addr buff
				; >>>> �������� ���������
				cmp		eax,	MAX_IMAGE_SIZE_X
				jg		@@ERROR2
				test	eax,	eax
				jnz		@F
					@@ERROR2:
					; ������� ������� ������ �����������
					invoke	ShowSizeError
					pop		eax ; ��������� � ���� CheckBoxVal
					pop		ecx ; ��������� � ���� Height
					mov		eax, FALSE
					ret
				@@:
				; <<<< �������� ���������
				push	eax
				
				push	WND
				
				call	CreateBackBuffer
				
				invoke	SetPenParam, FGColor, CurPenSize
				invoke	SetBrushColor, BGColor				
				
				invoke	SendMessage, hWnd, WM_CLOSE, NULL, NULL
				
				invoke	Resize
				
				invoke	InvalidateRect, WND, NULL, TRUE
				
			.ENDIF
		.ENDIF
	.ELSE
		mov	eax, FALSE
		ret
	.ENDIF
	
	mov	eax, TRUE
	ret
DlgWndProc endp

DlgShow proc

	mov		CheckBoxVal,	0
	
	invoke	DialogBoxParam, hInstance, addr cDlgName, WND, addr DlgWndProc, NULL
	mov		hDlg,	eax
	
	ret
DlgShow endp