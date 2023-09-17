DEBUG=1
PDB=1

DIRS=lib asm crt dir

!IFDEF _YMAKE_VER
all[dirs target=all]: $(DIRS)

clean[dirs target=clean]: $(DIRS)
!ELSE
all:
    @for %%i in ($(DIRS)) do @cd %%i & $(MAKE) /nologo DEBUG=$(DEBUG) PDB=$(PDB) all & cd ..

clean:
    @for %%i in ($(DIRS)) do @cd %%i & $(MAKE) /nologo DEBUG=$(DEBUG) PDB=$(PDB) clean & cd ..
!ENDIF
