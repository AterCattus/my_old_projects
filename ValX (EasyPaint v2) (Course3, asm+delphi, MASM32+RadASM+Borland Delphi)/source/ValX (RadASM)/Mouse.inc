;##################################################################
;    ...::: Valkiria :::...
;
; ����     : Mouse.inc
; �������� : �������� ��������� ������� �� ����
; ������   : 24.03
;
;##################################################################

Ymod MACRO reg, val
; ���������� � ���������� Y ������ ToolBar � ������� ����� ������
	mov		reg,	val
	sub		reg,	TOP_DELTA
	sub		reg,	LeftTop.y
ENDM

Xmod MACRO reg, val
; ���������� � ���������� X ������� ����� �����
	mov		reg,	val
	sub		reg,	LeftTop.x
ENDM 

LineModeCheck MACRO _X, _Y
; �������� �� ��������� �����. ��� ����. �����
	invoke	GetKeyState, 'H'
	test	ax,		80h
	jz		@F
		mov		eax,	StartPos.y
		mov		_Y,		eax
		jmp		@@end
	@@:
	invoke	GetKeyState, 'V'
	test	ax,		80h
	jz		@@end
		mov		eax,	StartPos.x
		mov		_X,		eax
@@end:
ENDM

CoordCheck MACRO reg
; �������� �� ����� ��������� � �������� ������������� �����
; (unsigned short==65... => signed long=-...)
	mov		eax,	reg
	cmp		ax,		0
	jnl		@F
		sub		eax,	0FFFFh
	@@:
	mov		reg,	eax
ENDM

MouseMove proc X:DWORD, Y:DWORD, K:DWORD
	LOCAL	B : BITMAP
	LOCAL	ClX, ClY : DWORD ; ���������� ����������
	
	; �������� ���������
	CoordCheck X
	CoordCheck Y

	; ��������� ���������� ����������
	m2m		ClX,	X
	mov		eax,	Y
	sub		eax,	TOP_DELTA
	mov		ClY,	eax

	; ���������� ����������: ���������� -> �������
	Xmod eax, X
	Ymod ecx, Y
	mov		X,	eax
	mov		Y,	ecx

	; ������ ������ ������ ����?
	mov		eax,	K
	and		eax,	MK_RBUTTON
	cmp		eax,	0
	je		@F
	.IF (OutMode==IDM_GETTER)
		; ������� ����
		invoke	GetPixel, hCompDC, X, Y
		mov		BGColor,	eax
		invoke	InvalidateRect, hStatusBar, NULL, TRUE
	.ENDIF
@@:

	; ������ ����� ������ ����?
	mov		eax,	K
	and		eax,	MK_LBUTTON
	cmp		eax,	0
	jne		@F
		ret
@@:

	cmp		HasBeenResized,	TRUE
	jne		@F
		ret
@@:

	mov		eax,	OutMode
	.IF (eax<=IDM_INSTRUMENTS || eax>=IDM_GETTER)
		ret
	.ENDIF
	
	;##############################################################################
	.IF (eax==IDM_PEN)
		invoke	MoveToEx, hCompDC, OldPos.x, OldPos.y, NULL
		invoke	LineTo, hCompDC, X, Y
		FLIP_AND_FLIP
		m2m		OldPos.x,	X
		m2m		OldPos.y,	Y
		ret
		
	;##############################################################################
	.ELSEIF (eax==IDM_ERASE)
		invoke	PutPixel, X, Y, BGColor
		FLIP_AND_FLIP
		ret
		
	;##############################################################################
	.ELSEIF (eax==IDM_GETTER)
		invoke	GetPixel, hCompDC, X, Y
		mov		FGColor,	eax
		invoke	SetPenParam, eax, CurPenSize
		invoke	InvalidateRect, hStatusBar, NULL, TRUE
		ret
		
	;##############################################################################
	.ELSEIF (eax==IDM_LINE)
		invoke	DrawClientRect
		invoke	MoveToEx, hBackDC, StartPos.x, StartPos.y, NULL
		LineModeCheck ClX, ClY
		invoke	LineTo, hBackDC, ClX, ClY
		
	;##############################################################################
	.ELSEIF (eax==IDM_RECT)
		invoke	DrawClientRect
		invoke	Rectangle, hBackDC, StartPos.x, StartPos.y, ClX, ClY
		
	;##############################################################################
	.ELSEIF (eax==IDM_ELLIPCE)
		invoke	DrawClientRect
		invoke	Ellipse, hBackDC, StartPos.x, StartPos.y, ClX, ClY
		
	.ENDIF
	
	invoke	FlipClientBackBuff2Wnd
	m2m		OldPos.x,	ClX
	m2m		OldPos.y,	ClY
	
	ret
MouseMove endp

MenuChecked proto ID:DWORD

Client2Image MACRO
; ������� ��������� �� ���������� � �������
	Xmod	eax,	StartPos.x
	Ymod	ecx,	StartPos.y
	add		ecx,	TOP_DELTA
ENDM

MouseLUp proc X:DWORD, Y:DWORD

	cmp		HasBeenResized,	TRUE
	jne		@F
		mov	HasBeenResized,	FALSE
		ret
@@:

	; �������� ����������
	CoordCheck X
	CoordCheck Y

	; ���������� ����������: ���������� -> �������
	Xmod eax, X
	Ymod ecx, Y
	mov		X,	eax
	mov		Y,	ecx

	mov		eax,	OutMode
	
	.IF (WHAT_MODE == MODE_DRAW)
	
		.IF (eax==IDM_LINE)
			Client2Image
			invoke	MoveToEx, hCompDC, eax, ecx, NULL
			LineModeCheck X, Y
			invoke	LineTo, hCompDC, X, Y
			DRAW_AND_FLIP
			invoke	THUMBNAIL_MAKE
			invoke	THUMBNAIL_IMAGE
			
		.ELSEIF (eax==IDM_PEN || eax==IDM_ERASE)
			invoke	THUMBNAIL_MAKE
			invoke	THUMBNAIL_IMAGE
			
		.ELSEIF (eax==IDM_RECT)
			Client2Image
			invoke	Rectangle, hCompDC, eax, ecx, X, Y
			DRAW_AND_FLIP
			invoke	THUMBNAIL_MAKE
			invoke	THUMBNAIL_IMAGE
		
		.ELSEIF (eax==IDM_ELLIPCE)
			Client2Image
			invoke	Ellipse, hCompDC, eax, ecx, X, Y
			DRAW_AND_FLIP
			invoke	THUMBNAIL_MAKE
			invoke	THUMBNAIL_IMAGE
			
		.ELSEIF (eax==IDM_GETTER)
			; �������������� ������� ����������
			invoke	MenuChecked,	PrevMode
		.ENDIF
		
	.ENDIF
	
	mov		ExistsPrev, FALSE

	ret
MouseLUp endp

MouseLDown proc X:DWORD, Y:DWORD
	LOCAL	bc : DWORD
	LOCAL	ClX, ClY : DWORD ; ���������� ����������

	mov		HasBeenResized,	FALSE
	
	; ��������� ���������� ����������
	m2m		ClX,	X
	mov		eax,	Y
	sub		eax,	TOP_DELTA
	mov		ClY,	eax
	
	; ���������� ����������: ���������� -> �������
	Xmod eax, X
	Ymod ecx, Y
	mov		X,	eax
	mov		Y,	ecx

	mov		eax,	OutMode

	.IF (eax==IDM_PEN)
		m2m		OldPos.x,	X
		m2m		OldPos.y,	Y
		
		.IF (CurPenSize == 1)
			invoke	SetPixel, hCompDC, X, Y, FGColor
		.ELSE
			invoke	MoveToEx, hCompDC, OldPos.x, OldPos.y, NULL
			invoke	LineTo, hCompDC, X, Y
		.ENDIF
		DRAW_AND_FLIP
		
	.ELSEIF (eax==IDM_ERASE)
		invoke	PutPixel, X, Y, BGColor
		DRAW_AND_FLIP
		
	.ELSEIF (eax==IDM_GETTER)
		invoke	GetPixel, hCompDC, X, Y
		mov		FGColor,	eax
		invoke	InvalidateRect, hStatusBar, NULL, TRUE
		
	.ELSEIF (eax==IDM_FILL)
		m2m		bc,	BGColor
		invoke	SetBrushColor, FGColor
		
		invoke	GetPixel, hCompDC, X, Y
		invoke	ExtFloodFill, hCompDC, X, Y, eax, FLOODFILLSURFACE
		DRAW_AND_FLIP
		
		invoke	SetBrushColor, bc
		
	.ELSE
		m2m		StartPos.x,	ClX
		m2m		StartPos.y,	ClY
	.ENDIF

	ret
MouseLDown endp

MouseRUp proc X:DWORD, Y:DWORD

	cmp		OutMode,	IDM_GETTER
	jne		@F
		; �������������� ������� ����������
		invoke	MenuChecked,	PrevMode
@@:

	ret
MouseRUp endp

MouseRDown proc X:DWORD, Y:DWORD
	LOCAL	bc : DWORD
	
	cmp		HasBeenResized,	TRUE
	jne		@F
		ret
@@:

	Xmod eax, X
	Ymod ecx, Y
	mov		X,	eax
	mov		Y,	ecx

	.IF (OutMode==IDM_GETTER)
		invoke	GetPixel, hCompDC, X, Y
		mov		BGColor,	eax
		invoke	InvalidateRect, hStatusBar, NULL, TRUE
		
	.ELSEIF (OutMode==IDM_FILL)
		m2m		bc,	BGColor
		invoke	SetBrushColor, BGColor
		
		invoke	GetPixel, hCompDC, X, Y
		invoke	ExtFloodFill, hCompDC, X, Y, eax, FLOODFILLSURFACE
		DRAW_AND_FLIP
		invoke	THUMBNAIL_MAKE
		invoke	THUMBNAIL_IMAGE
		
		invoke	SetBrushColor, bc		
	.ENDIF

	ret
MouseRDown endp