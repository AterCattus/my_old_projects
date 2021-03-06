;##################################################################
;    ...::: Valkiria :::...
;
; ����     : ToolBar.inc
; �������� : ���������� ������ ������������
; ������   : 11.03
;
;##################################################################
.const
	ToolBarID	equ	100
	TOOL_BAR_HEIGHT_DELTA	equ	44

.data
	hToolBar	DWORD	0
	image		HBITMAP	0
	
.data?
	tbb		TBBUTTON	<>

.code

NewButton proc bID:DWORD, cID:DWORD
	m2m tbb.iBitmap,	bID
	m2m tbb.idCommand,	cID
	mov tbb.fsStyle,	TBSTYLE_BUTTON	
	
	invoke SendMessage, hToolBar, TB_ADDBUTTONS, 1, addr tbb
	
	ret
NewButton endp

NewCheck proc bID:DWORD, cID:DWORD
	m2m	tbb.iBitmap,	bID
	m2m tbb.idCommand,	cID
	mov tbb.fsStyle,	TBSTYLE_GROUP or TBSTYLE_CHECKGROUP
	invoke SendMessage, hToolBar, TB_ADDBUTTONS, 1, addr tbb
	
	ret
NewCheck endp

Blank proc
	mov tbb.iBitmap,	-1
	mov tbb.idCommand,	0
	mov tbb.fsStyle,	TBSTYLE_SEP
	invoke SendMessage, hToolBar, TB_ADDBUTTONS, 1, addr tbb
	
	ret
Blank endp

SetBmpBGColor proto

InitToolBar proc
	LOCAL rect	: RECT
	LOCAL tbab	: TBADDBITMAP
	
	invoke	LoadBitmap, hInstance, 100
	cmp		eax,	NULL
	jne		@F
		invoke	MessageBox, 0, SADD("������ � ��������� ���������!"), SADD("������"), MB_OK or MB_ICONERROR or MB_SYSTEMMODAL
		invoke	ExitProcess, 0
@@:
	mov		image,	eax
	
	invoke CreateToolbarEx,	WND, \
							WS_CHILD or TBSTYLE_TOOLTIPS, \
                            ToolBarID, \
                            0, \
                            NULL, \
                            NULL, \
                            NULL, \
                            0, \
                            32, \
                            32, \
                            32, \
                            32, \
                            sizeof TBBUTTON
                            
	mov	hToolBar, eax

	invoke	SetBmpBGColor
	
	mov tbab.hInst,	0
	m2m tbab.nID,	image
	invoke SendMessage, hToolBar, TB_ADDBITMAP, 12, addr tbab

	mov	tbb.fsState,	TBSTATE_ENABLED
	mov tbb.dwData,		0
	mov tbb.iString,	0

	; new open save | pen line rect ellipse erase fill get | help | exit
    invoke	Blank
    invoke	NewButton, 0, IDM_NEW
    invoke	NewButton, 1, IDM_OPEN
    invoke	NewButton, 2, IDM_SAVE
    invoke	Blank
    invoke	NewCheck, 3, IDM_PEN
    invoke	NewCheck, 4, IDM_LINE
    invoke	NewCheck, 5, IDM_RECT
    invoke	NewCheck, 6, IDM_ELLIPCE
    invoke	NewCheck, 7, IDM_ERASE
    invoke	NewCheck, 8, IDM_FILL
    invoke	NewCheck, 9, IDM_GETTER
    invoke	Blank
    invoke	NewButton, 10, IDM_ABOUT
    invoke	Blank
    invoke	NewButton, 11, IDM_EXIT

    
    ; ������� �� ������ ��������
    invoke	SendMessage, hToolBar, TB_CHECKBUTTON, IDM_PEN, 1
    
	invoke	ShowWindow, hToolBar, SW_SHOW
	
	invoke	GetWindowRect, hToolBar, addr rect
	
	add		TOP_DELTA,	TOOL_BAR_HEIGHT_DELTA

	ret
InitToolBar endp

SetBmpBGColor proc
	LOCAL	mDC			: HDC
	LOCAL	hBrush		: HBRUSH
	LOCAL	hOldBmp		: HBITMAP
	LOCAL	hOldBrush	: HBRUSH

	invoke	CreateCompatibleDC, NULL
	mov		mDC, eax

	invoke	SelectObject, mDC, image
	mov		hOldBmp, eax

	invoke	GetSysColor, COLOR_BTNFACE
	invoke	CreateSolidBrush, eax
	mov		hBrush, eax

	invoke	SelectObject, mDC, hBrush
	mov		hOldBrush, eax

	invoke	GetPixel, mDC, 1, 1
	invoke	ExtFloodFill, mDC, 1, 1, eax, FLOODFILLSURFACE

	invoke	SelectObject, mDC, hOldBrush
	invoke	DeleteObject, hBrush

	invoke	SelectObject, mDC, image
	invoke	DeleteDC, mDC

	ret
SetBmpBGColor endp

ShowToolBar proc Show:DWORD
	
	.IF (Show==0)
		; ����� �������� ������
		invoke	ShowWindow, hToolBar, FALSE
		sub		TOP_DELTA,	TOOL_BAR_HEIGHT_DELTA
	.ELSE
		; ����� ���������� ������
		invoke	ShowWindow, hToolBar, TRUE
		add		TOP_DELTA,	TOOL_BAR_HEIGHT_DELTA
	.ENDIF
	
	invoke	Resize
	invoke	ScrollX, 0
	invoke	ScrollY, 0
	invoke	CalcRgns
	invoke	InvalidateRect, WND, NULL, TRUE
	
	invoke	DrawClientRect
	invoke	Flip
	invoke	FlipClientBackBuff2Wnd
	
	invoke	THUMBNAIL_IMAGE
	
	ret
ShowToolBar endp