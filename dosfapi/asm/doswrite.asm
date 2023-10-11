;
; DOSWRITE.ASM
;
; Primitive character output
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

.CODE _TEXT

public DosWrite
DosWrite proc

push bp
mov bp, sp

; SYSERR APIENTRY
; DosWrite(
;     WORD wHandle,          [BP + 16]
;     PBYTE pBuffer,         [High BP + 14, Low BP + 12]
;     WORD wBufferLength,    [BP + 10]
;     PWORD pwBytesWritten   [High BP + 8, Low BP + 6]
; );

push ds
push bx

mov ds, [bp + 14]
mov dx, [bp + 12]
mov bx, [bp + 16]
mov cx, [bp + 10]

mov ah, 40h
int 21h

jb DosWriteFail

mov ds, [bp + 8]
mov bx, [bp + 6]
mov word ptr [bx], ax

xor ax, ax

DosWriteFail:

pop bx
pop ds

mov sp, bp
pop bp

ret 12

DosWrite endp

END
