bits 32
org 0x0
jmp _main_

_main_:
push ebp
mov ebp, esp
sub esp, 0x10
mov eax, 0x2
push eax
mov eax, 0x3
push eax
mov eax, 0x4
push eax
mov eax, 0x5
push eax
mov eax, 0x6
push eax
pop edx
pop eax
imod eax, edx
push eax
mov eax, 0x7
push eax
pop edx
pop eax
add  eax, edx
push eax
pop edx
pop eax
imul eax, edx
push eax
mov eax, 0x8
push eax
pop edx
pop eax
imod eax, edx
push eax
pop edx
pop eax
add  eax, edx
push eax
mov eax, 0x9
push eax
pop edx
pop eax
add  eax, edx
push eax
pop edx
pop eax
imul eax, edx
push eax
mov [ebp-0x8], eax
mov eax, 0x2
push eax
mov eax, [ebp-0x8]; e
push eax
mov eax, [ebp-0x8]; e
push eax
pop edx
pop eax
idiv eax, edx
push eax
mov eax, [ebp-0x8]; e
push eax
mov eax, 0x3
push eax
mov eax, [ebp-0x8]; e
push eax
pop edx
pop eax
imul eax, edx
push eax
pop edx
pop eax
add  eax, edx
push eax
pop edx
pop eax
imul eax, edx
push eax
mov eax, 0x4
push eax
mov eax, 0x5
push eax
mov eax, 0x6
push eax
pop edx
pop eax
imul eax, edx
push eax
mov eax, 0x7
push eax
pop edx
pop eax
add  eax, edx
push eax
mov eax, [ebp-0x8]; e
push eax
pop edx
pop eax
add  eax, edx
push eax
mov eax, [ebp-0x8]; e
push eax
mov eax, [ebp-0x8]; e
push eax
call _func_
pop edx
pop edx
push eax
pop edx
pop eax
add  eax, edx
push eax
pop edx
pop eax
imul eax, edx
push eax
pop edx
pop eax
imod eax, edx
push eax
pop edx
pop eax
add  eax, edx
push eax
mov [ebp-0xc], eax
mov eax, [ebp-0x4]; a
push eax
mov eax, 0x3
push eax
call _func_
pop edx
pop edx
push eax
mov eax, [ebp-0x8]; e
push eax
mov eax, [ebp-0xc]; fg
push eax
pop edx
pop eax
cmp eax, edx
jg  LABEL_while_begin_true_100
mov eax, 0x0
jmp LABEL_while_begin_end_100
LABEL_while_begin_true_100:
mov eax, 0x1
LABEL_while_begin_end_100:
push eax
pop eax
cmp eax, 0
je  LABEL97_while_end
LABEL97_while_begin:
mov eax, [ebp-0xc]; fg
push eax
mov eax, 0x1
push eax
pop edx
pop eax
add  eax, edx
push eax
mov [ebp-0xc], eax
mov eax, [ebp-0xc]; fg
push eax
mov eax, [ebp-0x8]; e
push eax
pop edx
pop eax
cmp eax, edx
je  LABEL_while_begin_true_113
mov eax, 0x0
jmp LABEL_while_begin_end_113
LABEL_while_begin_true_113:
mov eax, 0x1
LABEL_while_begin_end_113:
push eax
pop eax
cmp eax, 0
je  LABEL110_else
mov eax, [ebp-0x8]; e
push eax
mov [ebp-0xc], eax
jmp LABEL110_if_end
LABEL110_else:
mov eax, 0x0
push eax
mov eax, 0x1
push eax
pop edx
pop eax
sub  eax, edx
push eax
mov eax, [ebp-0xc]; fg
push eax
pop edx
pop eax
imul eax, edx
push eax
mov [ebp-0x8], eax
mov eax, [ebp-0xc]; fg
push eax
mov eax, 0x1
push eax
pop edx
pop eax
add  eax, edx
push eax
mov [ebp-0xc], eax
LABEL110_if_end:
mov eax, [ebp-0x8]; e
push eax
mov eax, [ebp-0xc]; fg
push eax
pop edx
pop eax
cmp eax, edx
jg  LABEL_while_end_true_100
mov eax, 0x0
jmp LABEL_while_end_end_100
LABEL_while_end_true_100:
mov eax, 0x1
LABEL_while_end_end_100:
push eax
pop eax
cmp eax, 0
jne LABEL97_while_begin
LABEL97_while_end:
mov eax, 0x3
push eax
mov [ebp-0x4], eax
mov eax, 0x0
push eax
pop eax
jmp _program_halt_

_func_:
push ebp
mov ebp, esp
sub esp, 0x10
mov eax, 0x3
push eax
mov [ebp-0x4], eax
mov eax, 0x4
push eax
mov [_x_], eax
mov eax, [ebp+0x8]; c
push eax
mov eax, [ebp+0xc]; b
push eax
mov eax, [_x_]; x
push eax
pop edx
pop eax
imul eax, edx
push eax
pop edx
pop eax
add  eax, edx
push eax
mov eax, [ebp-0x4]; f
push eax
pop edx
pop eax
imod eax, edx
push eax
pop eax
leave
ret

_program_halt_:
hlt

_x_:
dd 0x0

_y_:
dd 0x0

_z_:
dd 0x0
