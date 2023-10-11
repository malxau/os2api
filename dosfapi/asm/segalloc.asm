;
; SEGALLOC.ASM
;
; Allocate and free segments
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

public DosAllocSeg
DosAllocSeg proc

push bp
mov bp, sp

; SYSERR APIENTRY
; DosAllocSeg(
;     WORD wSize,            [BP + 12]
;     PSEL pSelector,        [High BP + 10, Low BP + 8]
;     WORD wfAlloc           [BP + 6]
; );

push ds
push bx
mov ax, [bp + 12]
cmp ax, 0fff0h
ja DosAllocSegEntireSegment

; Add 15 and divide by 4 to find the rounded up multiple in paragraphs
add ax, 0fh
shr ax, 1
shr ax, 1
shr ax, 1
shr ax, 1

; Treat zero as an entire segment
test ax, ax
jnz DosAllocSegAlloc

DosAllocSegEntireSegment:

mov ax, 1000h

DosAllocSegAlloc:

mov bx, ax
mov ah, 48h
int 21h

jnb DosAllocSegSuccess
jmp DosAllocSegReturn

DosAllocSegSuccess:

mov ds, [bp + 10]
mov bx, [bp + 8]

mov word ptr [bx], ax

xor ax, ax

DosAllocSegReturn:

pop bx
pop ds

mov sp, bp
pop bp

ret 8

DosAllocSeg endp

public DosFreeSeg
DosFreeSeg proc

push bp
mov bp, sp

; SYSERR APIENTRY
; DosFreeSeg(
;     WORD Selector          [BP + 6]
; );

push es
mov es, [bp + 6]
mov ah, 49h
int 21h

pop es
xor ax, ax

mov sp, bp
pop bp

ret 2

DosFreeSeg endp

END
