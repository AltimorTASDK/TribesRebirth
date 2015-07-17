##############################################################################

INCdir = inc;glide;$(PhoenixInc)

PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#%set PRECOMPILED_HEADER=

.PATH.exe = .

%ifdef CG32
CPPFLAGS += -vG
%endif

%if $(COMPILER) == "m"
CPPFLAGS += /DNODLLAPI
%endif

CPPFLAGS += /DM_NOFILEIO

##############################################################################
# The library gets the runtime modules

GFX_DATA_OBJS =  g_bitmap.obj       \
                 g_barray.obj       \
                 g_font.obj         \
                 g_bm.obj           \
                 \
                 g_timer.obj        \
                 g_pal.obj          \
                 gfxRes.obj         \
                 g_clip.obj         \
                 palMap.obj

#
GFX_OBJS =      \
                g_surfac.obj        \
                gdmanag.obj         \
                gdevice.obj         \
                fn_table.obj        \
                \
                gglidedev.obj       \
                glidesfc.obj        \
                glidetx.obj         \
                glidein.obj         \
                \
                gOGLDev.obj         \
                gOGLSfc.obj         \
                gOGLTx.obj          \
                gOGLFn.obj          \
                gOGLTCache.obj      \
                glLoader.obj        \
                \
                gsdevice.obj        \
                g_surfac.obj        \
                g_sub.obj           \
                g_mem.obj           \
                g_cds.obj           \
                g_ddraw.obj         \
                p_scane.obj         \
                p_txrc.obj          \
                g_fontrend.obj      \
                g_point.obj         \
                g_clip.obj          \
                g_clear.obj         \
                g_cleara.obj        \
                b_blit.obj          \
                b_draw.obj          \
                g_contxt.obj        \
                \
                r_draw.obj          \
                rn_fill.obj         \
                r_raster.obj        \
                writfunc.obj        \
                \
                rn_filln.obj        \
                rn_nontx.obj        \
                rn_nptex.obj        \
                rn_persp.obj        \
                p_rastn.obj         \
                p_rastdc.obj        \
                p_edge.obj          \
                p_funcs.obj         \
                gbase.obj           \
                p_txcach.obj        \
                \
                l_draw.obj          \
                l_raster.obj        \
                \
                ddrawerr.obj        \
                g_timer.obj         \
                m_random.obj        \



all: $(LIBdest)\$(DEBUG)$(COMPILER)gfx.lib      \
     $(LIBdest)\$(DEBUG)$(COMPILER)gfxio.lib    \
%ifdef DYNAMIC
     $(LIBdest)\$(DEBUG)$(COMPILER)dgfx.dll     \
%endif

$(LIBdest)\$(DEBUG)$(COMPILER)gfxio.lib:  \
   %ifdef CG32
   $(BORLAND)\lib\cg32.lib                \
   %endif
   $(GFX_DATA_OBJS)

$(LIBdest)\$(DEBUG)$(COMPILER)dgfx.dll:   \
   %ifdef CG32
   $(BORLAND)\lib\cg32.lib                \
   %endif
   $(DEBUG)$(COMPILER)core.lib    \
   $(LIBdest)\$(DEBUG)$(COMPILER)gfxio.lib    \
   $(GFX_OBJS)

$(LIBdest)\$(DEBUG)$(COMPILER)gfx.lib: $(GFX_OBJS)
   %if $(COMPILER) == "b"
   %set LIBFLAGS = /C /P256
   %endif
   %do %.lib

#
copy :
   %echo copying DGfx files...
   @xcopy /D inc\*.h       $(PHOENIXINC) /R /Q  > NUL
   @xcopy /D glide\*.h     $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.dll     $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code $(PHOENIXCODE) /R /Q  > NUL
   %endif
   @--xcopy /D lib\*.lib     $(PHOENIXLIB) /R /Q  > NUL
   %if $(COMPILER) == "b"
      @--xcopy /D lib\*.tds     $(PHOENIXLIB) /R /Q  > NUL
   %endif
# - Copy dll to project directory if necessary
   %if %defined(PROJECTEXE)
      @xcopy /D lib\*.dll $(PROJECTEXE) /R /Q  > NUL
   %endif
# - Copy rendition uCode file to data directory...
#   %if %defined(PHOENIXDATA)
#      @xcopy /D Rendition\*.uc $(PHOENIXDATA) /R /Q  > NUL
#   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif

