.data
	hCompDC		HDC		0
	
.data?
	hDC			HDC		?
	Rect		RECT	<>
	hCompBtmp	HBITMAP	?
	hOldBtmp	HBITMAP	?
	
	AreaHeight	DWORD	?

	; ������� ���������
	CurPenSize	DWORD	?
	
.code

;---------------------------------------------------------------------------------
;                   DeleteBackBuffer
;---------------------------------------------------------------------------------
DeleteBackBuffer proc
	mov		eax,	hCompDC
	cmp		eax,	0
	je		@F
		invoke	SelectObject, hCompDC, hOldBtmp
		invoke	DeleteObject, hCompBtmp
		invoke	DeleteDC, hCompDC
		mov		hCompDC,	0		
@@:
  
    ret
    
DeleteBackBuffer endp

Clear proto

;---------------------------------------------------------------------------------
;                   CreateBackBuffer
;---------------------------------------------------------------------------------
CreateBackBuffer proc hWnd:HWND, W:DWORD, H:DWORD, OldSave:DWORD
LOCAL	_hDC	: HDC
LOCAL	_hCompDC : HDC
LOCAL	_Rect : RECT
LOCAL	_hCompBtmp : HBITMAP
LOCAL	_hOldBtmp : HBITMAP

	mov		_Rect.left, 0
	mov		eax, TBHeight
	mov		_Rect.top, eax
	m2m		_Rect.right, W
	m2m		_Rect.bottom, H
	
	invoke	GetDC, hWnd
	mov		_hDC, eax
	
	invoke	CreateCompatibleDC, _hDC
	mov		_hCompDC, eax
	
	invoke	CreateCompatibleBitmap, _hDC, _Rect.right, _Rect.bottom
	mov		_hCompBtmp,	eax
	
	invoke	SelectObject, _hCompDC, eax
	mov		_hOldBtmp, eax
	
	invoke	ReleaseDC, hWnd, _hDC
	
	cmp		hCompDC,	0
	je		@@neednt_copy
		; ������ ��� 
		invoke	BitBlt, _hCompDC, 0, 0, _Rect.right, _Rect.bottom, NULL, 0, 0, WHITENESS
		
		; ���� �����, ������� ������ ���������� 
		.IF (OldSave)
			invoke	BitBlt, _hCompDC, 0, 0, Rect.right, Rect.bottom, hCompDC, 0, 0, SRCCOPY
		.ENDIF
		
		; ������ ������ �����
		invoke	SelectObject, hCompDC, hOldBtmp
		invoke	DeleteObject, hCompBtmp
		invoke	DeleteDC, hCompDC
		
		; � �������� ����� ��������
		m2m		hCompDC,	_hCompDC
		m2m		hCompBtmp,	_hCompBtmp
		m2m		hOldBtmp,	_hOldBtmp
		invoke	RtlMoveMemory, addr Rect, addr _Rect, sizeof RECT
		
		jmp		@F
		
@@neednt_copy:
	; �������� ����� ��������
	m2m		hCompDC,	_hCompDC
	m2m		hCompBtmp,	_hCompBtmp
	m2m		hOldBtmp,	_hOldBtmp
	invoke	RtlMoveMemory, addr Rect, addr _Rect, sizeof RECT
	; ������ ���
	invoke	Clear

@@:		

	mov		eax,	1
	ret

CreateBackBuffer endp

;---------------------------------------------------------------------------------
;                  Clear
;---------------------------------------------------------------------------------
Clear proc

	mov		eax,	hCompDC
	cmp		eax,	0
	je		@F
		invoke	BitBlt, eax, 0, 0, Rect.right, Rect.bottom, NULL, 0, 0, WHITENESS
@@:
	  
    ret
Clear endp

;---------------------------------------------------------------------------------
;                  Flip
;---------------------------------------------------------------------------------
Flip proc

	mov		eax,	hCompDC
	cmp		eax,	0
	je		@@end
		invoke	BitBlt, hDC, 0, TBHeight, Rect.right, AreaHeight, hCompDC, 0, 0, SRCCOPY
@@end:

	ret
Flip endp

;---------------------------------------------------------------------------------
;                  Resize
;---------------------------------------------------------------------------------
Resize proc
	LOCAL R : RECT
	
	invoke	GetClientRect, WND, addr R
	mov		ecx,	R.bottom
	sub		ecx,	TBHeight
	sub		ecx,	SBHeight
	; ���� ������ ������ 0, �� ����� ��������� ��� (������!)
	cmp		ecx,	0
	jnl		@F
	ret
@@:
	mov		edx,	Rect.bottom
	; ���� ������ ��������� ������� ������ ������ �����������, �� ����� ������� �����������
	cmp		ecx,	edx
	jna		@F
	xchg	ecx,	edx
@@:

	mov	AreaHeight,	ecx
	
	ret
Resize endp

.data ; ��������� �������
	sERROR db "������!",0
	sINFO db "���������",0
	sGetParamError db "������ ��������� ���������� �����������!",0
	sWriteFileError db "������ ������ � ����!",0
	
.code

SetPenParam proto Color:DWORD, W:DWORD
SetBrushColor proto Color:DWORD

LoadFromFile proc path:DWORD
	LOCAL bmp : HBITMAP
	LOCAL DC : HDC
	LOCAL B : BITMAP

	cmp		hCompDC,	0
	jne		@F
		ret
@@:

	; �������� ������
	invoke	LoadImage, NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, 0, SADD("������ ��� �������� �����������!"), addr sERROR, MB_OK or MB_ICONERROR
		ret
@@:
	mov		bmp,	eax
	; ������� HBITMAP
	
	; ������ ��������� ��������
	invoke	CreateCompatibleDC, hCompDC
	mov		DC, eax
	
	; ��������� �������� � ��������
	invoke	SelectObject, DC, bmp

	; ������� ������� �������
	invoke	GetObject, bmp, sizeof BITMAP, addr B
	
	.IF (!eax)
		invoke	MessageBox, 0, addr sGetParamError, addr sERROR, MB_OK or MB_ICONERROR
		
		invoke	SelectObject, DC, NULL
		invoke	DeleteObject, bmp
		invoke	DeleteDC, DC
		
		ret
	.ENDIF
	
	; ������ ����� �����
	invoke	CreateBackBuffer, WND, B.bmWidth, B.bmHeight, FALSE
	
	; ������� � ���� ������
	invoke	BitBlt, hCompDC, 0, 0, B.bmWidth, B.bmHeight, DC, 0, 0, SRCCOPY
	
	
	; �������������� ��������� ������������
	invoke	SetPenParam, FGColor, CurPenSize
	invoke	SetBrushColor, BGColor
	
	; ������������� ����
	invoke	InvalidateRect, WND, NULL, TRUE
	
	; � ������ �����
	invoke	SelectObject, DC, NULL
	
	invoke	DeleteObject, bmp
	
	invoke	DeleteDC, DC
	
	; �����
	invoke	MessageBox, 0, SADD("����������� ���������!"), addr sINFO, MB_OK or MB_ICONINFORMATION	
	
	ret
LoadFromFile endp

SaveToFile proc path:DWORD
	LOCAL	bfh : BITMAPFILEHEADER
	LOCAL	BI : BITMAPINFO
	LOCAL	f : DWORD
	LOCAL	wb : DWORD
	LOCAL	buff : DWORD
	LOCAL	BMPParam : BITMAP
	LOCAL	BMPSize : DWORD
	

; �������� �� ������� ����������� ###############################################################################
	cmp		hCompDC,	0
	jne		@F
		invoke	MessageBox, 0, SADD("� ��� ��� ��������� ��?"), addr sERROR, MB_OK or MB_ICONERROR
		ret
@@:

; ������� ������� ������� #######################################################################################
	invoke	GetObject, hCompBtmp, sizeof BITMAP, addr BMPParam
	.IF (!eax)
		invoke	MessageBox, 0, addr sGetParamError, addr sERROR, MB_OK or MB_ICONERROR
		ret
	.ENDIF
	
; �������� ������ ����������� � ������ � ������� ������ #########################################################
	mov		eax,	BMPParam.bmWidthBytes
	mov		ecx,	BMPParam.bmHeight
	xor		edx,	edx
	mul		ecx
	mov		BMPSize,	eax
	
	invoke	GlobalAlloc, GMEM_FIXED, BMPSize
	.IF (!eax)
		invoke	MessageBox, 0, SADD("������ ��������� ������!"), addr sERROR, MB_OK or MB_ICONERROR
		ret
	.ENDIF
		
	mov		buff,	eax
	
; ������� ������ ������� ########################################################################################
	invoke	RtlZeroMemory, addr BI, sizeof BITMAPINFO
	mov		BI.bmiHeader.biSize,	sizeof BITMAPINFOHEADER
	m2m		BI.bmiHeader.biWidth,	BMPParam.bmWidth
	m2m		BI.bmiHeader.biHeight,	BMPParam.bmHeight
	m2m		BI.bmiHeader.biPlanes,	BMPParam.bmPlanes
	m2m		BI.bmiHeader.biBitCount,	BMPParam.bmBitsPixel
	mov		BI.bmiHeader.biCompression,	BI_RGB

	invoke	GetDIBits, hCompDC, hCompBtmp, 0, BMPParam.bmHeight, buff, addr BI, DIB_RGB_COLORS
	.IF (!eax)
		invoke	MessageBox, 0, addr sGetParamError, addr sERROR, MB_OK or MB_ICONERROR
		invoke	GlobalFree, buff
		ret
	.ENDIF
	
; ������ ���� ###################################################################################################
	invoke	CreateFile, path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
	cmp		eax,	INVALID_HANDLE_VALUE
	jne		@F
		invoke	MessageBox, 0, SADD("������ ��� �������� �����!"), addr sERROR, MB_OK or MB_ICONERROR
		invoke	GlobalFree, buff
		ret
@@:
	mov		f,	eax
	

; �������� BITMAPFILEHEADER #####################################################################################
	invoke	RtlZeroMemory, addr bfh, sizeof BITMAPFILEHEADER
	
	mov		bfh.bfType,	4D42h

	mov		eax,	BMPSize
	add		eax,	(sizeof BITMAPFILEHEADER)+(sizeof BITMAPINFOHEADER)
	mov		bfh.bfSize,	eax
	
	mov		bfh.bfOffBits,	54

; ��������� BITMAPFILEHEADER � ���� #############################################################################
	invoke	WriteFile, f, addr bfh, sizeof BITMAPFILEHEADER, addr wb, NULL
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, 0, addr sWriteFileError, addr sERROR, MB_OK or MB_ICONERROR
		invoke	GlobalFree, buff
		invoke	CloseHandle, f
		ret
@@:

; ��������� BITMAPINFOHEADER � ���� #############################################################################
	invoke	WriteFile, f, addr BI.bmiHeader, sizeof BITMAPINFOHEADER, addr wb, NULL
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, 0, addr sWriteFileError, addr sERROR, MB_OK or MB_ICONERROR
		invoke	GlobalFree, buff
		invoke	CloseHandle, f
		ret
@@:

; ��������� ��� ������� ������ ##################################################################################
	invoke	WriteFile, f, buff, BMPSize, addr wb, NULL
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, 0, addr sWriteFileError, addr sERROR, MB_OK or MB_ICONERROR
		invoke	GlobalFree, buff
		invoke	CloseHandle, f
		ret
@@:

; ���������� ������� ############################################################################################

	invoke	GlobalFree, buff
	invoke	CloseHandle, f
	invoke	MessageBox, 0, SADD("����������� ���������!"), addr sINFO, MB_OK or MB_ICONINFORMATION
		
	ret
SaveToFile endp