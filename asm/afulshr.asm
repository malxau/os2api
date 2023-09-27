;
; BIGSHR.ASM
;
; Implementation for for signed and unsigned right shift of a 32 bit integer.
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
.CODE

; DWORD [High DX, Low AX]
; __aFulshr(
;     DWORD Value, [High DX, Low AX]
;     WORD Shift [CL]
; );

public __aFulshr
__aFulshr proc

; If the shift is for more than 32 bits, all the data would be gone, so
; return zero.  If the shift is for more than 16 bits, the high 16 bits
; of the result must be zero, and the low 16 bits contains the high 16
; bits of input after shifting.  If the shift is less than 16 bits, then
; both components must be shifted with bits carried between the two.

cmp cl,32
jae ulshr_no_shift
cmp cl,16
jae ulshr_long_shift

push bx
mov bx, dx
shr ax, cl
shr dx, cl
sub cl, 16
neg cl
shl bx, cl
or ax, bx
pop bx
ret

ulshr_long_shift:
sub cl, 16
shr dx, cl
mov ax, dx
xor dx, dx
ret

ulshr_no_shift:
xor ax, ax
xor dx, dx
ret

__aFulshr endp

; DWORD [High DX, Low AX]
; _aFlshr(
;     DWORD Value, [High DX, Low AX]
;     WORD Shift [CL]
; );

; It's not clear to me what the meaning of a signed bitshift is, but for now
; give it to the unsigned implementation.

public __aFlshr
__aFlshr proc
jmp __aFulshr
__aFlshr endp


; "Assignment" function that just does the above and puts the result in a
; pointer.  I can't believe the compiler is too stupid to do this itself.
; The compiler is smart enough to know that ds is initialized, so it can
; invoke the "near" version of code in a purely far model program.

public __aFNaulshr
__aFNaulshr proc

push bp
mov bp, sp

; VOID
; __aFNaulshr(
;     DWORD __near * Value, [BP + 6],
;     WORD ShiftCount [BP + 8]
; );

push bx
mov bx, [bp + 6]
mov ax, [bx]
mov dx, [bx + 2]
mov cx, [bp + 8]

call __aFulshr

mov [bx + 2], dx
mov [bx], ax

pop bx
mov sp, bp
pop bp
ret 4

__aFNaulshr endp

END
