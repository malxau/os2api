all: os2asm.lib

!INCLUDE "..\common.mk"

ASMOBJS = startup0.obj    \
          bigdiv.obj      \
          bigmul.obj      \
          bigrem.obj      \
          bigshl.obj      \
          bigshr.obj      \
          dbgbrk.obj      \
          sel.obj         \

os2asm.lib: $(ASMOBJS) os2asm.rsp
    @echo $@
    @erase $@ 2>NUL
	@$(LIB) $(LIBFLAGS) @os2asm.rsp
