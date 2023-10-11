;
; KEYBRD.ASM
;
; Read from the keyboard
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

public KbdCharIn
KbdCharIn proc

push bp
mov bp, sp

; SYSERR APIENTRY
; KbdCharIn(
;     PKBDKEYINFO pCharData, [High BP + 12, Low BP + 10]
;     WORD wWait,            [BP + 8]
;     HKBD hkbd              [BP + 6]
; );

push ds
push bx
mov ah, 0h
int 16h
mov ds, [bp + 12]
mov bx, [bp + 10]

mov word ptr [bx], ax
xor ax, ax
mov word ptr [bx + 2], ax
mov word ptr [bx + 4], ax
mov word ptr [bx + 6], ax
mov word ptr [bx + 8], ax

pop bx
pop ds

mov sp, bp
pop bp

ret 8

KbdCharIn endp

END
