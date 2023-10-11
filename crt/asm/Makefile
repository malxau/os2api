all: os2asm.lib

!INCLUDE "..\..\common.mk"

ASMOBJS = afuldiv.obj     \
          anuldiv.obj     \
          afulmul.obj     \
          anulmul.obj     \
          afulrem.obj     \
          anulrem.obj     \
          afulshl.obj     \
          anulshl.obj     \
          afulshr.obj     \
          anulshr.obj     \
          dbgbrk.obj      \

os2asm.lib: $(ASMOBJS) os2asm.rsp
    @echo $@
    @erase $@ 2>NUL
	@$(OBJLIB) $(LIBFLAGS) @os2asm.rsp
