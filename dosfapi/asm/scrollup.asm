;
; SCROLLUP.ASM
;
; Scroll the video up.
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

public VioScrollUp
VioScrollUp proc

push bp
mov bp, sp

; VOID
; VioScrollUp(
;     WORD wTopRow,          [BP + 20]
;     WORD wLeftColumn,      [BP + 18]
;     WORD wBottomRow,       [BP + 16]
;     WORD wRightColumn,     [BP + 14]
;     WORD wBlankLines,      [BP + 12]
;     PWORD pwCell,          [High BP + 10, Low BP + 8]
;     HVIO hvio              [BP + 6]
; );

push ds
push bx
mov ds, [bp + 10]
mov bx, [bp + 8]
mov bx, [bx]
mov ch, [bp + 20]
mov cl, [bp + 18]
mov dh, [bp + 16]
mov dl, [bp + 14]
mov al, [bp + 12]
mov ah, 06h
int 10h


pop bx
pop ds

xor ax, ax

mov sp, bp
pop bp

ret 16

VioScrollUp endp

END
