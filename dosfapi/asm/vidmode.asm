;
; VIDMODE.ASM
;
; Get video mode.
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

public VioGetMode
VioGetMode proc

push bp
mov bp, sp

; SYSERR APIENTRY
; VioGetMode(
;     PVIOMODEINFO pMode,    [High BP + 10, Low BP + 8]
;     HVIO hvio              [BP + 6]
; );

push ds
push bx
push di
mov ds, [bp + 10]
mov di, [bp + 8]

xor ax, ax
mov word ptr [di + 2], ax
mov word ptr [di + 4], ax
mov word ptr [di + 6], ax
mov word ptr [di + 8], ax
mov word ptr [di + 10], ax

; Ask for the EGA font, which can return the highest line number on the
; display.  Zero dx before the call and if it's nonzero afterwards, use
; that.
push es
push bp
mov ax, 01130h
mov bh, 03h
int 10h
pop bp
pop es
test al, al
jz VioGetModeGetVideoMode

mov dh, 0
inc dl
mov word ptr [di + 6], dx

; Query the display mode.  This returns the number of columns.  It also
; returns the display mode which can be used to infer the line count if
; we didn't get a better answer earlier.
VioGetModeGetVideoMode:
mov ah, 0fh
int 10h

mov bl, ah
mov bh, 0

mov word ptr [di + 4], bx
test dx, dx
jnz VioGetModeDone

mov word ptr [di + 6], 25
cmp al, 17
je VioGetMode30Lines

cmp al, 23
je VioGetMode43Lines
cmp al, 25
je VioGetMode43Lines
cmp al, 29
je VioGetMode43Lines
cmp al, 66
je VioGetMode43Lines
cmp al, 84
je VioGetMode43Lines
cmp al, 86
je VioGetMode43Lines
cmp al, 88
je VioGetMode43Lines

cmp al, 81
je VioGetMode50Lines
cmp al, 97
je VioGetMode50Lines
cmp al, 102
je VioGetMode50Lines
cmp al, 105
je VioGetMode50Lines

cmp al, 30
je VioGetMode60Lines
cmp al, 33
je VioGetMode60Lines
cmp al, 38
je VioGetMode60Lines
cmp al, 67
je VioGetMode60Lines
cmp al, 82
je VioGetMode60Lines
cmp al, 100
je VioGetMode60Lines

jmp VioGetModeDone

VioGetMode30Lines:
mov word ptr [di + 6], 30
jmp VioGetModeDone

VioGetMode43Lines:
mov word ptr [di + 6], 43
jmp VioGetModeDone

VioGetMode50Lines:
mov word ptr [di + 6], 50
jmp VioGetModeDone

VioGetMode60Lines:
mov word ptr [di + 6], 60
jmp VioGetModeDone


VioGetModeDone:

pop di
pop bx
pop ds

xor ax, ax

mov sp, bp
pop bp

ret 6

VioGetMode endp

END
