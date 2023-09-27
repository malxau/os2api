;
; SEL.ASM
;
; Macro to convert selector to far pointer.  This must be a common macro
; somewhere, but I can't find it, so...
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

.MODEL MemModel, pascal

.CODE


public SelToFarPtr
SelToFarPtr proc

push bp
mov bp, sp

; PVOID [High DX, Low AX]
; SelToFarPtr(
;     WORD Selector [Low BP + 6]
; );

mov dx, [bp + 6];
xor ax, ax

mov sp, bp
pop bp
ret 2

SelToFarPtr endp


public FarPtrToSel
FarPtrToSel proc

push bp
mov bp, sp

; WORD [AX]
; FarPtrToSel(
;     PVOID Ptr [High BP + 8, Low BP + 6]
; );

mov ax, [bp + 8];

mov sp, bp
pop bp
ret 4

FarPtrToSel endp


END
