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
		var_LTRL1 sdword 2
		var_LTRL2 sdword 0
		var_LTRL3 byte 'StartDemo', 0
		var_LTRL4 sdword 10
		var_LTRL5 sdword 3
		var_LTRL6 byte 'x = 10, y = 3', 10, 0
		var_LTRL7 byte 'x + y = ', 0
		var_LTRL8 byte 10, 0
		var_LTRL9 byte 'x - y = ', 0
		var_LTRL10 byte 10, 0
		var_LTRL11 byte 'x * y = ', 0
		var_LTRL12 byte 10, 0
		var_LTRL13 byte 'x / y = ', 0
		var_LTRL14 byte 10, 0
		var_LTRL15 byte 'x % y = ', 0
		var_LTRL16 byte 10, 0
		var_LTRL17 byte 'Bitwise OR (x | y): ', 0
		var_LTRL18 byte 10, 0
		var_LTRL19 byte 'Bitwise AND (x & y): ', 0
		var_LTRL20 byte 10, 0
		var_LTRL21 sdword 1
		var_LTRL22 byte 'Bitwise NOT (~vbn): ', 0
		var_LTRL23 byte 10, 0
		var_LTRL24 byte 'Logic & If:', 10, 0
		var_LTRL25 sdword 0
		var_LTRL26 byte 10, 0
		var_LTRL27 byte 'Flag is true.', 10, 0
		var_LTRL28 byte 'x is not equal to y', 10, 0
		var_LTRL29 byte 'Function & Bool:', 10, 0
		var_LTRL30 byte 'Check if 4 is even: ', 0
		var_LTRL31 sdword 4
		var_LTRL32 byte 'Yes', 10, 0
		var_LTRL33 byte 'Strings:', 10, 0
		var_LTRL34 byte 'Hello', 9, 'Wd', 0
		var_LTRL35 byte 'String with tab: ', 0
		var_LTRL36 byte 10, 0
		var_LTRL37 byte 'Length of string: ', 0
		var_LTRL38 byte 10, 0
		var_LTRL39 byte 'Int to String: ', 0
		var_LTRL40 sdword 12345
		var_LTRL41 byte 10, 0
		var_LTRL42 byte 10, 0
		var_LTRL43 byte 'Do-While Loop:', 10, 0
		var_LTRL44 byte 'Iter: ', 0
		var_LTRL45 byte 10, 0
		var_LTRL46 byte 10, 0
		var_LTRL47 sdword 100
		var_LTRL48 byte 9, 0
		var_LTRL49 byte 10, 0
		var_LTRL50 byte 10, 0
		var_LTRL51 sdword 250
		var_LTRL52 byte 'End of Demo', 10, 0
.data
		temp sdword ?
		buffer byte 256 dup(0)
		var_sumres dword 0
		var_x dword 0
		var_y dword 0
		var_varname byte 0
		var_s dword 0
		var_flag dword 0
		var_not_vbn byte 0
		var_vbn byte 0
		var_text dword 0
		var_ss dword 0
		var_e byte 0
		var_o byte 0
		var_b byte 0
		var_u byte 0
		var_bit_or byte 0
		var_overflow_test byte 0
.code

;----------- sum ------------
sum PROC,
	var_suma : dword, var_sumb : dword  
; --- save registers ---
push ebx
push edx
; ----------------------
push var_suma
push var_sumb
pop ebx
pop eax
add eax, ebx
push eax
pop eax
mov var_sumres, eax

push var_sumres
pop eax
; --- restore registers ---
pop edx
pop ebx
; -------------------------
ret
sum ENDP
;------------------------------


;----------- isEven ------------
isEven PROC,
	var_isEvenv : dword  
; --- save registers ---
push ebx
push edx
; ----------------------
push var_isEvenv
push var_LTRL1
pop ebx
pop eax
cdq
mov edx,0
idiv ebx
push edx
push var_LTRL2
pop ebx
pop eax
cmp eax, ebx
sete al
movzx eax, al
push eax
pop eax
; --- restore registers ---
pop edx
pop ebx
; -------------------------
ret
isEven ENDP
;------------------------------


;----------- MAIN ------------
main PROC
push offset var_LTRL3
call outrad

push var_LTRL4
pop eax
mov var_x, eax

push var_LTRL5
pop eax
mov var_y, eax

push offset var_LTRL6
call outrad

push offset var_LTRL7
call outrad


push var_y
push var_x
call sum
push eax
call outlich

push offset var_LTRL8
call outrad

push offset var_LTRL9
call outrad

push var_x
push var_y
pop ebx
pop eax
sub eax, ebx
push eax
call outlich

push offset var_LTRL10
call outrad

push offset var_LTRL11
call outrad

push var_x
push var_y
pop ebx
pop eax
imul eax, ebx
push eax
call outlich

push offset var_LTRL12
call outrad

push offset var_LTRL13
call outrad

push var_x
push var_y
pop ebx
pop eax
cdq
idiv ebx
push eax
call outlich

push offset var_LTRL14
call outrad

push offset var_LTRL15
call outrad

push var_x
push var_y
pop ebx
pop eax
cdq
mov edx,0
idiv ebx
push edx
call outlich

push offset var_LTRL16
call outrad

push offset var_LTRL17
call outrad

push var_x
push var_y
pop ebx
pop eax
or eax, ebx
push eax
call outlich

push offset var_LTRL18
call outrad

push offset var_LTRL19
call outrad

push var_x
push var_y
pop ebx
pop eax
and eax, ebx
push eax
call outlich

push offset var_LTRL20
call outrad

push var_LTRL21
pop eax
mov byte ptr [var_vbn], al

movzx eax, byte ptr [var_vbn]
push eax
pop eax
not eax
push eax
pop eax
mov byte ptr [var_not_vbn], al

push offset var_LTRL22
call outrad

movzx eax, byte ptr [var_not_vbn]
push eax
call outlich

push offset var_LTRL23
call outrad

push offset var_LTRL24
call outrad

push var_LTRL25
pop eax
cmp eax, 0
sete al
movzx eax, al
push eax
pop eax
mov var_flag, eax

push var_flag
call outlich

push offset var_LTRL26
call outrad

push var_flag
pop eax
cmp eax, 0
je lbl_if_end_1

push offset var_LTRL27
call outrad

lbl_if_end_1:

push var_x
push var_y
pop ebx
pop eax
cmp eax, ebx
setne al
movzx eax, al
push eax
pop eax
cmp eax, 0
je lbl_if_end_2

push offset var_LTRL28
call outrad

lbl_if_end_2:

push offset var_LTRL29
call outrad

push offset var_LTRL30
call outrad


push var_LTRL31
call isEven
push eax
pop eax
cmp eax, 0
je lbl_if_end_3

push offset var_LTRL32
call outrad

lbl_if_end_3:

push offset var_LTRL33
call outrad

push offset var_LTRL34
pop eax
mov var_s, eax

push offset var_LTRL35
call outrad

push var_s
call outrad

push offset var_LTRL36
call outrad

push offset var_LTRL37
call outrad


push var_s
push offset buffer
call strlen
push eax
call outlich

push offset var_LTRL38
call outrad

push offset var_LTRL39
call outrad


push var_LTRL40
push offset buffer
call tostring
push eax
call outrad

push offset var_LTRL41
call outrad


push var_x
push offset buffer
call tostring
push eax
pop eax
mov var_ss, eax

push var_ss
call outrad

push offset var_LTRL42
call outrad

push offset var_LTRL43
call outrad

push var_LTRL2
pop eax
mov var_x, eax

lbl_do_4:

push offset var_LTRL44
call outrad

push var_x
call outlich

push offset var_LTRL45
call outrad

push var_x
push var_LTRL21
pop ebx
pop eax
add eax, ebx
push eax
pop eax
mov var_x, eax

push var_x
push var_LTRL5
pop ebx
pop eax
cmp eax, ebx
setne al
movzx eax, al
push eax
pop eax
cmp eax, 0
jne lbl_do_4

push var_LTRL2
pop eax
mov byte ptr [var_e], al

push var_LTRL1
movzx eax, byte ptr [var_e]
push eax
pop ebx
pop eax
add eax, ebx
push eax
call outlich

push offset var_LTRL46
call outrad

push var_LTRL4
pop eax
mov byte ptr [var_o], al

push var_LTRL47
pop eax
mov byte ptr [var_b], al

movzx eax, byte ptr [var_o]
push eax
movzx eax, byte ptr [var_b]
push eax
push var_LTRL1
pop ebx
pop eax
cdq
idiv ebx
push eax
pop ebx
pop eax
add eax, ebx
push eax
pop eax
mov byte ptr [var_u], al

push offset var_LTRL48
call outrad

movzx eax, byte ptr [var_u]
push eax
call outlich

push offset var_LTRL49
call outrad

movzx eax, byte ptr [var_o]
push eax
movzx eax, byte ptr [var_b]
push eax
pop ebx
pop eax
or eax, ebx
push eax
pop eax
mov byte ptr [var_bit_or], al

movzx eax, byte ptr [var_bit_or]
push eax
call outlich

push offset var_LTRL50
call outrad

push var_LTRL51
pop eax
mov byte ptr [var_overflow_test], al

push offset var_LTRL52
call outrad


ret
push 0
call ExitProcess
main ENDP
end main
