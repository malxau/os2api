all: dosbind.lib

!INCLUDE "..\common.mk"

OBJS = dosbind.obj      \

dosbind.lib: $(OBJS) dosbind.rsp
    @echo $@
    @erase $@ 2>NUL
	@$(OBJLIB) $(LIBFLAGS) @dosbind.rsp
