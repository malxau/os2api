all: os2.lib

!INCLUDE "..\common.mk"

INPUTLIBS = doscalls.lib kbdcalls.lib nls.lib viocalls.lib

.SUFFIXES: .def .lib

os2.lib: $(INPUTLIBS) os2.rsp
    @echo $@
    @erase $@ 2>NUL
    $(OBJLIB) $(LIBFLAGS) @os2.rsp

.def.lib:
    $(IMPLIB) $(IMPLIBFLAGS) /nologo $@ $<
