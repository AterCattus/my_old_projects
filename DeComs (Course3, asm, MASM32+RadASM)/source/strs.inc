strCat proc source:DWORD, dest:DWORD
; # ������������ �����.
; source += dest;

	invoke	szLen,	source
	add		eax,	source
	mov		ecx,	eax
	invoke	szLen,	dest
	inc		eax
	invoke	RtlMoveMemory, ecx, dest, eax

	ret
strCat endp

strCopy proc source:DWORD, dest:DWORD
; # ����������� �����.
; source = dest;

	invoke	szLen,	dest
	inc		eax
	invoke	RtlMoveMemory, source, dest, eax

	ret
strCopy endp

strExt proc text:DWORD
; # ���������� � ����� ������ ������� #0.
	
	invoke	szLen,	text
	add		eax,	text
	inc		eax
	mov		byte ptr [eax],	0
	
	ret
strExt endp

Delim2Zero proc text:DWORD
; # ������ ���� (,) � ������ �� #0.

	mov		eax,	text
	
@@loop:
	mov		cl,		byte ptr [eax]
	cmp		cl,		0
	jne		@F
		ret
@@:

	cmp		cl,		','
	jne		@F
		mov		byte ptr [eax],	0
@@:
	inc		eax
	jmp		@@loop

	ret
Delim2Zero endp

CopyZero2Delim proc dest:DWORD, text:DWORD
; # ������ ���� ����������� #0 � ������ text �� (,). ��������� ������������ � dest.

	mov		eax,	text
	mov		edx,	dest
	
@@loop:
	mov		cl,		byte ptr [eax]
	cmp		cl,		0
	jne		@F
		inc		eax
		cmp		byte ptr [eax],	0
		je		@@exit ; ������ ������������ ����
		
		; ��������� ���� ������� �� (,)
		mov		cl,	','
		dec		eax
	
@@:
	mov		byte ptr [edx], cl

	inc		eax
	inc		edx
	jmp		@@loop

@@exit:

	mov		byte ptr [edx], 0

	ret
CopyZero2Delim endp

check_on_slash proc text:DWORD
; # �������� �� ������� ������������ ����� � ������. ���� ��� - ��������.

	slash_zero  equ	5Ch ; '\',0

	invoke	szLen,	text
	add		eax,	text
	dec		eax
	cmp		byte ptr [eax],	'\'
	je		@F
		mov		word ptr [eax+1],	slash_zero
@@:	
	
	ret
check_on_slash endp

Path_Append proc path:DWORD, eop:DWORD
; # ����������� ���� � �����.
	
	invoke	check_on_slash, path
	invoke	strCat, path, eop
	invoke	check_on_slash, path
	
	ret
Path_Append endp

PathDisAppend proc path:DWORD
; # �������� ��������� ����� �� ����.

	invoke	szLen, path
	add		eax,	path
	dec		eax ; #0
	dec		eax ; '\'
	
@@loop:
	cmp		byte ptr [eax],	'\'
	jne		@F
		mov	byte ptr [eax],	0
		ret 
@@:

	dec		eax
	jmp		@@loop
	
	ret
PathDisAppend endp

; #########################################################################################
;                   ���������� � ������ ������ �� ������������
; #########################################################################################

;ShowDWORD macro dw_val
;; # ����� �������� dw_val.
;
;	invoke	dw2a, dw_val, tmp_str
;	invoke	MessageBox, 0, tmp_str, NULL, MB_OK or MB_ICONINFORMATION
;	
;endm
;
;ShowText macro txt
;; # ����� ������.
;
;	invoke	MessageBox, 0, txt, NULL, MB_OK or MB_ICONINFORMATION
;	
;endm
;
;NewS macro text:VARARG
;; # ������������ ��������� ������ ��� SZ.
;
;	LOCAL txt_name
;	.data
;		txt_name db text,0
;	.code
;	lea		eax,	txt_name
;	exitm <eax>
;endm

;char2bin proc
;
;	cmp		al,		'9'
;	ja		@F
;		sub		al,		'0'
;		ret
;@@:
;	; ��� �����. ����� ���� � ���� 3 ����...
;	cmp		al,		'F'
;	ja		@F
;	cmp		al,		'A'
;	jb		@F
;		sub		al,		'A'-10
;		ret
;@@:
;
;	cmp		al,		'f'
;	ja		@F
;	cmp		al,		'a'
;	jb		@F
;		sub		al,		'a'-10
;		ret
;@@:
;	
;	ret
;char2bin endp
;
;ex_str2str proc text:DWORD
;	LOCAL buff : DWORD
;	
;	invoke	szLen,	text
;	inc		eax
;	inc		eax
;	
;	push	ebx
;	
;	invoke	LocalAlloc, LMEM_FIXED, eax
;	mov		buff,	eax
;	mov		ebx,	eax
;	
;	mov		ecx,	text
;	
;@@loop:
;	mov		al,		byte ptr [ecx]
;	cmp		al,		0
;	je		@@exit
;	cmp		al,		'$'
;	jne		@F
;		; ������ ��� �������
;		inc		ecx
;		mov		al,		byte ptr [ecx]
;		invoke	char2bin
;		mov		ah,		al
;		shl		ah,		4
;		inc		ecx
;		mov		al,		byte ptr [ecx]
;		invoke	char2bin
;		xadd	al,		ah ; ��� ���� ���������� ����� 486
;@@:
;
;	mov		byte ptr [ebx],	al
;	
;	inc		ecx
;	inc		ebx
;	jmp		@@loop	
;	
;@@exit:
;
;	mov		byte ptr [ebx], 0
;	inc		ebx
;	mov		byte ptr [ebx], 0
;	
;	invoke	strCopy, text, buff
;
;	invoke	LocalFree, buff
;	
;	pop		ebx
;	
;	ret
;ex_str2str endp