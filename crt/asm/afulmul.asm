;
; AFULMUL.ASM
;
; Implementation for far signed and unsigned multiplication of a 32 bit
; integer by a 32 bit integer.
;
; Copyright (c) 2017-2023 Malcolm J. Smith
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

public __aFulmul
__aFulmul proc

push bp
mov bp, sp

; DWORD [High DX, Low AX]
; __aFulmul(
;     DWORD Value1,          [High BP + 12, Low BP + 10]
;     DWORD Value2           [High BP + 8, Low BP + 6]
; );

; Multiply low by high of both components, adding them into a temporary
; register.  Multiply low by low, which might result in a high component,
; and add back the temporary result into this high component.

push bx

mov ax, [bp + 6]
mov dx, [bp + 12]
mul dx
mov bx, ax

mov ax, [bp + 10]
mov dx, [bp + 8]
mul dx
add bx, ax

mov ax, [bp + 6]
mov dx, [bp + 10]
mul dx
add dx, bx

pop bx
mov sp, bp
pop bp

ret 8
__aFulmul endp

END
