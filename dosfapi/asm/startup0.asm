;
; STARTUP0.ASM
;
; Initial program entry point for DOS programs.
;
; Copyright (c) 2023 Malcolm J. Smith
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
;

.MODEL large, pascal

.DOSSEG

__startup PROTO FAR PASCAL

.DATA
public __acrtused
__acrtused = 1234h

.DATA?
_EnvSelector    WORD ?
_CmdlineOffset  WORD ?

.STACK 6144

.CODE _TEXT
.STARTUP

mov word ptr [_EnvSelector], es
mov word ptr [_CmdlineOffset], 081h

call __startup

public GetProgramName
GetProgramName proc

xor dx, dx
xor ax, ax

ret

GetProgramName endp

public GetCommandArgs
GetCommandArgs proc

push ds
push si
mov si, [_CmdlineOffset]
mov ds, [_EnvSelector]

jmp GetCommandArgsStart
GetCommandArgsNextChar:
inc si
GetCommandArgsStart:
mov al, [si]
cmp al, 0dh
jnz GetCommandArgsNextChar

mov byte ptr [si], 0
mov dx, ds

pop si
pop ds

mov ax, [_CmdlineOffset]

ret

GetCommandArgs endp

END
