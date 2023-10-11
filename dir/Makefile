all: bzdir.exe dzdir.exe ozdir.exe szdir.exe

!INCLUDE "..\common.mk"

ozdir.exe: zdir.obj $(OS2LIBS)
	@echo $@
	@$(LINK) $(LDFLAGS) zdir.obj,ozdir.exe,ozdir.map,$(OS2LIBS) ..\lib\os2.lib,zdir.def

dzdir.exe: zdir.obj $(DOSLIBS)
	@echo $@
	@$(LINK) $(LDFLAGS) /cp:1 zdir.obj,dzdir.exe,dzdir.map,$(DOSLIBS) ..\crt\os2crt.lib $(BINDLIBS),NUL

szdir.exe: zdir.obj $(OS2LIBS) dzdir.exe
	@echo $@
	@$(LINK) $(LDFLAGS) zdir.obj,szdir.exe,szdir.map,$(OS2LIBS) ..\lib\os2.lib,szdir.def

!IF "$(BIND)"!=""
bzdir.exe: ozdir.exe
	@echo $@
	@$(BIND) /nologo ozdir.exe $(BINDLIBS) /m bzdir.map /o bzdir.exe
!ELSE
bzdir.exe: szdir.exe
	@echo $@ (copy binary)
	@copy szdir.exe $@
!ENDIF
