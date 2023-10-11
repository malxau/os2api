;
; QFSINFO.ASM
;
; Get file system information.
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

public DosQFSInfo
DosQFSInfo proc

push bp
mov bp, sp

; SYSERR APIENTRY
; DosQFSInfo(
;     WORD wDriveNumber,     [BP + 14]
;     WORD wInfoLevel,       [BP + 12]
;     PVOID pInfoBuf,        [High BP + 10, Low BP + 8]
;     WORD wInfoBufLength    [BP + 6]
; );

push ds
push bx
push di

mov dl, [bp + 14]
mov ah, 036h
int 21h

mov ds, [bp + 10]
mov di, [bp + 8]

; MSFIX This is a DOS 2.0 API that doesn't look sufficient for today's
; disks...?

mov word ptr [di + 4], ax     ; Sectors per allocation unit (low)
mov word ptr [di + 8], dx     ; Total allocation units (low)
mov word ptr [di + 12], bx    ; Available allocation units (low)
mov word ptr [di + 16], cx    ; Bytes per sector

xor ax, ax

mov word ptr [di], ax         ; File system ID
mov word ptr [di + 2], ax     ; File system ID
mov word ptr [di + 6], ax     ; Sectors per allocation unit (high)
mov word ptr [di + 10], ax    ; Total allocation units (high)
mov word ptr [di + 14], ax    ; Available allocation units (high)

pop di
pop bx
pop ds

mov sp, bp
pop bp

ret 10

DosQFSInfo endp

END
