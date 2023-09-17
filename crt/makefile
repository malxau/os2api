all: os2crt.lib

!INCLUDE "..\common.mk"

COBJS = halloc.obj      \
        mem.obj         \
        printf.obj      \
        salloc.obj      \
        startup1.obj    \
        string.obj      \

os2crt.lib: $(COBJS) ..\asm\os2asm.lib os2crt.rsp
    @echo $@
    @erase $@ 2>NUL
	@$(LIB) $(LIBFLAGS) @os2crt.rsp
