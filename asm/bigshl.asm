;
; BIGSHL.ASM
;
; Implementation for for signed and unsigned left shift of a 32 bit integer.
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

; DWORD [High DX, Low AX]
; __aFulshl(
;     DWORD Value, [High DX, Low AX]
;     WORD Shift [CL]
; );

public __aFulshl
__aFulshl proc

; If the shift is for more than 32 bits, all the data would be gone, so
; return zero.  If the shift is for more than 16 bits, the low 16 bits
; of the result must be zero, and the high 16 bits contains the low 16
; bits of input after shifting.  If the shift is less than 16 bits, then
; both components must be shifted with bits carried between the two.

cmp cl,32
jae ulshl_no_shift
cmp cl,16
jae ulshl_long_shift

push bx
mov bx, ax
shl ax, cl
shl dx, cl
sub cl, 16
neg cl
shr bx, cl
or dx, bx
pop bx
ret

ulshl_long_shift:
sub cl, 16
shl ax, cl
mov dx, ax
xor ax, ax
ret

ulshl_no_shift:
xor ax, ax
xor dx, dx
ret

__aFulshl endp

; DWORD [High DX, Low AX]
; _aFlshl(
;     DWORD Value, [High DX, Low AX]
;     WORD Shift [CX]
; );

; It's not clear to me what the meaning of a signed bitshift is, but for now
; give it to the unsigned implementation.

public __aFlshl
__aFlshl proc
jmp __aFulshl
__aFlshl endp

; "Assignment" function that just does the above and puts the result in a
; pointer.  I can't believe the compiler is too stupid to do this itself.
; The compiler is smart enough to know that ds is initialized, so it can
; invoke the "near" version of code in a purely far model program.

public __aFNaulshl
__aFNaulshl proc

push bp
mov bp, sp

; VOID
; __aFNaulshl(
;     DWORD __near * Value, [BP + 6],
;     WORD ShiftCount [BP + 8]
; );

push bx
mov bx, [bp + 6]
mov ax, [bx]
mov dx, [bx + 2]
mov cx, [bp + 8]

call __aFulshl

mov [bx + 2], dx
mov [bx], ax

pop bx
mov sp, bp
pop bp
ret 4

__aFNaulshl endp


END
