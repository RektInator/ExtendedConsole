_TEXT SEGMENT

; Hook variables
EXTERN OnFrameHook_JumpBack_Jz:qword
EXTERN OnFrameHook_JumpBack:qword
EXTERN Console_OnFrame:PROC
EXTERN OnFrameHook_Func:qword
EXTERN OnFrameHook_CmpAddr:qword

; OnFrameHook stub
PUBLIC OnFrameHookStub
OnFrameHookStub PROC

	call OnFrameHook_Func;
	call Console_OnFrame;

	mov rax, OnFrameHook_CmpAddr;
	cmp DWORD PTR [rax], 0;
	jz zero;
	
	mov rax, OnFrameHook_JumpBack;
	jmp rax;

zero:
	mov rax, OnFrameHook_JumpBack_Jz;
	jmp rax;

OnFrameHookStub ENDP

_TEXT ENDS
END
