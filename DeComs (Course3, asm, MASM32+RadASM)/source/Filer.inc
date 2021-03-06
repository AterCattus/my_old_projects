.data?
	; ������ ����� �����
	FileFormat	dd	?
	; ������ ����� ����� ��� ������
	format		db 20 dup (?)
	; ������������� ����� �����
	buff_ptr	dd	?
	; ����� ���������� ������� � ������. ���� �� ������, �� �� ��������.
	templ_num	dw	? ; ����� � ����, �� �� �������� ����� ����
	
	; ������ ��������
	OLC			db	20 dup (?)
	MLCB		db	20 dup (?)
	MLCE		db	20 dup (?)
	TD			db	20 dup (?)
	
.data
	EOL			db	0Dh,0Ah,0,0
	
.code

equal proc templ:DWORD
; # ����� ��������� ������ �������� [templ] � ����� � ������� [esi].
;
;	����:
;		templ - ��������� �� ������ ��������, ����������� #0. � ����� ��� ���� #0.
;		esi - ��������� �� ����� �����.
;	�����:
;		eax:
;			-1	: ����� �� ����� ������ �����
;			#0	: ����� �� ����� ������ ��������
;			any	: ����� � ������ � �������� ������ ������
;		ecx - ����� ���������� �������
;		edi - ��������� �� #0 �� ��������� ��������
;		esi - ��������� �� ������ �� ������ ���������� ������� � ������ �����
;		templ_num - ����� ���������� ������� � ������.

	LOCAL	_esi : DWORD
	
	mov		_esi,	esi
	
	push	bx
	xor		bx,		bx ; ������� templ_num
	
	mov		eax,	templ
	mov		edi,	eax
	xor		ecx,	ecx ; ����� �������� �������
@@loop:

	mov		al,		byte ptr [esi]	; ������ �� ������
; ------------------------------------------------------------------------
	mov		ah,		byte ptr [edi]	; ������ �� �������
	cmp		ah,		0				; ������� �� ����� ������� ?
	jne		@F
		; ����� ������� - ���� ����������
		mov		eax,	esi
		sub		eax,	ecx
		
		mov		templ_num,	bx
		pop		bx
		
		ret
@@:
; ------------------------------------------------------------------------
	cmp		al,		0				; ������� �� ����� ������ ?
	jne		@F
		; ����� ������ ����� - �� ������� ����������
		xor		eax,	eax
		dec		eax				; eax = -1 (����� �� ����� ������ �����)
		
		pop		bx
		
		ret
@@:
; ------------------------------------------------------------------------
	cmp		al,		ah
	jne		@F
		inc		esi
		inc		edi
		inc		ecx
		jmp		@@loop
@@:
	
; >>> ����������� ... >>>
	; �������������� ����� �����
	mov		esi,	_esi
	; ��������� ����� �������� �������
	xor		ecx,	ecx
	; ���������� ������� ��������
	inc		bx
	
	; ��� ����� �������� �������
@@:
	inc		edi
	cmp		byte ptr [edi-1],	0
	jne		@B
; <<< ����������� ... <<<
	
	; � �� ����� �� ������ �������� ?
	cmp		byte ptr [edi],	0
	jne		@@loop

	; �����, ������...
	; ���������� ���, ������
	xor		eax,	eax		; eax = 0 (����� �� ����� ������ ��������)
	
	pop		bx

	ret
equal endp

equal_from_pos proc templ:DWORD, from:DWORD
; # ����� ��������� ������ �������� [templ] � ����� �� ������ [from] � �� ����� ������ (#0).
;
;	����:
;		templ - ��������� �� ������ ��������, ����������� #0. � ����� ��� ���� #0.
;		from - ����� ������ �����. 
;	�����:
;		eax:
;			#0	: ������ �� ������
;			any	: ����� � ������ � �������� ������ ������
;		ecx - ����� ���������� �������
;		edi - ��������� �� #0 �� ��������� ��������
;	������:
;		esi - �� ��������

	push	esi
	
	mov		esi,	from
	
@@loop:
	invoke	equal, templ
	cmp		eax,	-1
	jne		@F
		; ����� ������ ����� - ������ �� ����� � ������
		pop		esi
		ret
@@:

	cmp		eax,	0		; "test eax, eax" ?
	jne		@F			; ���� �������� ������, �� ��� ������
		inc		esi
		jmp		@@loop
@@:
	
	; ���-�� ������� - ������
	pop		esi 

	ret
equal_from_pos endp

it_is proc templ:DWORD
; # �������� �� ������� � ������ ������� ������� �������.
;
;	����:
;		templ - ��������� �� ������ ��������, ����������� #0. � ����� ��� ���� #0.
;		esi - ����� ������ �����.
;	�����:
;		eax - (!0/0) : ��������� ��� ���
;	������:
;		esi - �� ��������

	push	esi
	invoke	equal,	templ
	pop		esi
	
	cmp		eax,	-1
	jne		@F
		inc		eax
@@:	
	
	ret
it_is endp

ProcessingOneFile proc
; # ��������� �����, ��� ������������ � Edit.

	; "����� ���������� �������" ������ ��������� � ������� ������������
	LOCAL	beg_templ_num : WORD

	mov		eax,	old_file_size
	cmp		eax,	0
	jne		@F
		ret
@@:

	push	esi
	push	edi
	
	; ����� ����������� ������ � ����� ������������ ��� �����
	
	invoke	LocalLock, buff
	mov		buff_ptr,	eax
	
	mov		esi,	eax
	
@@loop:

; #################################### OLC ####################################
	invoke	it_is, addr OLC
	cmp		eax,	0
	je		@@OLC_jump
		mov		edx,	eax
		add		eax,	ecx
		
		invoke	equal_from_pos, addr EOL, eax
		.IF (eax==-1)
			; ����� �� ����� �����
			; !!! �.�. OLC ����� ���� ��������� � �����, ������� ����� � �� ��������� ��������������� ����������� ������,
			; �� ���������� ���������, ��� (edx) �� ��������� �� OLC
			mov		esi,	edx
			invoke	it_is, addr OLC
			cmp		eax,	0
			je		@@isnt_end
				; � ������ ���� ...
				
				sub		eax,	buff_ptr
				invoke	DelConfirm, edx, eax
				cmp		eax,	IDYES
				jne		@F
					mov		byte ptr [esi], 0 ; � ���� ? 
					; ������������ ������ ������ �����
					mov		eax,	esi
					sub		eax,	buff_ptr
					mov		old_file_size,	eax
@@:

@@isnt_end:

			jmp		@@exit
		.ELSE
			add		eax, ecx	; eax = eax + length(EOL)
			push	eax
			mov		esi, edx

			; ������ ���� ��������� � ��������� [edx..eax]
			; ������ ���
			mov		ecx, buff_ptr
			sub		edx, ecx
			sub		eax, ecx
			dec		eax
			dec		eax
			
			invoke	DelConfirm, edx, eax
			.IF (eax == IDYES)
				pop		eax
			.ELSE
				pop		esi
				cmp		eax,	IDCANCEL
				jne		@F
					invoke	LocalUnlock, buff
					xor		eax,	eax
					jmp		@@end_func
@@:
			.ENDIF

			jmp	@@continue
		.ENDIF
@@OLC_jump:

; #################################### MLC ####################################
	invoke	it_is, addr MLCB
	cmp		eax,	0
	je		@@MLC_jump
		mov		edx,	eax
		add		eax,	ecx
		
		; �������� ����� ���������� �������
		m2m		beg_templ_num,	templ_num

; ���� ������ ���������������� ������� ����� �������������� �����������
@@tmpl_num_loop:
		invoke	equal_from_pos, addr MLCE, eax
		.IF (eax==-1)
			; ����� �� ����� �����
			jmp		@@exit
		.ELSE
		
			; ��������� ������ ��������
			mov		si,		beg_templ_num
			cmp		si,		templ_num
			je		@F
				; ����� ������ ������, ��� ������
				add		eax,	ecx	; eax = eax + length(MLCE)
				jmp		@@tmpl_num_loop
@@:
			
			; ���������� �������, ��������� ������ ...
			
			add		eax, ecx	; eax = eax + length(MLCE)
			push	eax
			mov		esi, edx

			; ������ ���� ��������� � ��������� [edx..eax]
			; ������ ���			
			mov		ecx, buff_ptr
			sub		edx, ecx
			sub		eax, ecx
			
			invoke	DelConfirm, edx, eax
			.IF (eax == IDYES)
				pop		eax
			.ELSE
				pop		esi
				cmp		eax,	IDCANCEL
				jne		@F
					invoke	LocalUnlock, buff
					xor		eax,	eax
					jmp		@@end_func
@@:
			.ENDIF
			
			jmp	@@continue
		.ENDIF
@@MLC_jump:

; #################################### TD #####################################
	invoke	it_is, addr TD
	cmp		eax,	0
	je		@@TD_jump
		mov		edx,	eax
		add		eax,	ecx
		
		invoke	equal_from_pos, addr TD, eax
		.IF (eax==-1)
			; ����� �� ����� �����
			jmp		@@exit
		.ELSE
			add	eax, ecx	; eax = eax + length(TD)
			mov	esi, eax

			jmp	@@continue
		.ENDIF
@@TD_jump:

; #############################################################################
@@continue:

	; �������� �� ����� ������ �����
	cmp		byte ptr [esi], 0
	je		@@exit	; ���� �����, �� ������
	inc		esi
	jmp		@@loop

@@exit:

	invoke	LocalUnlock, buff

	xor		eax,	eax
	inc		eax

@@end_func:

	pop		edi
	pop		esi
	
	ret
ProcessingOneFile endp

ViewFile proc Path:DWORD
; # ����� ������ � �����.
;
;	����:
;		Path - ���� � �����, ����� "C:\Windows\"
;	�����:
;		eax - (0/1) - ��������/���������� ������

	LOCAL	data : WIN32_FIND_DATA
	LOCAL	fh : DWORD
	
	; ��������� ����� � ���� ����������
	invoke	FindFirstFile, FileFormat, addr data
	mov		fh,	eax
	
@@loop:
	cmp		fh,	INVALID_HANDLE_VALUE
	je		@@break
	
	mov		eax,	dword ptr [data.cFileName]
	
	cmp		ax,		2Eh		; '.',0
	je	@F
	
	and		eax,	0FFFFFFh
	cmp		eax,	2E2Eh	; '..',0
	je	@F
	
	; � ������ ������ �������������� ��� ����� ����� ������ ��� ����

	mov		eax,	data.dwFileAttributes
	and		eax,	FILE_ATTRIBUTE_DIRECTORY
	jnz		@@find_file
	
		invoke	strCopy, tmp_str, addr data.cFileName
		invoke	szLen,	tmp_str
		add		eax,	tmp_str
		mov		word ptr [eax], 003Fh ; ������������ �������� � ����� ����� ����� '?', ����� �������� �� �����	

		invoke	TreeView_InsertItem, hTmpTreeView, item, tmp_str
	
		invoke	LoadFromFile, addr data.cFileName
		invoke	ProcessingOneFile
		cmp		eax,	0
		jne		@F
			invoke	FindClose, fh
			xor		eax,	eax
			ret
@@:
		invoke	SaveToFile, addr data.cFileName

@@find_file:

	invoke	FindNextFile, fh, addr data
	cmp		eax,	0
	jne		@@loop
	
@@break:

	invoke	FindClose, fh
	
	xor		eax,	eax
	inc		eax

	ret
ViewFile endp

ViewDir proc Path:DWORD
; # ����������� ����� �����.
;
;	����:
;		Path - ���� � �����, ����� "C:\Windows\"
;	�����:
;		eax - (0/1) - ��������/���������� ������

	LOCAL	data : WIN32_FIND_DATA
	LOCAL	fh : DWORD
	
	invoke SetCurrentDirectory, Path
	
	; ����������� ����� � ������ ����������
	invoke	ViewFile, Path
	cmp		eax,	0
	jne		@F
		ret
@@:

	; ��������� ����� � ���� ����������	
	invoke	FindFirstFile, SADD("*.*"), addr data
	mov		fh,	eax
	
@@loop:
	cmp		fh,	INVALID_HANDLE_VALUE
	je		@@break
	
	mov		eax,	dword ptr [data.cFileName]
	
	cmp		ax,		2Eh		; '.',0
	je	@F
	
	and		eax,	0FFFFFFh
	cmp		eax,	2E2Eh	; '..',0
	je	@F
	
	; � ������ ������ �������������� ��� ����� ����� ������ ��� ����
	mov		eax,	data.dwFileAttributes
	and		eax,	FILE_ATTRIBUTE_DIRECTORY
	jz		@@find_dir
	
		; �������� ����������
		invoke	TreeView_InsertItem, hTmpTreeView, item, addr data.cFileName
		mov		item,	eax
	
		invoke	ViewDir, addr data.cFileName ; ����������� ��������� ��������� �����
		cmp		eax,	0
		jne		@F
			invoke	FindClose, fh
			xor		eax,	eax
			ret
@@:

@@find_dir:
	invoke	FindNextFile, fh, addr data
	cmp		eax,	0
	jne		@@loop
	
@@break:

	invoke	FindClose, fh
	
	invoke	SetCurrentDirectory, SADD("..")
	
	mov		item,	TreeView_GetParent(hTmpTreeView, item)
	
	xor		eax,	eax
	inc		eax

	ret
ViewDir endp

FileProcess proc Path:DWORD, Format:DWORD ; ������� ���� � ������ ��� ������
; # ��������� ������.
;
;	����:
;		Path - ��������� ����, ����� "C:\Windows\"
;		Format - ������ ����� "*.pas"
;	�����:
;		---

	invoke	ShowWindow, WND, SW_MAXIMIZE

	m2m		FileFormat,	Format
	
	invoke	TreeView_InsertItem, hTmpTreeView, NULL, StartPath
	
	mov		item,	TreeView_GetRoot(hTmpTreeView) ; ������, ^^^ eax ^^^
	
	invoke	ViewDir, Path
	cmp		eax,	0
	jne		@F
		invoke	MessageBox, WND, SADD("��������� ���� ��������!"), addr AppName, MB_OK or MB_ICONINFORMATION
@@:
	
	invoke	Clear
	
	invoke	ShowWindow, WND, SW_RESTORE
	
	invoke	FileListDlgShow
	
	ret
FileProcess endp