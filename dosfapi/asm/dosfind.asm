;
; DOSFIND.ASM
;
; DosFindFirst, DosFindNext and friends
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

.DATA?
DosFindBuffer BYTE 0540h DUP (?)

.CODE _TEXT

public DosCopyDta
DosCopyDta proc

; VOID APIENTRY
; DosCopyDta(
;     PFILEFINDBUF pFileFindBuf, [High ES, Low AX]
;     PVOID pDta             [High DS, Low BX]
; );

push si
push di

mov di, ax
mov si, bx

mov ax, [si + 016h]
mov word ptr es:[di], ax                       ; Creation date
mov word ptr es:[di + 4], ax                   ; Write date
mov word ptr es:[di + 8], ax                   ; Access date
mov ax, [si + 018h]
mov word ptr es:[di + 2], ax                   ; Creation time
mov word ptr es:[di + 6], ax                   ; Write time
mov word ptr es:[di + 10], ax                  ; Access time
mov ax, [si + 01ah]
mov word ptr es:[di + 12], ax                  ; File size (low)
mov word ptr es:[di + 16], ax                  ; Allocation size (low)
mov ax, [si + 01ch]
mov word ptr es:[di + 14], ax                  ; File size (high)
mov word ptr es:[di + 18], ax                  ; Allocation size (high)
mov al, [si + 015h]
mov byte ptr es:[di + 20], al                  ; Attributes (low)
mov byte ptr es:[di + 21], 0                   ; Attributes (high)

mov ax, di
add di, 23
add si, 30
DosCopyDtaNameNextChar:
mov cl, [si]
mov es:[di], cl
inc si
inc di
test cl, cl
jnz DosCopyDtaNameNextChar

mov si, ax  ; si is now the original di
mov ax, di  ; find difference between current di and original
sub ax, si
sub ax, 24  ; ax now has file name bytes copied excluding NULL

mov di, si
mov byte ptr es:[di + 22], al

pop di
pop si

ret

DosCopyDta endp

public DosCopyFindData
DosCopyFindData proc

; VOID APIENTRY
; DosCopyFindData(
;     PFILEFINDBUF pFileFindBuf, [High ES, Low AX]
;     PVOID pFindData        [High DS, Low BX]
; );

push si
push di

mov di, ax
mov si, bx

mov ax, [si + 04h]
mov word ptr es:[di + 2], ax                   ; Creation time
mov ax, [si + 06h]
mov word ptr es:[di], ax                       ; Creation date
mov ax, [si + 0ch]
mov word ptr es:[di + 10], ax                  ; Access time
mov ax, [si + 0eh]
mov word ptr es:[di + 8], ax                   ; Access date
mov ax, [si + 014h]
mov word ptr es:[di + 6], ax                   ; Write time
mov ax, [si + 016h]
mov word ptr es:[di + 4], ax                   ; Write date
mov ax, [si + 020h]
mov word ptr es:[di + 12], ax                  ; File size (low)
mov word ptr es:[di + 16], ax                  ; Allocation size (low)
mov ax, [si + 022h]
mov word ptr es:[di + 14], ax                  ; File size (high)
mov word ptr es:[di + 18], ax                  ; Allocation size (high)
mov ax, [si]
mov word ptr es:[di + 20], ax                  ; Attributes

mov ax, di
add di, 23
add si, 44
DosCopyFindDataNameNextChar:
mov cl, [si]
mov es:[di], cl
inc si
inc di
test cl, cl
jnz DosCopyFindDataNameNextChar

mov si, ax  ; si is now the original di
mov ax, di  ; find difference between current di and original
sub ax, si
sub ax, 24  ; ax now has file name bytes copied excluding NULL

mov di, si
mov byte ptr es:[di + 22], al

pop di
pop si

ret

DosCopyFindData endp

public DosFindFirst
DosFindFirst proc

push bp
mov bp, sp

; SYSERR APIENTRY
; DosFindFirst(
;     PSZ pszFileSpec,       [High BP + 28, Low BP + 26]
;     PHDIR pDirHandle,      [High BP + 24, Low BP + 22]
;     WORD wAttr,            [BP + 20]
;     PFILEFINDBUF pFileFindBuf, [High BP + 18, Low BP + 16]
;     WORD wBufSize,         [BP + 14]
;     PWORD pwSearch,        [High BP + 12, Low BP + 10]
;     DWORD dwReserved       [High BP + 8, Low BP + 6]
; );

push es
push di
push si
push bx
; Intentionally last so it can be popped during function
push ds

; Check pDirHandle is the only valid value
mov ds, [bp + 24]
mov di, [bp + 22]
mov ax, [di]
cmp ax, 1
je DosFindFirstIssueCall

mov ax, 6
jmp DosFindFirstError

DosFindFirstIssueCall:

; While the dir handle pointer is loaded, set this to -1 to indicate an
; 8.3 name call was used
mov ax, 0ffffh
mov [di], ax

mov ax, 0714eh
mov cx, [bp + 20]
mov ch, 0
mov si, 1
mov ds, [bp + 28]
mov dx, [bp + 26]

; Load the ds register into the es register.  This is the output of the
; call.  The search string is in the ds register.
pop es
push es
mov di, OFFSET DosFindBuffer
int 21h

; On error, try the 8.3 version
jb DosFindFirstIssueClassicCall

; Save find handle
mov ds, [bp + 24]
mov di, [bp + 22]
mov [di], ax

; Switch the output (es) segment to the data segment
push es
pop ds

mov es, [bp + 18]
mov ax, [bp + 16]
mov bx, OFFSET DosFindBuffer

call DosCopyFindData

xor ax, ax
jmp DosFindFirstError

DosFindFirstIssueClassicCall:

mov ds, [bp + 28]
mov dx, [bp + 26]
mov cx, [bp + 20]
mov ah, 04eh
int 21h

jb DosFindFirstError

mov ah, 02fh
int 21h

push es
pop ds

mov es, [bp + 18]
mov ax, [bp + 16]
; MSFIX - Need to count length of file name to check buffer size

call DosCopyDta

xor ax, ax

DosFindFirstError:

pop ds
pop bx
pop si
pop di
pop es
mov sp, bp
pop bp

ret 24

DosFindFirst endp

public DosFindNext
DosFindNext proc

push bp
mov bp, sp

; SYSERR APIENTRY
; DosFindNext(
;     HDIR hDirHandle,       [BP + 16]
;     PFILEFINDBUF pFileFindBuf, [High BP + 14, Low BP + 12]
;     WORD wBufSize,         [BP + 10]
;     PWORD pwSearch         [High BP + 8, Low BP + 6]
; );

push es
push bx
push si
; Intentionally last so it can be popped during function
push ds

mov ax, [bp + 16]
cmp ax, 0ffffh
je DosFindNextIssueClassicCall

mov bx, ax
mov ax, 0714fh
mov si, 1
; Load the ds register into the es register.  This is the output of the
; call.
pop es
push es
mov di, OFFSET DosFindBuffer

int 21h

jb DosFindNextError

; Switch the output (es) segment to the data segment
push es
pop ds

mov es, [bp + 14]
mov ax, [bp + 12]
mov bx, di

call DosCopyFindData

jmp DosFindNextSuccess

DosFindNextIssueClassicCall:
mov ah, 04fh
int 21h

jb DosFindNextError

mov ah, 02fh
int 21h

push es
pop ds

mov es, [bp + 14]
mov ax, [bp + 12]

call DosCopyDta

DosFindNextSuccess:

xor ax, ax

DosFindNextError:

pop ds
pop si
pop bx
pop es
mov sp, bp
pop bp

ret 12

DosFindNext endp

public DosFindClose
DosFindClose proc

push bp
mov bp, sp

; SYSERR APIENTRY
; DosFindClose(
;     HDIR hDirHandle        [BP + 6]
; );

push bx

int 3h

mov ax, [bp + 6]
cmp ax, 0ffffh
je DosFindCloseSuccess

mov bx, ax
mov ax, 071a1h
int 21h

jb DosFindCloseError

DosFindCloseSuccess:
xor ax, ax

DosFindCloseError:

pop bx

mov sp, bp
pop bp

ret 2

DosFindClose endp

END
