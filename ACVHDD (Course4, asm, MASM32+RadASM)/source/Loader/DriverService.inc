.const
	DRIVER_NAME		db 'ACVirtHDD',0
	DRIVER_CAPTION	db 'AterCattus Virtual HDD',0
	DRIVER_TARGET	db '\Device\ACVirtDrive',0
	DRIVER_FILE		db '\acvhdd.sys',0
	
	IOCTL_UNMOUNT_DISK	equ CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
	IOCTL_MOUNT_DISK	equ CTL_CODE(FILE_DEVICE_UNKNOWN, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
	IOCTL_SET_KEY_DISK	equ CTL_CODE(FILE_DEVICE_UNKNOWN, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
	
	OPEN_IMG_FILTER	db '���� ������ ����� (*.ACVH)', 0, '*.acvh', 0, 0
	
	MAXSIZE			equ	1024
	
	FIND_DRIVER_NOT_FOUND	equ	0 ; ������� �� ������
	FIND_DRIVER_STOPPED		equ	1 ; ������, �� �� ��������
	FIND_DRIVER_RUNNING		equ	2 ; ������ � ��������
	

.data?
	BufFileName		db MAXSIZE+1 dup (?)
	; �� ������ NewImageDlg.inc
	hProgressBar	DWORD ?

.data
	bDriverInMem	dd	0 ; �������� �� �������?
	DiskName		db '\\.\'
	DiskLetter		db 'D:',0,0

.code

	; ����� ��������� ���������� � ������������ ������
AC_ErrorMessage proto error:DWORD
	; ����������� �������� �� ������� � ������
	; ������������ �������� - FIND_DRIVER_
AC_FindDriverInMem proto
	; ����������� ������� � SCManager
	; ���������� TRUE - ������ �������, ����� FALSE
AC_RegisterService proto
	; �������� ������ ����� � �������, �������� �����-������ � ������ �������
	; ���� (!lpFileName), �� ���� ������ ���������� � ���������� ����, ����� ���� ������� �� ���������� ������
AC_StartService proto lpFileName:DWORD
	; ��������� �������, �������� �����-������ � �������� ����� �� �������
AC_StopService proto
	; �������� ������� �� SCManager
AC_DeleteService proto
	; �������� ������ �����-������
AC_CreateNewImage proto imgsize:DWORD, imgpath:DWORD
	; �������� �������� �����-������ (� ���������� �����)
AC_DeleteImage proto

AC_Cmd_LoadDriver proto lpFileName:DWORD
AC_Cmd_UnloadDriver proto

AC_ErrorMessage proc error:DWORD
LOCAL	buffer	: DWORD
; ����� ������ ������� GetLastError() ������?

	invoke	FormatMessage,	FORMAT_MESSAGE_ALLOCATE_BUFFER or FORMAT_MESSAGE_FROM_SYSTEM, \
							NULL, error, 0, addr buffer, 0, 0
	invoke	MessageBox, NULL, buffer, addr APPLICATION_NAME, MB_OK
	invoke	LocalFree, buffer

	ret
AC_ErrorMessage endp

AC_FindDriverInMem proc
LOCAL	_ss			: SERVICE_STATUS
LOCAL	hSCManager	: HWND
LOCAL	hService	: HWND
LOCAL	status		: DWORD

	mov		status, 		FIND_DRIVER_NOT_FOUND
	mov		bDriverInMem,	FALSE

	invoke	OpenSCManager, NULL, NULL, SC_MANAGER_CONNECT
	.if (eax != NULL)
		mov		hSCManager, eax
		invoke	OpenService, eax, addr DRIVER_NAME, SERVICE_QUERY_STATUS
		.if (eax != NULL)
			mov		hService, eax
			invoke	RtlZeroMemory, addr _ss, sizeof _ss
			invoke	QueryServiceStatus, hService, addr _ss
			
			.if (_ss.dwCurrentState==SERVICE_RUNNING)
				mov status, 		FIND_DRIVER_RUNNING
				mov	bDriverInMem,	TRUE
			.elseif (_ss.dwCurrentState==SERVICE_STOPPED)
				mov status, 		FIND_DRIVER_STOPPED
				mov	bDriverInMem,	TRUE
			.else
				invoke MessageBox,	hMainWnd, SADD("��������� ������! ��������� ���������� �������"),
									addr APPLICATION_NAME, MB_OK or MB_ICONERROR
			.endif
			
			invoke CloseServiceHandle, hService
		.endif
		invoke CloseServiceHandle, hSCManager
	.endif
	
	mov	eax, status
	ret
AC_FindDriverInMem endp

AC_RegisterService proc
LOCAL	hSCManager	: HWND
LOCAL	status		: DWORD
LOCAL	cur_dir[MAX_PATH] : BYTE

	mov		status, FALSE
	invoke	OpenSCManager, NULL, NULL, SC_MANAGER_ALL_ACCESS
	.if (eax != NULL)
		mov		hSCManager, eax
		
		invoke	CreateService,	hSCManager, addr DRIVER_NAME, addr DRIVER_CAPTION, 0, \
								SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, \
								addr driver_path, NULL, NULL, NULL, NULL, NULL
		
		.if (eax != NULL)
			invoke	CloseServiceHandle, eax
			mov	status, TRUE
		.else
			mov	status, FALSE
			invoke	AC_ErrorMessage, $invoke(GetLastError)
		.endif
		
		invoke	CloseServiceHandle, hSCManager
	.endif

	mov		eax, status
	ret
AC_RegisterService endp

AC_StartService proc lpFileName:DWORD
LOCAL	ofn					: OPENFILENAME
LOCAL	_ss					: SERVICE_STATUS
LOCAL	status				: DWORD
LOCAL	Letter[4]			: BYTE
LOCAL	dwBytesReturned		: DWORD
LOCAL	hDevice				: HWND
LOCAL	hSCManager			: HWND
LOCAL	hService			: HWND
LOCAL	hHeap				: HWND
LOCAL	pMemory				: PVOID
LOCAL	lpServiceArgVectors	: PVOID
LOCAL	password			: DWORD

; ������������� ����������
	mov		status,		FALSE
	mov 	hDevice,	NULL
	mov 	hSCManager,	NULL
	mov 	hService,	NULL
	mov 	hHeap,		NULL
	mov 	pMemory,	NULL
	
; �������� �����-������
	.IF (lpFileName == NULL)
		; ���� �� ������. ��������� ������
		invoke	RtlZeroMemory, addr ofn, sizeof OPENFILENAME
		invoke	RtlZeroMemory, addr BufFileName, MAXSIZE
		
		mov		ofn.lStructSize,	sizeof OPENFILENAME
		m2m		ofn.hwndOwner,		hMainWnd
		m2m		ofn.hInstance,		hInstance
		mov		ofn.lpstrFilter,	offset OPEN_IMG_FILTER
		mov		ofn.lpstrFile,		offset BufFileName
		mov		ofn.nMaxFile,		MAXSIZE
		mov		ofn.Flags,			OFN_FILEMUSTEXIST or OFN_PATHMUSTEXIST or OFN_LONGNAMES or \
									OFN_EXPLORER or OFN_HIDEREADONLY
									
		invoke	GetOpenFileName, addr ofn
		or		eax,	eax
		jz		ACSS_exit
	.ELSE
		; ���� ������. ��������� ���� ����
		mov		byte ptr BufFileName,	0
		invoke	StrCatBuff, offset BufFileName, lpFileName, MAXSIZE
	.ENDIF
	
; �������� ������� ����� (����� ����� ��, ������ 0, �����������)
	invoke	CreateFile,	addr BufFileName, GENERIC_READ or GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0
	.IF (eax == INVALID_HANDLE_VALUE)
@@AC_SS_err:
		invoke	MessageBox, hMainWnd, $CTA0('��������� ���� ����������'), addr APPLICATION_NAME, MB_OK or MB_ICONERROR
		return(status)
	.ELSE
		; ���� ���������� �������
		push	ebx
		mov		ebx,	eax
		invoke	GetFileSize, ebx, NULL
		.IF (eax == -1)
			; ������ ��������� ������� �����
			invoke	CloseHandle, ebx
			pop		ebx
			jmp		@@AC_SS_err
		.ENDIF
		; ������ � EAX
		mov		ecx,	eax
		and		ecx,	0FFF00000h	; ����� ��
		and		eax,	0000FFFFFh	; ����� ���� ����� ������ ����� ��
		.IF ((ecx == 0) || (eax != 0))
			invoke	MessageBox, hMainWnd, $CTA0('������ ����� ������ ���� ������ 1 ��'), addr APPLICATION_NAME, MB_OK or MB_ICONERROR
			invoke	CloseHandle, ebx
			pop		ebx
			return(status)
		.ENDIF
		invoke	CloseHandle, ebx
		pop		ebx
	.ENDIF
	
; ���� ������
	invoke	PasswordDlgShow
	.IF (byte ptr [eax] == 0)
		; ������ �� ������
		jmp ACSS_exit
	.ENDIF
	mov		password,	eax

; ��������� ��������
	invoke	HeapCreate, HEAP_NO_SERIALIZE, 1024, 1024
	.if (eax == NULL)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp	ACSS_error
	.endif
	mov		hHeap, eax
	invoke	HeapAlloc, eax, HEAP_ZERO_MEMORY, 1024
	.if (eax == NULL)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		invoke	HeapDestroy, hHeap
		jmp	ACSS_error
	.endif
	mov		pMemory, eax
	
	invoke	lstrcpyW, pMemory, $CTW0("\\??\\")
	invoke	lstrlen, addr BufFileName
	mov		edx, pMemory
	add		edx, 4*2
	invoke	MultiByteToWideChar, CP_ACP, 0, addr BufFileName, eax, edx, MAXSIZE
	
	invoke	OpenSCManager, NULL, NULL, SC_MANAGER_ALL_ACCESS
	.if (eax != NULL)
		mov		hSCManager, eax
	.else
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp		ACSS_error
	.endif
	
	invoke	OpenService, hSCManager, addr DRIVER_NAME, SERVICE_ALL_ACCESS
	.if (eax != NULL)		
		mov		hService, eax
		mov		lpServiceArgVectors, 0
		invoke	StartService, hService, 0, addr lpServiceArgVectors
		.if (eax != TRUE)
			invoke	AC_ErrorMessage, $invoke(GetLastError)
			jmp ACSS_error
		.endif
	.else
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp ACSS_error
	.endif
	
; ������ ���������
	mov		al,			[drive_letter]
	mov		[DiskLetter],	al
	invoke	lstrcpy, addr Letter, addr DiskLetter
	
	invoke	CreateFile,	offset DiskName, GENERIC_READ or GENERIC_WRITE,\
						FILE_SHARE_READ or FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0
	.if (eax != INVALID_HANDLE_VALUE)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp	ACSS_error
	.endif
	
	invoke	DefineDosDevice, DDD_RAW_TARGET_PATH, addr Letter, addr DRIVER_TARGET
	.if (eax==TRUE)
		invoke	CreateFile,	offset DiskName, GENERIC_READ or GENERIC_WRITE,\
							FILE_SHARE_READ or FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0
		.if (eax != INVALID_HANDLE_VALUE)
			mov		hDevice, eax
			; ������� �������� ����
			invoke	szLen, password
			mov		edx,	eax
			invoke	DeviceIoControl, hDevice, IOCTL_SET_KEY_DISK, password, edx, NULL, 0, addr dwBytesReturned, 0
			.IF (eax == TRUE)
				invoke	lstrlenW, pMemory
				add		eax, eax
				add		eax, 2
				mov		edx, eax
				invoke	DeviceIoControl, hDevice, IOCTL_MOUNT_DISK, pMemory, edx, NULL, 0, addr dwBytesReturned, 0
				.if (eax==0)
					invoke	AC_ErrorMessage, $invoke(GetLastError)
					invoke	DefineDosDevice, DDD_REMOVE_DEFINITION, addr Letter, NULL
					invoke	ControlService, hService, SERVICE_CONTROL_STOP, addr _ss
				.else
					; �� OK
					mov		status,	TRUE
				.endif
			.endif
		.else
			invoke	AC_ErrorMessage, $invoke(GetLastError)
			invoke	DefineDosDevice, DDD_REMOVE_DEFINITION, addr Letter, NULL
			invoke	ControlService, hService, SERVICE_CONTROL_STOP, addr _ss
			jmp ACSS_error
		.endif
	.else
		invoke	AC_ErrorMessage, $invoke(GetLastError)
	.endif

ACSS_error:

	; ������������ ������
	.if (hDevice != NULL)
		invoke	CloseHandle, hDevice
	.endif
	.if (hService != NULL)
		invoke	CloseServiceHandle, hService
	.endif
	.if (hSCManager != NULL)
		invoke	CloseServiceHandle, hSCManager
	.endif
	.if (hHeap != NULL)
		invoke	RtlZeroMemory, pMemory, 1024
		invoke	HeapFree, hHeap, HEAP_NO_SERIALIZE, pMemory
		invoke	HeapDestroy, hHeap
	.endif

ACSS_exit:
	mov		eax, status
	ret
AC_StartService endp

AC_StopService proc
LOCAL	_ss				: SERVICE_STATUS
LOCAL	hDevice			: HWND
LOCAL	status			: DWORD
LOCAL	dwBytesReturned	: DWORD
LOCAL	hSCManager		: HWND
LOCAL	hService		: HWND

	mov		status, FALSE
	
	invoke	CreateFile, offset DiskName, GENERIC_READ or GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0
	.if (eax == INVALID_HANDLE_VALUE)
		invoke	GetLastError
		.if (eax == ERROR_SHARING_VIOLATION)
			; ����� ��������� � ������ ������ ���������� � �����. ��������
			invoke	MessageBox,	hMainWnd, $CTA0('���� ������������. ���������� � ������ ������ ����������'),
								addr APPLICATION_NAME, MB_OK or MB_ICONERROR
			mov		eax,	2 ; ������ ���, ����� ���������������� ������� �����
			ret
		.else
			invoke	AC_ErrorMessage, eax
			jmp		ACSS_error
		.endif
	.endif
	mov	hDevice, eax
	
	invoke	DeviceIoControl, hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, addr dwBytesReturned, NULL
	.if (eax == 0)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp		ACSS_error
	.endif
	
	invoke	DeviceIoControl, hDevice, IOCTL_UNMOUNT_DISK, NULL, 0, NULL, 0, addr dwBytesReturned, NULL
	.if (eax == 0)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp		ACSS_error
	.endif
	
	invoke	DeviceIoControl, hDevice, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, addr dwBytesReturned, NULL
	.if (eax == 0)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp		ACSS_error
	.endif
	
	invoke	DeviceIoControl, hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, addr dwBytesReturned, NULL
	.if (eax == 0)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
;		jmp		ACSS_error
	.endif

ACSS_error:

	invoke	FlushFileBuffers, hDevice
	invoke	CloseHandle, hDevice
	invoke	DefineDosDevice, DDD_REMOVE_DEFINITION, offset DiskName+4, 0
	.if (eax == 0)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
	.endif
	
ACSS_closedriver:

	invoke	OpenSCManager, NULL, NULL, SC_MANAGER_CONNECT
	.if (eax != NULL)
		mov		hSCManager, eax
		invoke	OpenService, hSCManager, addr DRIVER_NAME, SERVICE_STOP
		.if (eax == NULL)
			invoke	AC_ErrorMessage, $invoke(GetLastError)
			jmp		ACSS_exit
		.else
			mov		hService, eax
			invoke	RtlZeroMemory, addr _ss, sizeof _ss
			invoke	ControlService, hService, SERVICE_CONTROL_STOP, addr _ss
			.if (eax == 0)
				invoke	AC_ErrorMessage, $invoke(GetLastError)
			.else
				mov	status, TRUE
			.endif
		.endif
	.else
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		jmp		ACSS_exit
	.endif
	
ACSS_exit:
	invoke	CloseServiceHandle, hService
	invoke	CloseServiceHandle, hSCManager
	mov		eax, status
	
	ret
AC_StopService endp

AC_DeleteService proc
LOCAL	hSCManager	: HWND
LOCAL	hService	: HWND
LOCAL	status		: DWORD

	invoke	OpenSCManager, NULL, NULL, SC_MANAGER_ALL_ACCESS
	.if (eax != NULL)
		mov		hSCManager, eax
		invoke	OpenService, hSCManager, addr DRIVER_NAME, SC_MANAGER_ALL_ACCESS
		mov		hService, eax
		or		eax, eax
		jz		srvDeleteError
		invoke	DeleteService, eax
		or		eax, eax
		jz		srvDeleteError
		mov		status,	TRUE
	.else
srvDeleteError:
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		mov		status,	FALSE
	.endif
	
	invoke CloseServiceHandle, hService
	invoke CloseServiceHandle, hSCManager
	
	mov		eax, status
	ret
AC_DeleteService endp

AC_CreateNewImage proc imgsize:DWORD, imgpath:DWORD
LOCAL	status			: DWORD
LOCAL	hFile			: HWND
LOCAL	FileSize		: LONG
LOCAL	BytesWritten	: DWORD
LOCAL	Sectors			: DWORD
LOCAL	hDlgWnd			: DWORD

	invoke	CreateFile, imgpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
	.if (eax == INVALID_HANDLE_VALUE)
		invoke	AC_ErrorMessage, $invoke(GetLastError)
		xor		eax,	eax
		ret
	.endif
	mov		hFile, eax
	
	mov		status,		FALSE
	
	invoke	LocalAlloc, LMEM_FIXED, 1024*1024
	mov		Sectors,	eax
	
	; ���������� ������� ����� ������
	push	ebx
	; imgsize - ������ ����� � ��
	mov		ebx,	imgsize

	; ������ �������� �������
	mov		MayClose,	FALSE

	; ������� ���������� ��������� ProgressBar'�
	mov		eax,	ebx
	shl		eax,	16
	invoke	SendMessage, hProgressBar, PBM_SETRANGE, 0, eax
	invoke	SendMessage, hProgressBar, PBM_SETPOS, 0, 0
	invoke	SendMessage, hProgressBar, PBM_SETSTEP, 1, 0
	mov		hDlgWnd,	$invoke(GetWindowLong, hProgressBar, GWL_HWNDPARENT)
	
;	invoke	SetFilePointer, hFile, 1024*1024, NULL, FILE_BEGIN
;	invoke	SetEndOfFile, hFile
;	push	0
;	push	1024*1024
;	push	hFile
;	call	SetFileValidData
	
	.while (ebx > 0)
;;		invoke	SetFilePointer, hFile, 1024*1024, NULL, FILE_CURRENT
		invoke	WriteFile, hFile, Sectors, 1024*1024, addr BytesWritten, NULL
		invoke	FlushFileBuffers, hFile
;;		invoke	SetEndOfFile, hFile
;		
;		;invoke	SetFileValidData, hFile, 1024*1024*1024
;;		push	1024*1024
;;		push	hFile
;;		call	SetFileValidData
		
		dec		ebx
		; ��� �� ������ �� ���-������ ������������� � ������� ��������� ���
		invoke	SendMessage, hProgressBar, PBM_STEPIT, 0, 0
		invoke	ProcessMessages, hDlgWnd
	.endw
	pop		ebx
	
	; ���������� �������� �������
	mov		MayClose,	TRUE

	invoke	LocalFree, Sectors
	
	mov		status,	TRUE

ACCNI_error:

;	invoke	FlushFileBuffers, hFile
	invoke	CloseHandle, hFile
	
	.if (status != TRUE)
		invoke	DeleteFile, imgpath
	.endif
	
	mov		eax,	status
	ret
AC_CreateNewImage endp

AC_Cmd_LoadDriver proc lpFileName:DWORD

; ����� ������������ ��������
	invoke	AC_FindDriverInMem
	.if (eax == FIND_DRIVER_STOPPED)
		; ���� ������� ������, �� �� ���������� - ������
		invoke	AC_DeleteService
	
	.elseif (eax == FIND_DRIVER_RUNNING)
		; ���� ������� ������, � �� ��������, �� �������
		
		invoke	MessageBox,	hMainWnd, $CTA0('������� ��� ��������. ������� ��� ����� ���������'),
							addr APPLICATION_NAME, MB_OK or MB_ICONERROR
		mov		eax,	-1 ; ������ ���
		ret
	
	.else
		; ���� ������� �� ������ - �� �������� 
	.endif
	
; �������� ��������
	invoke	AC_RegisterService
	.if	(eax == FALSE)
@@load_driver_error:
		invoke	MessageBox,	hMainWnd, $CTA0('������ ��� �������� ��������'),
							addr APPLICATION_NAME, MB_OK or MB_ICONERROR
		xor		eax,	eax
		ret
	.endif
	
	invoke	AC_FindDriverInMem
	.if (eax == FIND_DRIVER_NOT_FOUND)
		jmp	@@load_driver_error
	.endif
	.if (eax == FIND_DRIVER_RUNNING)
		; ������ ���� �� �����, �� �� ���� �� ;)
		jmp	 @@load_driver_error
	.endif

; �������� ����� ������
	invoke	AC_StartService, lpFileName
	.if (eax != TRUE)
		invoke	AC_DeleteService ; ???
		jmp		@@load_driver_error
	.endif
	
	invoke	AC_FindDriverInMem
	; � eax - ����������

	ret
AC_Cmd_LoadDriver endp

AC_Cmd_UnloadDriver proc

; ����� ������������ ��������
	invoke	AC_FindDriverInMem
	.if (eax == FIND_DRIVER_NOT_FOUND)
		; ���� ������� �� ������, �� ��������� �� ����� ������
		invoke	MessageBox,	hMainWnd, $CTA0('������� �� ��������'),
							addr APPLICATION_NAME, MB_OK or MB_ICONERROR
		xor		eax,	eax
	.else
		; ���� ������� ������
		.if (eax == FIND_DRIVER_RUNNING)
			invoke	AC_StopService
			.if (eax == 2)
				; ���� ������������. ������ � �������
				xor		eax,	eax
			.else
				invoke	AC_DeleteService
				; � eax - ���������
			.endif
		.endif
	.endif

	ret
AC_Cmd_UnloadDriver endp

AC_GetUsedDriveLetter proc
LOCAL buff[1024] : BYTE
LOCAL dn[4] : BYTE

	mov		dword ptr [dn], 00003A41H ; 'A:',0,0
@@AC_GUDL_loop:

	invoke	QueryDosDevice, addr dn, addr buff, 1024
	.IF (eax != 0)
		invoke	szCmp, addr buff, addr DRIVER_TARGET
		.IF (eax != 0)
			; ����� ����
			mov		al,		byte ptr [dn]
			;return(eax)
			ret
		.ENDIF
	.ENDIF
	lea		eax,	dn
	inc		byte ptr [eax]
	cmp		byte ptr [eax],	'Z'
	jna		@@AC_GUDL_loop
	
	return(0)
AC_GetUsedDriveLetter endp