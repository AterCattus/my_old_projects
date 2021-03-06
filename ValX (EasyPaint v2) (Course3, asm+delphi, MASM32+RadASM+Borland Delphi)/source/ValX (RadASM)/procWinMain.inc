;##################################################################
;    ...::: Valkiria :::...
;
; ����     : procWinMain.inc
; �������� : �������� ��������� ����
; ������   : 11.03
;
;##################################################################
WinMain proc
	LOCAL	wc		:WNDCLASSEX
	LOCAL	msg		:MSG
	LOCAL	hAccel	:HACCEL
	LOCAL	w, h,
	        x, y	:DWORD
	LOCAL	R		:RECT
	
	invoke	RtlZeroMemory, addr wc, sizeof WNDCLASSEX
	mov		wc.cbSize, sizeof WNDCLASSEX
	mov		wc.style, cClassStyle
	mov		wc.lpfnWndProc, offset WndProc
	push	hInstance
	pop		wc.hInstance
	mov		wc.lpszClassName, offset ClassName
	invoke	LoadIcon, hInstance, 102
	mov		wc.hIcon, eax
	mov		wc.hIconSm, eax
	
	; ����������� ������
	invoke	LoadCursor, hInstance, 101
	cmp		eax,	NULL
	jne		@F
		invoke	MessageBox, 0, SADD("������ � ��������� ���������!"), SADD("������"), MB_OK or MB_ICONERROR or MB_SYSTEMMODAL
		invoke	ExitProcess, 0
@@:
	mov		StdCursor,	eax
	
	; ������ "��� ��������"
	invoke	LoadCursor, hInstance, 103
	cmp		eax,	NULL
	jne		@F
		invoke	MessageBox, 0, SADD("������ � ��������� ���������!"), SADD("������"), MB_OK or MB_ICONERROR or MB_SYSTEMMODAL
		invoke	ExitProcess, 0
@@:
	mov		DragCursor,	eax
	
	; ����������� �����
	invoke	RegisterClassEx, addr wc
	
	invoke	LoadMenu, hInstance, offset MenuName
	mov		hMenu,	eax
	
	; ������ ����
	invoke	CreateWindowEx,\
				NULL,\
				addr ClassName,\
				addr AppName,\
				cWndStyle,\
				CW_USEDEFAULT,\
				CW_USEDEFAULT,\
				CW_USEDEFAULT,\
				CW_USEDEFAULT,\
				NULL,\
				hMenu,\
				hInstance,\
				NULL
	mov		WND,	eax
	
	; ��� ���� ������� �������� ������
	mov		TOP_DELTA,	0
	
	invoke	InitToolBar
	invoke	InitStatusBar
	
include InitApp.inc

	invoke	LoadAccelerators, hInstance, SADD("ACCEL")
	mov		hAccel,	eax
	
	; ��������� ����
	invoke	ShowWindow, WND, SW_SHOWDEFAULT
	invoke	UpdateWindow, WND
	
	; �������� ��������� ���������
	
@@MainLoop:
	invoke	GetMessage, addr msg, NULL, 0, 0
	cmp		eax,	0
	je @@BreakMainLoop
	
	.IF (hDlg)
		invoke	IsDialogMessage, hDlg, addr msg
		.IF (eax)
			jmp short @@MainLoop
		.ENDIF
	.ENDIF

	invoke	TranslateAccelerator, WND, hAccel, addr msg
	cmp		eax,	0
	jne @@MainLoop
	
	invoke	TranslateMessage, addr msg

	invoke	DispatchMessage, addr msg
	
	jmp short @@MainLoop
	
@@BreakMainLoop:

include DoneApp.inc
	
	mov		eax, 0
	ret
WinMain endp