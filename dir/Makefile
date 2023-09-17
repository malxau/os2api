all: zdir.exe

!INCLUDE "..\common.mk"

zdiros2.exe: zdir.obj $(DEPS)
    @echo $@
    @$(LINK) $(LDFLAGS) zdir.obj,zdiros2.exe,zdiros2.map,..\crt\os2crt.lib ..\lib\os2.lib,zdir.def

!IF "$(BIND)"!=""
zdir.exe: zdiros2.exe
    @echo $@
    @$(BIND) /nologo zdiros2.exe /m zdir.map /o zdir.exe
!ELSE
zdir.exe: zdiros2.exe
    @echo $@ (copy binary)
    @copy zdiros2.exe $@
!ENDIF
