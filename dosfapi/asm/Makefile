all: dosasm.lib

!INCLUDE "..\..\common.mk"

ASMOBJS = cursor.obj      \
          dosexit.obj     \
          dosfind.obj     \
          doswrite.obj    \
          keybrd.obj      \
          qfsinfo.obj     \
          segalloc.obj    \
          scrollup.obj    \
          startup0.obj    \
          vidmode.obj     \
          wrtchrat.obj    \

dosasm.lib: $(ASMOBJS) dosasm.rsp
	@echo $@
	@erase $@ 2>NUL
	@$(OBJLIB) $(LIBFLAGS) @dosasm.rsp
