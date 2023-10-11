;
; CURSOR.ASM
;
; Get and set cursor position.
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

public VioGetCurPos
VioGetCurPos proc

push bp
mov bp, sp

; VOID
; VioGetCurPos(
;     PWORD pwRow,           [High BP + 14, Low BP + 12]
;     PWORD pwColumn,        [High BP + 10, Low BP + 8]
;     HVIO hvio              [BP + 6]
; );

push ds
push bx
mov ah, 03h
mov bh, 0h
int 10h

mov ds, [bp + 14]
mov bx, [bp + 12]

mov byte ptr [bx], dh
mov byte ptr [bx + 1], 0

mov ds, [bp + 10]
mov bx, [bp + 8]

mov byte ptr [bx], dl
mov byte ptr [bx + 1], 0

pop bx
pop ds

xor ax, ax

mov sp, bp
pop bp

ret 10

VioGetCurPos endp

public VioSetCurPos
VioSetCurPos proc

push bp
mov bp, sp

; VOID
; VioSetCurPos(
;     WORD pwRow,            [BP + 10]
;     WORD pwColumn,         [BP + 8]
;     HVIO hvio              [BP + 6]
; );

push bx
mov ah, 02h
mov bh, 0h
mov dh, [bp + 10]
mov dl, [bp + 8]
int 10h

pop bx

xor ax, ax

mov sp, bp
pop bp

ret 6

VioSetCurPos endp

END
