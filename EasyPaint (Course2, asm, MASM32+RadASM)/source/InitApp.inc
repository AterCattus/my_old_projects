	invoke	CreateBackBuffer, WND, cWidth, cHeight, FALSE
	.IF (!eax)
		invoke	MessageBox, WND, SADD("������ ��� �������� ������ �����!"), SADD("������"), MB_OK or MB_ICONERROR
		invoke	PostQuitMessage, NULL
		invoke	ExitProcess, 0
	.ENDIF
	
	invoke	SetPenParam, FGColor, 1
	invoke	SetBrushColor, BGColor	
	
;	invoke	Clear