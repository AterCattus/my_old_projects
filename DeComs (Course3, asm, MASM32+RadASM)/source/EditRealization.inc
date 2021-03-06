.data

	; ������ ������ � ����
	old_file_size	dd	0
	; ������������� �����
	buff			dd	0
	
.data?
	hEdit	HWND	?
	
.const
	EditStyle equ	WS_CHILD or WS_VISIBLE or WS_HSCROLL or WS_VSCROLL or ES_NOHIDESEL or ES_SAVESEL or \
					ES_LEFT or ES_MULTILINE or ES_AUTOHSCROLL or ES_AUTOVSCROLL or ES_SUNKEN
	
.code

Clear proc
; # ������� ����������� ���������.
	
	invoke	SendMessage, hEdit, EM_SETHANDLE, NULL, 0
	invoke	StatusBarSetText, addr sb_def_text
	
	ret
Clear endp

Edt_Init proc
; # �������� � ������������� ���������.

	invoke	CreateWindowEx, 0, SADD("edit"), NULL, EditStyle, 0, 0, 0, 0, WND, NULL, hInstance, NULL

	mov		hEdit, eax
	
	; �������� ���� � ��������
	invoke	SendMessage, hEdit, EM_GETHANDLE, NULL, 0
	invoke	LocalFree, eax
	invoke	SendMessage, hEdit, EM_SETHANDLE, 0, 0

	invoke	StatusBarSetText, addr sb_def_text
	
	mov		buff,	0
	mov		old_file_size,	0
	
	ret
Edt_Init endp

Edt_Done proc
; # ������������ ������, ���������� ��� ��������.

	invoke	LocalFree, buff
	
	invoke	DestroyWindow, hEdit
	
	ret	
Edt_Done endp

DelRange proc From:DWORD,To:DWORD
; # �������� ����� ������ �� ������, ��������� ����������� [From, To). �������� �����������.

	push	eax
	push	ecx
	push	edx

	; �������� ��������� >>
	mov		eax,	From
	mov		ecx,	To
	mov		edx,	old_file_size
	
	cmp		eax,	edx
	jna		@F
		jmp		@@exit	; From >= Size
@@:

	cmp		ecx,	edx
	jna		@F
		jmp		@@exit	; To >= Size
@@:

	cmp		eax,	ecx
	jb		@F
		jmp		@@exit	; From >= To
@@:
	
	; �������� >>
	
	push	esi
	push	edi

	mov		eax,	buff_ptr
	
	; �������� ����� �����
	mov		esi,	eax
	mov		edi,	eax
	
	add		edi,	From
	add		esi,	To
	
	; �������� ����� ����������� �����
	mov		ecx,	old_file_size
	sub		ecx,	To
	inc		ecx	; #0 ���� �������

	; �������
	push	ecx
	invoke	RtlMoveMemory, edi, esi, ecx
	pop		ecx
	dec		ecx ; ����� ������� �� #0
	
	; ������������ �������
	add		esi,	ecx
	add		edi,	ecx
	
	; �������� ������� ������ >>
	mov		eax,	To
	sub		eax,	From
	sub		old_file_size,	eax
	
	pop		edi
	pop		esi
	
@@exit:
	pop		edx
	pop		ecx
	pop		eax
	
	ret
DelRange endp

.const
	file_error db "������ ��� ������ � ������!",0
	mem_error db "������ ������ � �������",0
.code

LF2CRLF proto
CheckFileType proto

LoadFromFile proc Path:DWORD
; # �������� �����.

	LOCAL	file : DWORD
	LOCAL	file_size : DWORD
	LOCAL	rd : DWORD
	LOCAL	pntr : DWORD

	mov		eax,	buff
	cmp		eax,	0
	je		@F
		invoke	LocalFree, eax
@@:

	; �������� ����
	invoke	CreateFile, Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, WND, addr file_error, addr AppName, MB_OK or MB_ICONERROR
		ret
@@:
	mov		file,	eax

	; ����� ���� � ������
	invoke	GetFileSize, eax, NULL
	mov		file_size,	eax
	inc		eax	; � ����� ���� 0
	
	; ������ ������������, ����� "edit" �� ����� ������ ��������! ... � ������ ;)
	invoke	LocalAlloc, LMEM_MOVEABLE, eax
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, WND, addr mem_error, addr AppName, MB_OK or MB_ICONERROR
		invoke	CloseHandle, file
		ret	
@@:
	mov		buff,	eax
	
	invoke	LocalLock, buff
	mov		pntr,	eax
	
	invoke	ReadFile, file, pntr, file_size, addr rd, NULL
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, WND, addr file_error, addr AppName, MB_OK or MB_ICONERROR
		invoke	CloseHandle, file
		ret	
@@:

	; �������� ����
	invoke	CloseHandle, file
	
	; � ����� ����������� ����� ������ ������ 0
	mov		eax,	pntr
	add		eax,	file_size
	mov		byte ptr [eax],	0

	invoke	LocalUnlock, buff

	m2m		old_file_size, file_size
	
	; �������� ���� �����
	invoke	CheckFileType
	
	invoke	SendMessage, hEdit, EM_SETHANDLE, buff, 0

	invoke	StatusBarSetText, Path
	
	ret
LoadFromFile endp

SaveToFile proc Path:DWORD
; # ���������� �����.

	LOCAL	file : DWORD
	LOCAL	wd : DWORD
	LOCAL	pntr : DWORD
	
	mov		eax,	buff
	cmp		eax,	0
	jne		@F
		ret ; ������ ���������
@@:

	; �������� ����
	invoke	CreateFile, Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, WND, addr file_error, addr AppName, MB_OK or MB_ICONERROR
		ret
@@:
	mov		file,	eax

	; �������� ������� �������� ������ ��������
	invoke	LocalLock, buff
	mov		pntr,	eax
	
	; �������� ����� � ����
	invoke	WriteFile, file, pntr, old_file_size, addr wd, NULL
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, WND, addr file_error, addr AppName, MB_OK or MB_ICONERROR
		invoke	LocalUnlock, buff ; � �����, ���� ������ ������� ?
		invoke	CloseHandle, file
		ret	
@@:

	; �������� ����
	invoke	CloseHandle, file
	
	invoke	LocalUnlock, buff ; � ����� ???

	ret
SaveToFile endp

DelConfirm proc From:DWORD,To:DWORD
; # ������ �� �������� ����������� (������, ��. DelRange ����).

	invoke	SendMessage, hEdit, EM_SETHANDLE, buff, 0
	
	; ������� ��������� �����
	invoke	SendMessage, hEdit, EM_SETSEL, From, To
	; ���� ����� �� ��������� ���� Edit'�, �� ����������� �� ����
	invoke	SendMessage, hEdit, EM_SCROLLCARET, 0, 0
	
	invoke	MessageBox, WND, SADD("������� ������ �����������?"), addr AppName, MB_YESNOCANCEL or MB_ICONEXCLAMATION
	cmp		eax,	IDYES
	jne		@F
		; ����� ���������� �������� ����� �� �����... ��, ��...
		invoke	DelRange, From, To
		mov		eax,	IDYES
@@:
	
	ret
DelConfirm endp

CheckFileType proc
; # ��������� �������������� ����� � ����� "CR/LF" ��� "LF". � ��������� ������ ��������� ��������������.

	invoke	LocalLock, buff

@@loop:
	mov		cl,		byte ptr [eax]
	cmp		cl,		0
	je		@@exit
	
	cmp		cl,		0Dh ; CR
	je		@@exit	; ����� CR/LF - �������������� �� �����
					; ��� � ���, � ��������� �� ������� LF ����� CR. ��� ����� ��� ��� �� ��������, � ��������.
	
	cmp		cl,		0Ah ; LF
	jne		@F
		; ����� LF - ���������� ��������������
		invoke	LocalUnlock, buff
		invoke	LF2CRLF
		ret
@@:

	inc		eax
	jmp		@@loop	
	
@@exit:

	invoke	LocalUnlock, buff
	
	ret
CheckFileType endp

LF2CRLF proc
; # �������������� ����������� ������������������ "LF" � ������������ "CR LF".
	LOCAL	pntr : DWORD
	LOCAL	new_pntr : DWORD
	LOCAL	new_buff : DWORD
	
	invoke	LocalLock, buff
	mov		pntr,	eax
	
	; ������ ���������� ������������ ("LF", 0Ah)
	xor		ecx,	ecx ; �������
	
@@count_loop:
	mov		dl,		byte ptr [eax]
	cmp		dl,		0
	je		@@count_exit
	
	cmp		dl,		0Ah
	jne		@F
		inc		ecx
@@:
	inc		eax
	jmp		@@count_loop

@@count_exit:

	; ������ � ecx ����� ������ LF
	; ����� ����������� ������ ������ ����� ����� Size = Old_Size + ecx {+CR �� ���-�� LF}
	mov		eax,	old_file_size
	add		eax,	ecx
	mov		old_file_size,	eax
	inc		eax ; + �������� #0
	
	; ������ �����
	invoke	LocalAlloc, LMEM_MOVEABLE, eax
	mov		new_buff,	eax
	
	invoke	LocalLock, eax
	mov		new_pntr,	eax
	
	push	esi
	push	edi
	
	mov		esi,	pntr
	mov		edi,	new_pntr
	
	; ������� � �������	
@@replace_loop:
	mov		al,		byte ptr [esi]
	cmp		al,		0
	je		@@replace_exit
	
	.IF (al==0Ah)
		; ����� ����������� - ����� ���� �����
		mov		word ptr [edi],	0A0Dh
		inc		edi ; �.�. ������� �����, � �� ����
	.ELSE
		mov		byte ptr [edi],	al
	.ENDIF
	
	inc		esi
	inc		edi
	
	jmp		@@replace_loop

@@replace_exit:

	mov		byte ptr [edi],	0

	pop		edi
	pop		esi
	
	; ������ ������ ����� � �������� �����
	invoke	LocalUnlock, buff
	invoke	LocalUnlock, new_buff
	
	invoke	LocalFree, buff
	m2m		buff,	new_buff
	
	ret
LF2CRLF endp