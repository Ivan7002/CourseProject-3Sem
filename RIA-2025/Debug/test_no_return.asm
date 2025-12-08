.586
.model flat, stdcall
includelib legacy_stdio_definitions.lib
includelib msvcrtd.lib
includelib vcruntimed.lib
includelib ucrtd.lib
includelib kernel32.lib
includelib "..\RIA-2025\Debug\StaticLibrary.lib"
ExitProcess PROTO:DWORD 
.stack 4096


 outlich PROTO : DWORD

 outrad PROTO : DWORD

 str_len PROTO : DWORD, : DWORD

 strlen EQU str_len

 tostring PROTO : DWORD, : DWORD

 atoii  PROTO : DWORD,  : DWORD

.const
		newline byte 13, 10, 0
		var_LTRL1 sdword 1
		var_LTRL2 sdword 5
.data
		temp sdword ?
		buffer byte 256 dup(0)
		var_badFunctiony dword 0
		var_result dword 0
.code

;----------- badFunction ------------
badFunction PROC,
	var_badFunctionx : dword  
; --- save registers ---
push ebx
push edx
; ----------------------
push var_badFunctionx
push var_LTRL1
pop ebx
pop eax
add eax, ebx
push eax
pop eax
mov var_badFunctiony, eax


;----------- MAIN ------------
main PROC

push var_LTRL2
call badFunction
push eax
pop eax
mov var_result, eax

push 0
call ExitProcess
main ENDP
end main
