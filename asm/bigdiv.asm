;
; BIGDIV.ASM
;
; Implementation for for signed and unsigned division of a 32 bit integer
; by a 16 bit integer.
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

.MODEL large, pascal, FARSTACK, OS_OS2

.CODE


public __aFuldiv
__aFuldiv proc

push bp
mov bp, sp

; DWORD [High DX, Low AX]
; __aFuldiv(
;     DWORD Dividend, [High BP + 8, Low BP + 6],
;     DWORD Divisor [High BP + 12, Low BP + 10]
; );

; This implementation doesn't support 32 bit divisors.  If one is specified,
; fail.

mov ax, [bp + 12]
test ax, ax
jnz auldiv_overflow

push bx

; Divide the high 16 bits by the low 16 bits, save it in bx,
; and leave the remainder in dx.  Then, divide the low 16 bits
; plus the remainder, which must fit in a 16 bit value.  To satisfy
; the calling convention, move bx to dx, and return.

mov cx, [bp + 10]
xor dx, dx
mov ax, [bp + 8]
div cx
mov bx, ax
mov ax, [bp + 6]
div cx
mov dx, bx
pop bx
mov sp, bp
pop bp
ret 8

auldiv_overflow:
int 3
xor dx, dx
xor ax, ax
mov sp, bp
pop bp
ret 8

__aFuldiv endp


; "Assignment" function that just does the above and puts the result in a
; pointer.  I can't believe the compiler is too stupid to do this itself.
; The compiler is smart enough to know that ds is initialized, so it can
; invoke the "near" version of code in a purely far model program.

public __aFNauldiv
__aFNauldiv proc

push bp
mov bp, sp

; VOID
; __aFNauldiv(
;     DWORD __near * Dividend, [BP + 6],
;     DWORD Divisor [High BP + 10, Low BP + 8]
; );

mov bx, [bp + 6]
push [bp + 10]
push [bp + 8]
push [bx + 2]
push [bx]

call __aFuldiv

mov [bx + 2], dx
mov [bx], ax

mov sp, bp
pop bp
ret 6

__aFNauldiv endp


END
