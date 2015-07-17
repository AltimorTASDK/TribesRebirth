PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%set PRECOMPILED_HEADER=

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG
    %endif
%endif

TS3_OBJS =  ts_fast.obj          \
            ts_BoundingBox.obj   \
            ts_camera.obj        \
            ts_CelAnimMesh.obj   \
            ts_light.obj         \
            ts_material.obj      \
            ts_matRender.obj     \
            ts_PointArray.obj    \
            ts_RenderContext.obj \
            ts_shadow.obj        \
            ts_shape.obj         \
            ts_shapeInst.obj     \
            ts_partInstance.obj  \
            ts_template.obj      \
            ts_transform.obj     \
            ts_plane.obj         \
            ts_color.obj         \
            ts_vertex.obj

$(LIBdest)\$(BuildPrefix)ts3.lib:   \
	$(TS3_OBJS) 

copy :
   %echo copying TS3 files...
   @xcopy /D inc\*.h   $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\* $(PHOENIXCODE) /R /Q  > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
