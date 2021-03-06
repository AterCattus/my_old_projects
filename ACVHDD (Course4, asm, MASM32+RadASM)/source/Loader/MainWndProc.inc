.const
	APP_CLASS_NAME	db "AC.ACVHDD_WinClass",0
.code

WndProc proto :HWND, :UINT, :WPARAM, :LPARAM

WinMain proc hInst:HINSTANCE, hPrevInst:HINSTANCE, CmdLine:LPSTR, CmdShow:DWORD
LOCAL	wc		: WNDCLASSEX
LOCAL	msg		: MSG
LOCAL	hWnd	: DWORD

	invoke	RtlZeroMemory, addr wc, sizeof(WNDCLASSEX)
	mov		wc.cbSize,			sizeof(WNDCLASSEX)
	mov		wc.lpfnWndProc,		offset WndProc
	m2m		wc.hInstance,		hInst
	mov		wc.lpszClassName,	offset APP_CLASS_NAME
	
	invoke	RegisterClassEx, addr wc
	
	invoke	CreateWindowEx,	WS_EX_TOPMOST, addr APP_CLASS_NAME, NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT,
							CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL

	.while TRUE
		invoke	GetMessage, addr msg, NULL, 0, 0
		.BREAK .IF (!eax)
		invoke	DispatchMessage, addr msg
	.endw
	
	mov	eax, msg.wParam
	ret
WinMain endp

WndProc proc hWnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM

	.if uMsg==WM_CREATE
		; ������ ������ � ����
		m2m		hMainWnd, hWnd
		invoke	AC_TrayIconCreate
		
		; ��� ����������� �������
		invoke	AC_FindDriverInMem
		.if (eax == FIND_DRIVER_RUNNING)
			; ������� ��������. ����� �������� ����� �����, �� ������� �� "�����"
			invoke	AC_GetUsedDriveLetter
			.IF (eax != 0)
				; ������ ����, �� ������� "�����" �������
				; �������� ����� �����
				mov		byte ptr [DiskLetter],	al
			.ELSE
				; ������ �� ������. �����-�� ���� � �������: ������� ��������, � ����� ���!
				invoke	MessageBox, hMainWnd, $CTA0('���� � �������'), addr APPLICATION_NAME, MB_OK or MB_ICONERROR
				; ����� ���� ����� ���������� ��������� ������� ������� � ���������� �� ������?
				; ����� ��� ����� ��������� ���, ������ ������� ������ �� �� ��� ��������.
				; �� ��� � �������, �.�. ����� �� ���. � ������������ ����� �� �����, ��� ������� ��������
			.ENDIF 
		.else
			; ��������� ��������� ��� �� ����� - ��� ����� ���� �� ��������
		.endif
		
		; ����� ������ � ������� ��������� ��������
		invoke	AC_TrayIconMenuItemsCheckState
		
	.elseif uMsg==WM_DESTROY
		invoke	AC_TrayIconDelete
		invoke	PostQuitMessage, NULL
		
	.elseif uMsg==WM_COMMAND
		invoke	AC_TrayIconWndProc_WM_COMMAND, wParam, lParam
		
	.elseif uMsg==WM_TRAY
		invoke	AC_TrayIconWndProc_WM_TRAY, wParam, lParam
		
	.else
		invoke	DefWindowProc, hWnd, uMsg, wParam, lParam
		ret
	.endif
	
	xor	eax,eax
	ret
WndProc endp