;
; STARTUP0.ASM
;
; Initial program entry point.  In OS/2 it looks like this has to be
; assembly because core state is initialized in registers.  This tried to
; save that and get to C as quickly as possible.
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

.MODEL large, pascal, FARSTACK, OS_OS2

__startup PROTO FAR PASCAL

.DATA
public __acrtused
__acrtused = 1234h
_EnvSelector    WORD ?
_CmdlineOffset  WORD ?
_DataSegSize    WORD ?

.STACK 6144

.CODE
.STARTUP
; OS/2 Arguments
; AX Selector of environment
; BX Command line offset within environment selector
; CX Size of data segment

mov [_EnvSelector], AX
mov [_CmdlineOffset], BX
mov [_DataSegSize], CX

call __startup

; C code is responsible for exiting the process - execution should
; not get here

public GetCommandLine
GetCommandLine proc

mov dx, [_EnvSelector]
mov ax, [_CmdlineOffset]

ret

GetCommandLine endp

END
