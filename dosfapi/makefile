all: dosfapi.lib

!INCLUDE "..\common.mk"

COBJS = dosfapi.obj     \

dosfapi.lib: $(COBJS) asm\dosasm.lib dosfapi.rsp
    @echo $@
    @erase $@ 2>NUL
	@$(OBJLIB) $(LIBFLAGS) @dosfapi.rsp
