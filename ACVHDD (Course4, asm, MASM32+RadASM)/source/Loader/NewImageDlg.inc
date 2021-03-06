.const
	IDC_DRIVESIZE	equ 1002
	IDC_IMAGEPATH	equ 1005
	IDC_CHANGEPATH	equ 1006
	IDC_CREATE		equ 1007
	IDC_CANCEL		equ 1008
	
	MAX_IMAGE_SIZE	equ 4096
	
	cNewImageDlgName db "NEWIMAGEDLG",0
	
	SHFMT_ID_DEFAULT		equ 0FFFFh
	SHFMT_OPT_QUICKFORMAT	equ 0
	
.data?
	filebuffer		db MAX_PATH+1 dup (?)
;	SelectedIndex	DWORD ?
	; hProgressBar	DWORD ? <<-- � ������ DriverService.inc
	MayClose		DWORD ?
	
.data
	; ������ ����� ������ �����-������
	Ext_S equ $
	db "acvh",0
	
	Filter_S equ $
	db "ACVH - AterCattus Virtual HDD",0,"*.acvh",0

	All_S equ $
	db "��� ����� (*.*)",0,"*.*",0
	
	; ����� �������
	db 0

.code

_SaveFileDialog_ proc lpTitle:DWORD,lpFilter:DWORD,lpDefExt:DWORD

    LOCAL ofn:OPENFILENAME

    mov eax, OFFSET filebuffer
    mov BYTE PTR [eax], 0

    push edi
    mov ecx, sizeof OPENFILENAME
    mov al, 0
    lea edi, ofn
    rep stosb
    pop edi

    mov ofn.lStructSize,	sizeof OPENFILENAME
    m2m ofn.hWndOwner,		hMainWnd
    m2m ofn.hInstance,		hInstance
    m2m ofn.lpstrFilter,	lpFilter
    m2m ofn.lpstrFile,		offset filebuffer
    mov ofn.nMaxFile,		sizeof filebuffer
    m2m ofn.lpstrTitle,		lpTitle
    m2m ofn.lpstrDefExt,	lpDefExt
    mov ofn.Flags,			OFN_EXPLORER or OFN_LONGNAMES or OFN_NODEREFERENCELINKS or \
							OFN_HIDEREADONLY or OFN_OVERWRITEPROMPT
                       
    invoke GetSaveFileName, ADDR ofn
    
;	m2m		SelectedIndex,	ofn.nFilterIndex
   
    mov eax, OFFSET filebuffer
    ret

_SaveFileDialog_ endp

MySaveFileDlg MACRO hWin,hInstance,lpTitle,lpPattern,lpDefExt
	invoke	_SaveFileDialog_, reparg(lpTitle), reparg(lpPattern), reparg(lpDefExt)
	EXITM	<eax>
ENDM

AC_CreateNewImage proto imgsize:DWORD, imgpath:DWORD 

SwapButtonsEnabling proc hOwnerDlgWnd:DWORD
	push	ebx
	; "�������"
	mov		ebx,	$invoke(GetDlgItem, hOwnerDlgWnd, IDC_CREATE)
	invoke	GetWindowLong, ebx, GWL_STYLE
	xor		eax,	WS_DISABLED
	invoke	SetWindowLong, ebx, GWL_STYLE, eax
	; "������"
	mov		ebx,	$invoke(GetDlgItem, hOwnerDlgWnd, IDC_CANCEL)
	invoke	GetWindowLong, ebx, GWL_STYLE
	xor		eax,	WS_DISABLED
	invoke	SetWindowLong, ebx, GWL_STYLE, eax
	; "..."
	mov		ebx,	$invoke(GetDlgItem, hOwnerDlgWnd, IDC_CHANGEPATH)
	invoke	GetWindowLong, ebx, GWL_STYLE
	xor		eax,	WS_DISABLED
	invoke	SetWindowLong, ebx, GWL_STYLE, eax
	pop		ebx
	
	; ������������� ���� ����� ������� ������
	invoke	InvalidateRect, hOwnerDlgWnd, NULL, TRUE
	invoke	ProcessMessages, hOwnerDlgWnd
	
	ret
SwapButtonsEnabling endp

NewImageDlgWndProc proc hWnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
LOCAL	buff[1001]	: BYTE
LOCAL	imgsize		: DWORD
LOCAL	fname		: DWORD

	.IF (uMsg==WM_INITDIALOG)
		invoke	CreateWindowEx,	NULL, $CTA0('msctls_progress32'), NULL, WS_CHILD or WS_VISIBLE or PBS_SMOOTH,\
								6, 109, 462, 15, hWnd, 1, hInstance, NULL
		mov		hProgressBar,	eax
		; ������ ����� �������
		mov		MayClose,	TRUE

	.ELSEIF (uMsg==WM_SHOWWINDOW)
		invoke	SendDlgItemMessage, hWnd, IDC_DRIVESIZE, EM_EXLIMITTEXT, 0, 4
		; ������
		invoke	ltoa, 100, addr buff
		invoke	SetDlgItemText, hWnd, IDC_DRIVESIZE, addr buff
	
		invoke	GetDlgItem, hWnd, IDC_DRIVESIZE
		invoke	SetFocus, eax
		
	.ELSEIF (uMsg==WM_CLOSE)
		.if (MayClose==TRUE)
			invoke	EndDialog, hWnd, NULL
		.endif
		
	.ELSEIF (uMsg==WM_COMMAND)
		mov	eax, wParam
		mov	edx, eax
		shr	edx, 16
		.IF (dx==BN_CLICKED)
			.IF (eax == IDC_CANCEL)
				invoke	SendMessage, hWnd, WM_CLOSE, NULL, NULL
				
			.ELSEIF (eax == IDC_CHANGEPATH)
				mov		eax,	Ext_S
				mov		ecx,	Filter_S
				mov		fname, MySaveFileDlg(hMainWnd, hInstance, "�������� ������ �����", ecx, eax)
				cmp		BYTE PTR [eax], 0
				je		@F
					invoke	SetDlgItemText, hWnd, IDC_IMAGEPATH, eax
				@@:

			.ELSEIF (eax == IDC_CREATE)
				; ������ �����
				lea		eax,	buff
				push	eax
				invoke	GetDlgItemText, hWnd, IDC_DRIVESIZE, eax, 7
				pop		eax
				invoke	ustr2dw, addr buff
				; >>>> �������� ���������
				cmp		eax,	MAX_IMAGE_SIZE
				jg		@@ERROR1
				test	eax,	eax
				jnz		@F
					@@ERROR1:
					invoke	MessageBox, hWnd, $CTA0('������������ ������ ������ �����'), $CTA0('������'), MB_OK or MB_ICONERROR
					mov		eax, FALSE
					ret
				@@:
				; <<<< �������� ���������
				mov		imgsize,	eax
				
				; ���� � �����
				invoke	GetDlgItemText, hWnd, IDC_IMAGEPATH, addr buff, 1000
				invoke	CreateFile, addr buff, GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0
				.if (eax == INVALID_HANDLE_VALUE)
					invoke	MessageBox, hWnd, $CTA0('������ � ������ ����� �����'), $CTA0('������'), MB_OK or MB_ICONERROR
					mov		eax, FALSE
					ret
				.endif
				invoke	CloseHandle, eax
				invoke	DeleteFile, addr buff
				
				; ����� ������ ������������
				invoke	SwapButtonsEnabling, hWnd
				
				; ����� ������ �� ����
				invoke	SetCursor, $invoke(LoadCursor, NULL, IDC_WAIT)
				
				; ��� ������ ����
				invoke	AC_CreateNewImage, imgsize, addr buff
				push	eax
				invoke	SetCursor, $invoke(LoadCursor, NULL, IDC_ARROW)
				; ����� ������ ����������
				invoke	SwapButtonsEnabling, hWnd
				; �������� ������
				invoke	SendMessage, hWnd, WM_CLOSE, NULL, NULL
				pop		eax
				
				.if (eax == TRUE)
					invoke	MessageBox, hWnd, $CTA0('����� ����� ��� ������. ������ ���������� � ���������������?'), addr APPLICATION_NAME, MB_YESNO or MB_ICONQUESTION
					.IF (eax == IDYES)
						invoke	AC_FindDriverInMem
						.IF (eax == SERVICE_RUNNING)
							invoke	MessageBox, hWnd, $CTA0('� ������ ������ �������� ������ �����. ���������?'), addr APPLICATION_NAME, MB_YESNO or MB_ICONQUESTION
							.IF (eax == IDYES)
								invoke	AC_StopService
								.IF (eax == TRUE)
									jmp	@@NIDWP_LoadAndFormat
								.ENDIF
							.ELSE
								jmp	@@NIDWP_NeedFormat
							.ENDIF
						.ELSE
@@NIDWP_LoadAndFormat:
							invoke	AC_Cmd_LoadDriver, addr buff
							.IF (eax == -1)
								; nothing do
							.ELSEIF (eax != FALSE)
								; ���� �������� - ����� �� ����� ������ � ����
								invoke	AC_TrayIconMenuItemsCheckState
								xor		eax,	eax
								mov		al,	DiskLetter
								sub		al,	'A'
								invoke	SHFormatDrive, 0, eax, SHFMT_ID_DEFAULT, SHFMT_OPT_QUICKFORMAT
								.IF (eax == 0) ; ??? � �� ������, ��� ������ ����, �� ����� ���������
;									invoke	ShellExecute, 0, $CTA0('open'), $CTA0('explorer.exe'), addr DiskLetter, addr DiskLetter, SW_SHOWNORMAL
								.ELSE
									; ������
									jmp	@@NIDWP_NeedFormat
;									invoke	MessageBox,	0, $CTA0('����� ������� ������������� ����� ��� ���������� ��� ���������������'),
;											addr APPLICATION_NAME, MB_OK or MB_ICONINFORMATION
								.ENDIF
							.ENDIF
						.ENDIF
					.ELSE
@@NIDWP_NeedFormat:
						invoke	MessageBox,	hWnd, $CTA0('����� ������� ������������� ����� ��� ���������� ��� ���������������'),
								addr APPLICATION_NAME, MB_OK or MB_ICONINFORMATION
					.ENDIF
				.else
					invoke	MessageBox, hWnd, $CTA0('������ ��� �������� ����� ������'), addr APPLICATION_NAME, MB_OK or MB_ICONERROR
				.endif	; .if (eax == TRUE)
			.ENDIF
		.ENDIF
	.ELSE
		mov	eax, FALSE
		ret
	.ENDIF
	
	mov	eax, TRUE
	ret
NewImageDlgWndProc endp

NewImageDlgShow proc
	
	invoke	DialogBoxParam, hInstance, addr cNewImageDlgName, hMainWnd, addr NewImageDlgWndProc, NULL
	
	ret
NewImageDlgShow endp