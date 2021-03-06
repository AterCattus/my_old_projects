WinMain proc
; # �������� ����, ���������� � �������� ����.

	LOCAL	wc		:	WNDCLASSEX
	LOCAL	msg		:	MSG
	LOCAL	hAccel	:	HACCEL
	LOCAL	w, h,
	        x, y	:	DWORD
	LOCAL	R		:	RECT
	
	invoke	RtlZeroMemory, addr wc, sizeof WNDCLASSEX
	mov		wc.cbSize, sizeof WNDCLASSEX
	mov		wc.style, cClassStyle
	mov		wc.lpfnWndProc, offset WndProc
	push	hInstance
	pop		wc.hInstance
	mov		wc.lpszClassName, offset ClassName
;	invoke	LoadIcon, NULL, IDI_WINLOGO
	mov		wc.hIcon, eax
	mov		wc.hIconSm, eax

	invoke	LoadCursor, NULL, IDC_ARROW
	mov		wc.hCursor, eax	
	
	; ����������� �����
	invoke	RegisterClassEx, addr wc
	
	; �������� ������ ������ � ��������� �� ������
	; [���-�� ��� ����� �����... �����-�� APIfunc ��������]
	; w = �Width + 2*GetSystemMetrics(SM_CXFIXEDFRAME)
	invoke	GetSystemMetrics, SM_CXFIXEDFRAME
	shl		eax,	1
	add		eax,	cWidth
	mov		w,		eax
	; h = �Height + 2*GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENUSIZE)
	invoke	GetSystemMetrics, SM_CYFIXEDFRAME
	shl		eax,	1
	add		eax,	cHeight
	mov		h,		eax
	invoke	GetSystemMetrics, SM_CYCAPTION
	add		h,		eax
	invoke	GetSystemMetrics, SM_CYMENUSIZE
	add		h,		eax
	; x = (GetSystemMetrics(SM_CXSCREEN)-w) >> 1
	invoke	GetSystemMetrics, SM_CXSCREEN
	sub		eax,	w
	shr		eax,	1
	mov		x,		eax
	; y = (GetSystemMetrics(SM_CYSCREEN)-h) >> 1
	invoke	GetSystemMetrics, SM_CYSCREEN
	sub		eax,	h
	shr		eax,	1
	mov		y,		eax
	
	invoke	LoadMenu, hInstance, offset MenuName
	mov		hMenu,	eax
	
	; ������ ����
	invoke	CreateWindowEx,\
				NULL,\
				addr ClassName,\
				addr AppName,\
				cWndStyle,\
				x,\
				y,\
				w,\
				h,\
				NULL,\
				hMenu,\
				hInstance,\
				NULL
	mov		WND,	eax

	invoke	InitStatusBar
	
	invoke	Edt_Init

	invoke	LoadAccelerators, hInstance, SADD("ACCEL")
	mov		hAccel,	eax
	
	; ��������� ����
	invoke	ShowWindow, WND, SW_SHOW
	invoke	UpdateWindow, WND
	
	; �������� ��������� ����������
	invoke	MenuProcess, IDM_PARAMS
	
	; �������� ��������� ���������
	
@@MainLoop:
	invoke	GetMessage, addr msg, NULL, 0, 0
	cmp		eax,	0
	je @@BreakMainLoop

	invoke	TranslateAccelerator, WND, hAccel, addr msg
	cmp		eax,	0
	jne @@MainLoop
	
	invoke	TranslateMessage, addr msg ; ��� � �� ����� ����

	invoke	DispatchMessage, addr msg
	
	jmp short @@MainLoop
	
@@BreakMainLoop:

	invoke	Edt_Done

	xor		eax,	eax
	ret
WinMain endp