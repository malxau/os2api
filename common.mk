
!IFNDEF DEBUG
DEBUG=0
!ENDIF

!IFNDEF PDB
PDB=1
!ENDIF

ML=ml.exe
CC=cl.exe
OBJLIB=lib.exe
LINK=link.exe
IMPLIB=implib.exe
BIND=bind.exe

# Check if MASM is installed.  If not, we'll use preassembled binaries
# for these parts.
!IF [$(ML) 2>&1 | find "Microsoft" >NUL]>0
ML=
!ENDIF

# Check if BIND is installed.  If not, we can't compile bound programs,
# only pure OS/2.
# for these parts.
!IF [$(BIND) 2>&1 | find "Microsoft" >NUL]>0
BIND=
!ENDIF

CFLAGS=-nologo -AC -Gs -Gc -Gf -Gq -Gy -W3 -WX -I..\inc -I..\crt
MLFLAGS=-nologo -Zi -Zd -DMemModel=compact
LIBFLAGS=/nologo
IMPLIBFLAGS=/nologo
LDFLAGS=/nologo /nodefaultlibrarysearch /map:f

!IF $(DEBUG)==1
CFLAGS=$(CFLAGS) -Od
!ELSE
CFLAGS=$(CFLAGS) -Os /f-
!ENDIF

!IF $(PDB)==1
CFLAGS=$(CFLAGS) -Z7
LDFLAGS=$(LDFLAGS) /codeview
!ENDIF

OS2LIBS=..\os2start\os2start.lib ..\crt\os2crt.lib ..\lib\os2.lib
DOSLIBS=..\dosfapi\dosfapi.lib ..\crt\os2crt.lib
BINDLIBS=..\dosbind\dosbind.lib

clean:
	-@erase *.exe    2>NUL
	-@erase *.map    2>NUL
	-@erase *.obj    2>NUL
	-@erase *.lib    2>NUL

.c.obj:
	@$(CC) /c $(CFLAGS) $<

!IF "$(ML)"!=""
.asm.obj:
	@$(ML) /c $(MLFLAGS) $<
!ELSE
.asm.obj:
	@echo bin\$@ (copy binary)
	@copy bin\$@ .
!ENDIF
