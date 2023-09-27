;
; BIGREM.ASM
;
; Implementation for for signed and unsigned division of a 32 bit integer
; by a 16 bit integer, returning the remainder (ie., mod.)
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

.MODEL small, pascal

.CODE


public __aNulrem
__aNulrem proc

push bp
mov bp, sp

; DWORD [High DX, Low AX]
; __aNulrem(
;     DWORD Dividend, [High BP + 6, Low BP + 4],
;     DWORD Divisor [High BP + 10, Low BP + 8]
; );

; This implementation doesn't support 32 bit divisors.  If one is specified,
; fail.

mov ax, [bp + 10]
test ax, ax
jnz aulrem_overflow

; Divide the high 16 bits by the low 16 bits, and leave the
; remainder in dx.  Then, divide the low 16 bits plus the
; remainder, which must fit in a 16 bit value.  To satisfy
; the calling convention, move dx to ax, clear dx, and return.

mov cx, [bp + 8]
xor dx, dx
mov ax, [bp + 6]
div cx
mov ax, [bp + 4]
div cx
mov ax, dx
xor dx, dx
mov sp, bp
pop bp
ret 8

aulrem_overflow:
int 3
xor dx, dx
xor ax, ax
mov sp, bp
pop bp
ret 8

__aNulrem endp


; "Assignment" function that just does the above and puts the result in a
; pointer.  I can't believe the compiler is too stupid to do this itself.
; The compiler is smart enough to know that ds is initialized, so it can
; invoke the "near" version of code in a purely far model program.

public __aNNaulrem
__aNNaulrem proc

push bp
mov bp, sp

; VOID
; __aNNaulrem(
;     DWORD __near * Dividend, [BP + 4],
;     DWORD Divisor [High BP + 8, Low BP + 6]
; );

mov bx, [bp + 4]
push [bp + 8]
push [bp + 6]
push [bx + 2]
push [bx]

call __aNulrem

mov [bx + 2], dx
mov [bx], ax

mov sp, bp
pop bp
ret 6

__aNNaulrem endp


END
