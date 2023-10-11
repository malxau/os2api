DEBUG=1
PDB=1

DIRS=lib os2start\asm os2start crt\asm crt dosfapi\asm dosfapi dosbind dir

!IFDEF _YMAKE_VER
all[dirs target=all]: $(DIRS)

clean[dirs target=clean]: $(DIRS)
!ELSE
all:
    @for %%i in ($(DIRS)) do @cd %%i & $(MAKE) /nologo DEBUG=$(DEBUG) PDB=$(PDB) all & cd $(MAKEDIR)

clean:
    @for %%i in ($(DIRS)) do @cd %%i & $(MAKE) /nologo DEBUG=$(DEBUG) PDB=$(PDB) clean & cd $(MAKEDIR)
!ENDIF
