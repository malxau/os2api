all: os2start.lib

!INCLUDE "..\..\common.mk"

ASMOBJS = startup0.obj    \

os2start.lib: $(ASMOBJS) os2start.rsp
    @echo $@
    @erase $@ 2>NUL
	@$(OBJLIB) $(LIBFLAGS) @os2start.rsp
