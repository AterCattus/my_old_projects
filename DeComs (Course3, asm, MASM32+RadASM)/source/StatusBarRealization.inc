.const
	StatusBarID	equ	101
	sb_def_text db "���� �� ������",0
	
.data?
	hStatusBar	DWORD	?
	SBHeight	DWORD	?
	
.code

StatusBarSetText proto text:DWORD

InitStatusBar proc
; # �������� ��������� ������.

	LOCAL R : RECT
	
	invoke	CreateStatusWindow, WS_CHILD or WS_VISIBLE or SBS_SIZEGRIP, \
								NULL, \
								WND, StatusBarID
	
	mov	hStatusBar, eax
	
	invoke	GetWindowRect, hStatusBar, addr R
	mov		eax,	R.bottom
	sub		eax,	R.top
	inc		eax
	mov		SBHeight,	eax

	invoke	StatusBarSetText, addr sb_def_text
	
	invoke	ShowWindow, hStatusBar, SW_SHOW
	
	ret
InitStatusBar endp

StatusBarSetText proc text:DWORD
; # ��������� ������ ������ �������.

	invoke	SendMessage, hStatusBar, SB_SETTEXT, 0, text
	
	ret
StatusBarSetText endp