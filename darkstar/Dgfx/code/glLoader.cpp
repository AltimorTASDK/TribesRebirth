// _________________________________________________________________________
//
// Win32 module to redirect calls from OPENGL32.DLL to specified provider DLL, 
// with API function trace to OutputDebugString() or text file
//
// Released into public domain 9-Jul-97
//
// Please forward enhancements and bug reports to jmiles@digitalanvil.com
//
// _________________________________________________________________________
//
// Appropriated for use in the Darkstar Core by Dave Moore, 1/25/99.  Gutted
//  and cleaned, all this is good for now is loading and redirecting calls
//  to opengl32.dll.  Forward any rage to dave.moore@dynamix.com
//
// _________________________________________________________________________

#include <windows.h>
#include "base.h"

#pragma warning (disable:4273)   // No complaints about DLL linkage...
#pragma pack(1)                  // Disable structure rearrangement

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef GLubyte* GLstring;

//
// Names of OpenGL functions
//
static char *GLN[] = 
{
   "glAccum",
   "glAlphaFunc",
   "glAreTexturesResident",
   "glArrayElement",
   "glBegin",
   "glBindTexture",
   "glBitmap",
   "glBlendFunc",
   "glCallList",
   "glCallLists",
   "glClear",
   "glClearAccum",
   "glClearColor",
   "glClearDepth",
   "glClearIndex",
   "glClearStencil",
   "glClipPlane",
   "glColor3b",
   "glColor3bv",
   "glColor3d",
   "glColor3dv",
   "glColor3f",
   "glColor3fv",
   "glColor3i",
   "glColor3iv",
   "glColor3s",
   "glColor3sv",
   "glColor3ub",
   "glColor3ubv",
   "glColor3ui",
   "glColor3uiv",
   "glColor3us",
   "glColor3usv",
   "glColor4b",
   "glColor4bv",
   "glColor4d",
   "glColor4dv",
   "glColor4f",
   "glColor4fv",
   "glColor4i",
   "glColor4iv",
   "glColor4s",
   "glColor4sv",
   "glColor4ub",
   "glColor4ubv",
   "glColor4ui",
   "glColor4uiv",
   "glColor4us",
   "glColor4usv",
   "glColorMask",
   "glColorMaterial",
   "glColorPointer",
   "glCopyPixels",
   "glCopyTexImage1D",
   "glCopyTexImage2D",
   "glCopyTexSubImage1D",
   "glCopyTexSubImage2D",
   "glCullFace",
   "glDeleteLists",
   "glDeleteTextures",
   "glDepthFunc",
   "glDepthMask",
   "glDepthRange",
   "glDisable",
   "glDisableClientState",
   "glDrawArrays",
   "glDrawBuffer",
   "glDrawElements",
   "glDrawPixels",
   "glEdgeFlag",
   "glEdgeFlagPointer",
   "glEdgeFlagv",
   "glEnable",
   "glEnableClientState",
   "glEnd",
   "glEndList",
   "glEvalCoord1d",
   "glEvalCoord1dv",
   "glEvalCoord1f",
   "glEvalCoord1fv",
   "glEvalCoord2d",
   "glEvalCoord2dv",
   "glEvalCoord2f",
   "glEvalCoord2fv",
   "glEvalMesh1",
   "glEvalMesh2",
   "glEvalPoint1",
   "glEvalPoint2",
   "glFeedbackBuffer",
   "glFinish",
   "glFlush",
   "glFogf",
   "glFogfv",
   "glFogi",
   "glFogiv",
   "glFrontFace",
   "glFrustum",
   "glGenLists",
   "glGenTextures",
   "glGetBooleanv",
   "glGetClipPlane",
   "glGetDoublev",
   "glGetError",
   "glGetFloatv",
   "glGetIntegerv",
   "glGetLightfv",
   "glGetLightiv",
   "glGetMapdv",
   "glGetMapfv",
   "glGetMapiv",
   "glGetMaterialfv",
   "glGetMaterialiv",
   "glGetPixelMapfv",
   "glGetPixelMapuiv",
   "glGetPixelMapusv",
   "glGetPointerv",
   "glGetPolygonStipple",
   "glGetString",
   "glGetTexEnvfv",
   "glGetTexEnviv",
   "glGetTexGendv",
   "glGetTexGenfv",
   "glGetTexGeniv",
   "glGetTexImage",
   "glGetTexLevelParameterfv",
   "glGetTexLevelParameteriv",
   "glGetTexParameterfv",
   "glGetTexParameteriv",
   "glHint",
   "glIndexMask",
   "glIndexPointer",
   "glIndexd",
   "glIndexdv",
   "glIndexf",
   "glIndexfv",
   "glIndexi",
   "glIndexiv",
   "glIndexs",
   "glIndexsv",
   "glIndexub",
   "glIndexubv",
   "glInitNames",
   "glInterleavedArrays",
   "glIsEnabled",
   "glIsList",
   "glIsTexture",
   "glLightModelf",
   "glLightModelfv",
   "glLightModeli",
   "glLightModeliv",
   "glLightf",
   "glLightfv",
   "glLighti",
   "glLightiv",
   "glLineStipple",
   "glLineWidth",
   "glListBase",
   "glLoadIdentity",
   "glLoadMatrixd",
   "glLoadMatrixf",
   "glLoadName",
   "glLogicOp",
   "glMap1d",
   "glMap1f",
   "glMap2d",
   "glMap2f",
   "glMapGrid1d",
   "glMapGrid1f",
   "glMapGrid2d",
   "glMapGrid2f",
   "glMaterialf",
   "glMaterialfv",
   "glMateriali",
   "glMaterialiv",
   "glMatrixMode",
   "glMultMatrixd",
   "glMultMatrixf",
   "glNewList",
   "glNormal3b",
   "glNormal3bv",
   "glNormal3d",
   "glNormal3dv",
   "glNormal3f",
   "glNormal3fv",
   "glNormal3i",
   "glNormal3iv",
   "glNormal3s",
   "glNormal3sv",
   "glNormalPointer",
   "glOrtho",
   "glPassThrough",
   "glPixelMapfv",
   "glPixelMapuiv",
   "glPixelMapusv",
   "glPixelStoref",
   "glPixelStorei",
   "glPixelTransferf",
   "glPixelTransferi",
   "glPixelZoom",
   "glPointSize",
   "glPolygonMode",
   "glPolygonOffset",
   "glPolygonStipple",
   "glPopAttrib",
   "glPopClientAttrib",
   "glPopMatrix",
   "glPopName",
   "glPrioritizeTextures",
   "glPushAttrib",
   "glPushClientAttrib",
   "glPushMatrix",
   "glPushName",
   "glRasterPos2d",
   "glRasterPos2dv",
   "glRasterPos2f",
   "glRasterPos2fv",
   "glRasterPos2i",
   "glRasterPos2iv",
   "glRasterPos2s",
   "glRasterPos2sv",
   "glRasterPos3d",
   "glRasterPos3dv",
   "glRasterPos3f",
   "glRasterPos3fv",
   "glRasterPos3i",
   "glRasterPos3iv",
   "glRasterPos3s",
   "glRasterPos3sv",
   "glRasterPos4d",
   "glRasterPos4dv",
   "glRasterPos4f",
   "glRasterPos4fv",
   "glRasterPos4i",
   "glRasterPos4iv",
   "glRasterPos4s",
   "glRasterPos4sv",
   "glReadBuffer",
   "glReadPixels",
   "glRectd",
   "glRectdv",
   "glRectf",
   "glRectfv",
   "glRecti",
   "glRectiv",
   "glRects",
   "glRectsv",
   "glRenderMode",
   "glRotated",
   "glRotatef",
   "glScaled",
   "glScalef",
   "glScissor",
   "glSelectBuffer",
   "glShadeModel",
   "glStencilFunc",
   "glStencilMask",
   "glStencilOp",
   "glTexCoord1d",
   "glTexCoord1dv",
   "glTexCoord1f",
   "glTexCoord1fv",
   "glTexCoord1i",
   "glTexCoord1iv",
   "glTexCoord1s",
   "glTexCoord1sv",
   "glTexCoord2d",
   "glTexCoord2dv",
   "glTexCoord2f",
   "glTexCoord2fv",
   "glTexCoord2i",
   "glTexCoord2iv",
   "glTexCoord2s",
   "glTexCoord2sv",
   "glTexCoord3d",
   "glTexCoord3dv",
   "glTexCoord3f",
   "glTexCoord3fv",
   "glTexCoord3i",
   "glTexCoord3iv",
   "glTexCoord3s",
   "glTexCoord3sv",
   "glTexCoord4d",
   "glTexCoord4dv",
   "glTexCoord4f",
   "glTexCoord4fv",
   "glTexCoord4i",
   "glTexCoord4iv",
   "glTexCoord4s",
   "glTexCoord4sv",
   "glTexCoordPointer",
   "glTexEnvf",
   "glTexEnvfv",
   "glTexEnvi",
   "glTexEnviv",
   "glTexGend",
   "glTexGendv",
   "glTexGenf",
   "glTexGenfv",
   "glTexGeni",
   "glTexGeniv",
   "glTexImage1D",
   "glTexImage2D",
   "glTexParameterf",
   "glTexParameterfv",
   "glTexParameteri",
   "glTexParameteriv",
   "glTexSubImage1D",
   "glTexSubImage2D",
   "glTranslated",
   "glTranslatef",
   "glVertex2d",
   "glVertex2dv",
   "glVertex2f",
   "glVertex2fv",
   "glVertex2i",
   "glVertex2iv",
   "glVertex2s",
   "glVertex2sv",
   "glVertex3d",
   "glVertex3dv",
   "glVertex3f",
   "glVertex3fv",
   "glVertex3i",
   "glVertex3iv",
   "glVertex3s",
   "glVertex3sv",
   "glVertex4d",
   "glVertex4dv",
   "glVertex4f",
   "glVertex4fv",
   "glVertex4i",
   "glVertex4iv",
   "glVertex4s",
   "glVertex4sv",
   "glVertexPointer",
   "glViewport",
   "wglChoosePixelFormat",
   "wglCopyContext",
   "wglCreateContext",
   "wglCreateLayerContext",
   "wglDeleteContext",
   "wglDescribeLayerPlane",
   "wglDescribePixelFormat",
   "wglGetCurrentContext",
   "wglGetCurrentDC",
   "wglGetDefaultProcAddress",
   "wglGetLayerPaletteEntries",
   "wglGetPixelFormat",
   "wglGetProcAddress",
   "wglMakeCurrent",
   "wglRealizeLayerPalette",
   "wglSetLayerPaletteEntries",
   "wglSetPixelFormat",
   "wglShareLists",
   "wglSwapBuffers",
   "wglSwapLayerBuffers",
   "wglUseFontBitmapsA",
   "wglUseFontBitmapsW",
   "wglUseFontOutlinesA",
   "wglUseFontOutlinesW",
};

static char *GLUN[] = 
{
   "gluErrorString",
   "gluBuild2DMipmaps",
};

//
// Numerical identifiers
//

enum GLID
{
   FP_glAccum = 0,
   FP_glAlphaFunc,
   FP_glAreTexturesResident,
   FP_glArrayElement,
   FP_glBegin,
   FP_glBindTexture,
   FP_glBitmap,
   FP_glBlendFunc,
   FP_glCallList,
   FP_glCallLists,
   FP_glClear,
   FP_glClearAccum,
   FP_glClearColor,
   FP_glClearDepth,
   FP_glClearIndex,
   FP_glClearStencil,
   FP_glClipPlane,
   FP_glColor3b,
   FP_glColor3bv,
   FP_glColor3d,
   FP_glColor3dv,
   FP_glColor3f,
   FP_glColor3fv,
   FP_glColor3i,
   FP_glColor3iv,
   FP_glColor3s,
   FP_glColor3sv,
   FP_glColor3ub,
   FP_glColor3ubv,
   FP_glColor3ui,
   FP_glColor3uiv,
   FP_glColor3us,
   FP_glColor3usv,
   FP_glColor4b,
   FP_glColor4bv,
   FP_glColor4d,
   FP_glColor4dv,
   FP_glColor4f,
   FP_glColor4fv,
   FP_glColor4i,
   FP_glColor4iv,
   FP_glColor4s,
   FP_glColor4sv,
   FP_glColor4ub,
   FP_glColor4ubv,
   FP_glColor4ui,
   FP_glColor4uiv,
   FP_glColor4us,
   FP_glColor4usv,
   FP_glColorMask,
   FP_glColorMaterial,
   FP_glColorPointer,
   FP_glCopyPixels,
   FP_glCopyTexImage1D,
   FP_glCopyTexImage2D,
   FP_glCopyTexSubImage1D,
   FP_glCopyTexSubImage2D,
   FP_glCullFace,
   FP_glDeleteLists,
   FP_glDeleteTextures,
   FP_glDepthFunc,
   FP_glDepthMask,
   FP_glDepthRange,
   FP_glDisable,
   FP_glDisableClientState,
   FP_glDrawArrays,
   FP_glDrawBuffer,
   FP_glDrawElements,
   FP_glDrawPixels,
   FP_glEdgeFlag,
   FP_glEdgeFlagPointer,
   FP_glEdgeFlagv,
   FP_glEnable,
   FP_glEnableClientState,
   FP_glEnd,
   FP_glEndList,
   FP_glEvalCoord1d,
   FP_glEvalCoord1dv,
   FP_glEvalCoord1f,
   FP_glEvalCoord1fv,
   FP_glEvalCoord2d,
   FP_glEvalCoord2dv,
   FP_glEvalCoord2f,
   FP_glEvalCoord2fv,
   FP_glEvalMesh1,
   FP_glEvalMesh2,
   FP_glEvalPoint1,
   FP_glEvalPoint2,
   FP_glFeedbackBuffer,
   FP_glFinish,
   FP_glFlush,
   FP_glFogf,
   FP_glFogfv,
   FP_glFogi,
   FP_glFogiv,
   FP_glFrontFace,
   FP_glFrustum,
   FP_glGenLists,
   FP_glGenTextures,
   FP_glGetBooleanv,
   FP_glGetClipPlane,
   FP_glGetDoublev,
   FP_glGetError,
   FP_glGetFloatv,
   FP_glGetIntegerv,
   FP_glGetLightfv,
   FP_glGetLightiv,
   FP_glGetMapdv,
   FP_glGetMapfv,
   FP_glGetMapiv,
   FP_glGetMaterialfv,
   FP_glGetMaterialiv,
   FP_glGetPixelMapfv,
   FP_glGetPixelMapuiv,
   FP_glGetPixelMapusv,
   FP_glGetPointerv,
   FP_glGetPolygonStipple,
   FP_glGetString,
   FP_glGetTexEnvfv,
   FP_glGetTexEnviv,
   FP_glGetTexGendv,
   FP_glGetTexGenfv,
   FP_glGetTexGeniv,
   FP_glGetTexImage,
   FP_glGetTexLevelParameterfv,
   FP_glGetTexLevelParameteriv,
   FP_glGetTexParameterfv,
   FP_glGetTexParameteriv,
   FP_glHint,
   FP_glIndexMask,
   FP_glIndexPointer,
   FP_glIndexd,
   FP_glIndexdv,
   FP_glIndexf,
   FP_glIndexfv,
   FP_glIndexi,
   FP_glIndexiv,
   FP_glIndexs,
   FP_glIndexsv,
   FP_glIndexub,
   FP_glIndexubv,
   FP_glInitNames,
   FP_glInterleavedArrays,
   FP_glIsEnabled,
   FP_glIsList,
   FP_glIsTexture,
   FP_glLightModelf,
   FP_glLightModelfv,
   FP_glLightModeli,
   FP_glLightModeliv,
   FP_glLightf,
   FP_glLightfv,
   FP_glLighti,
   FP_glLightiv,
   FP_glLineStipple,
   FP_glLineWidth,
   FP_glListBase,
   FP_glLoadIdentity,
   FP_glLoadMatrixd,
   FP_glLoadMatrixf,
   FP_glLoadName,
   FP_glLogicOp,
   FP_glMap1d,
   FP_glMap1f,
   FP_glMap2d,
   FP_glMap2f,
   FP_glMapGrid1d,
   FP_glMapGrid1f,
   FP_glMapGrid2d,
   FP_glMapGrid2f,
   FP_glMaterialf,
   FP_glMaterialfv,
   FP_glMateriali,
   FP_glMaterialiv,
   FP_glMatrixMode,
   FP_glMultMatrixd,
   FP_glMultMatrixf,
   FP_glNewList,
   FP_glNormal3b,
   FP_glNormal3bv,
   FP_glNormal3d,
   FP_glNormal3dv,
   FP_glNormal3f,
   FP_glNormal3fv,
   FP_glNormal3i,
   FP_glNormal3iv,
   FP_glNormal3s,
   FP_glNormal3sv,
   FP_glNormalPointer,
   FP_glOrtho,
   FP_glPassThrough,
   FP_glPixelMapfv,
   FP_glPixelMapuiv,
   FP_glPixelMapusv,
   FP_glPixelStoref,
   FP_glPixelStorei,
   FP_glPixelTransferf,
   FP_glPixelTransferi,
   FP_glPixelZoom,
   FP_glPointSize,
   FP_glPolygonMode,
   FP_glPolygonOffset,
   FP_glPolygonStipple,
   FP_glPopAttrib,
   FP_glPopClientAttrib,
   FP_glPopMatrix,
   FP_glPopName,
   FP_glPrioritizeTextures,
   FP_glPushAttrib,
   FP_glPushClientAttrib,
   FP_glPushMatrix,
   FP_glPushName,
   FP_glRasterPos2d,
   FP_glRasterPos2dv,
   FP_glRasterPos2f,
   FP_glRasterPos2fv,
   FP_glRasterPos2i,
   FP_glRasterPos2iv,
   FP_glRasterPos2s,
   FP_glRasterPos2sv,
   FP_glRasterPos3d,
   FP_glRasterPos3dv,
   FP_glRasterPos3f,
   FP_glRasterPos3fv,
   FP_glRasterPos3i,
   FP_glRasterPos3iv,
   FP_glRasterPos3s,
   FP_glRasterPos3sv,
   FP_glRasterPos4d,
   FP_glRasterPos4dv,
   FP_glRasterPos4f,
   FP_glRasterPos4fv,
   FP_glRasterPos4i,
   FP_glRasterPos4iv,
   FP_glRasterPos4s,
   FP_glRasterPos4sv,
   FP_glReadBuffer,
   FP_glReadPixels,
   FP_glRectd,
   FP_glRectdv,
   FP_glRectf,
   FP_glRectfv,
   FP_glRecti,
   FP_glRectiv,
   FP_glRects,
   FP_glRectsv,
   FP_glRenderMode,
   FP_glRotated,
   FP_glRotatef,
   FP_glScaled,
   FP_glScalef,
   FP_glScissor,
   FP_glSelectBuffer,
   FP_glShadeModel,
   FP_glStencilFunc,
   FP_glStencilMask,
   FP_glStencilOp,
   FP_glTexCoord1d,
   FP_glTexCoord1dv,
   FP_glTexCoord1f,
   FP_glTexCoord1fv,
   FP_glTexCoord1i,
   FP_glTexCoord1iv,
   FP_glTexCoord1s,
   FP_glTexCoord1sv,
   FP_glTexCoord2d,
   FP_glTexCoord2dv,
   FP_glTexCoord2f,
   FP_glTexCoord2fv,
   FP_glTexCoord2i,
   FP_glTexCoord2iv,
   FP_glTexCoord2s,
   FP_glTexCoord2sv,
   FP_glTexCoord3d,
   FP_glTexCoord3dv,
   FP_glTexCoord3f,
   FP_glTexCoord3fv,
   FP_glTexCoord3i,
   FP_glTexCoord3iv,
   FP_glTexCoord3s,
   FP_glTexCoord3sv,
   FP_glTexCoord4d,
   FP_glTexCoord4dv,
   FP_glTexCoord4f,
   FP_glTexCoord4fv,
   FP_glTexCoord4i,
   FP_glTexCoord4iv,
   FP_glTexCoord4s,
   FP_glTexCoord4sv,
   FP_glTexCoordPointer,
   FP_glTexEnvf,
   FP_glTexEnvfv,
   FP_glTexEnvi,
   FP_glTexEnviv,
   FP_glTexGend,
   FP_glTexGendv,
   FP_glTexGenf,
   FP_glTexGenfv,
   FP_glTexGeni,
   FP_glTexGeniv,
   FP_glTexImage1D,
   FP_glTexImage2D,
   FP_glTexParameterf,
   FP_glTexParameterfv,
   FP_glTexParameteri,
   FP_glTexParameteriv,
   FP_glTexSubImage1D,
   FP_glTexSubImage2D,
   FP_glTranslated,
   FP_glTranslatef,
   FP_glVertex2d,
   FP_glVertex2dv,
   FP_glVertex2f,
   FP_glVertex2fv,
   FP_glVertex2i,
   FP_glVertex2iv,
   FP_glVertex2s,
   FP_glVertex2sv,
   FP_glVertex3d,
   FP_glVertex3dv,
   FP_glVertex3f,
   FP_glVertex3fv,
   FP_glVertex3i,
   FP_glVertex3iv,
   FP_glVertex3s,
   FP_glVertex3sv,
   FP_glVertex4d,
   FP_glVertex4dv,
   FP_glVertex4f,
   FP_glVertex4fv,
   FP_glVertex4i,
   FP_glVertex4iv,
   FP_glVertex4s,
   FP_glVertex4sv,
   FP_glVertexPointer,
   FP_glViewport,
   FP_wglChoosePixelFormat,
   FP_wglCopyContext,
   FP_wglCreateContext,
   FP_wglCreateLayerContext,
   FP_wglDeleteContext,
   FP_wglDescribeLayerPlane,
   FP_wglDescribePixelFormat,
   FP_wglGetCurrentContext,
   FP_wglGetCurrentDC,
   FP_wglGetDefaultProcAddress,
   FP_wglGetLayerPaletteEntries,
   FP_wglGetPixelFormat,
   FP_wglGetProcAddress,
   FP_wglMakeCurrent,
   FP_wglRealizeLayerPalette,
   FP_wglSetLayerPaletteEntries,
   FP_wglSetPixelFormat,
   FP_wglShareLists,
   FP_wglSwapBuffers,
   FP_wglSwapLayerBuffers,
   FP_wglUseFontBitmapsA,
   FP_wglUseFontBitmapsW,
   FP_wglUseFontOutlinesA,
   FP_wglUseFontOutlinesW,
};

enum GLUID
{
   FP_gluErrorString,
   FP_gluBuild2DMipmaps,
};

//
// Jump table vectors to GLTRACE entries (aka "real" OPENGL32.DLL)
//

struct
{
   void (APIENTRY *glAccum) (GLenum op, GLfloat value);
   void (APIENTRY *glAlphaFunc) (GLenum func, GLclampf ref);
   GLboolean (APIENTRY *glAreTexturesResident) (GLsizei n, const GLuint *textures, GLboolean *residences);
   void (APIENTRY *glArrayElement) (GLint i);
   void (APIENTRY *glBegin) (GLenum mode);
   void (APIENTRY *glBindTexture) (GLenum target, GLuint texture);
   void (APIENTRY *glBitmap) (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
   void (APIENTRY *glBlendFunc) (GLenum sfactor, GLenum dfactor);
   void (APIENTRY *glCallList) (GLuint list);
   void (APIENTRY *glCallLists) (GLsizei n, GLenum type, const GLvoid *lists);
   void (APIENTRY *glClear) (GLbitfield mask);
   void (APIENTRY *glClearAccum) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
   void (APIENTRY *glClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void (APIENTRY *glClearDepth) (GLclampd depth);
   void (APIENTRY *glClearIndex) (GLfloat c);
   void (APIENTRY *glClearStencil) (GLint s);
   void (APIENTRY *glClipPlane) (GLenum plane, const GLdouble *equation);
   void (APIENTRY *glColor3b) (GLbyte red, GLbyte green, GLbyte blue);
   void (APIENTRY *glColor3bv) (const GLbyte *v);
   void (APIENTRY *glColor3d) (GLdouble red, GLdouble green, GLdouble blue);
   void (APIENTRY *glColor3dv) (const GLdouble *v);
   void (APIENTRY *glColor3f) (GLfloat red, GLfloat green, GLfloat blue);
   void (APIENTRY *glColor3fv) (const GLfloat *v);
   void (APIENTRY *glColor3i) (GLint red, GLint green, GLint blue);
   void (APIENTRY *glColor3iv) (const GLint *v);
   void (APIENTRY *glColor3s) (GLshort red, GLshort green, GLshort blue);
   void (APIENTRY *glColor3sv) (const GLshort *v);
   void (APIENTRY *glColor3ub) (GLubyte red, GLubyte green, GLubyte blue);
   void (APIENTRY *glColor3ubv) (const GLubyte *v);
   void (APIENTRY *glColor3ui) (GLuint red, GLuint green, GLuint blue);
   void (APIENTRY *glColor3uiv) (const GLuint *v);
   void (APIENTRY *glColor3us) (GLushort red, GLushort green, GLushort blue);
   void (APIENTRY *glColor3usv) (const GLushort *v);
   void (APIENTRY *glColor4b) (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
   void (APIENTRY *glColor4bv) (const GLbyte *v);
   void (APIENTRY *glColor4d) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
   void (APIENTRY *glColor4dv) (const GLdouble *v);
   void (APIENTRY *glColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
   void (APIENTRY *glColor4fv) (const GLfloat *v);
   void (APIENTRY *glColor4i) (GLint red, GLint green, GLint blue, GLint alpha);
   void (APIENTRY *glColor4iv) (const GLint *v);
   void (APIENTRY *glColor4s) (GLshort red, GLshort green, GLshort blue, GLshort alpha);
   void (APIENTRY *glColor4sv) (const GLshort *v);
   void (APIENTRY *glColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
   void (APIENTRY *glColor4ubv) (const GLubyte *v);
   void (APIENTRY *glColor4ui) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
   void (APIENTRY *glColor4uiv) (const GLuint *v);
   void (APIENTRY *glColor4us) (GLushort red, GLushort green, GLushort blue, GLushort alpha);
   void (APIENTRY *glColor4usv) (const GLushort *v);
   void (APIENTRY *glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
   void (APIENTRY *glColorMaterial) (GLenum face, GLenum mode);
   void (APIENTRY *glColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (APIENTRY *glCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
   void (APIENTRY *glCopyTexImage1D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
   void (APIENTRY *glCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
   void (APIENTRY *glCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
   void (APIENTRY *glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
   void (APIENTRY *glCullFace) (GLenum mode);
   void (APIENTRY *glDeleteLists) (GLuint list, GLsizei range);
   void (APIENTRY *glDeleteTextures) (GLsizei n, const GLuint *textures);
   void (APIENTRY *glDepthFunc) (GLenum func);
   void (APIENTRY *glDepthMask) (GLboolean flag);
   void (APIENTRY *glDepthRange) (GLclampd zNear, GLclampd zFar);
   void (APIENTRY *glDisable) (GLenum cap);
   void (APIENTRY *glDisableClientState) (GLenum array);
   void (APIENTRY *glDrawArrays) (GLenum mode, GLint first, GLsizei count);
   void (APIENTRY *glDrawBuffer) (GLenum mode);
   void (APIENTRY *glDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
   void (APIENTRY *glDrawPixels) (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
   void (APIENTRY *glEdgeFlag) (GLboolean flag);
   void (APIENTRY *glEdgeFlagPointer) (GLsizei stride, const GLvoid *pointer);
   void (APIENTRY *glEdgeFlagv) (const GLboolean *flag);
   void (APIENTRY *glEnable) (GLenum cap);
   void (APIENTRY *glEnableClientState) (GLenum array);
   void (APIENTRY *glEnd) (void);
   void (APIENTRY *glEndList) (void);
   void (APIENTRY *glEvalCoord1d) (GLdouble u);
   void (APIENTRY *glEvalCoord1dv) (const GLdouble *u);
   void (APIENTRY *glEvalCoord1f) (GLfloat u);
   void (APIENTRY *glEvalCoord1fv) (const GLfloat *u);
   void (APIENTRY *glEvalCoord2d) (GLdouble u, GLdouble v);
   void (APIENTRY *glEvalCoord2dv) (const GLdouble *u);
   void (APIENTRY *glEvalCoord2f) (GLfloat u, GLfloat v);
   void (APIENTRY *glEvalCoord2fv) (const GLfloat *u);
   void (APIENTRY *glEvalMesh1) (GLenum mode, GLint i1, GLint i2);
   void (APIENTRY *glEvalMesh2) (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
   void (APIENTRY *glEvalPoint1) (GLint i);
   void (APIENTRY *glEvalPoint2) (GLint i, GLint j);
   void (APIENTRY *glFeedbackBuffer) (GLsizei size, GLenum type, GLfloat *buffer);
   void (APIENTRY *glFinish) (void);
   void (APIENTRY *glFlush) (void);
   void (APIENTRY *glFogf) (GLenum pname, GLfloat param);
   void (APIENTRY *glFogfv) (GLenum pname, const GLfloat *params);
   void (APIENTRY *glFogi) (GLenum pname, GLint param);
   void (APIENTRY *glFogiv) (GLenum pname, const GLint *params);
   void (APIENTRY *glFrontFace) (GLenum mode);
   void (APIENTRY *glFrustum) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
   GLuint (APIENTRY *glGenLists) (GLsizei range);
   void (APIENTRY *glGenTextures) (GLsizei n, GLuint *textures);
   void (APIENTRY *glGetBooleanv) (GLenum pname, GLboolean *params);
   void (APIENTRY *glGetClipPlane) (GLenum plane, GLdouble *equation);
   void (APIENTRY *glGetDoublev) (GLenum pname, GLdouble *params);
   GLenum (APIENTRY *glGetError) (void);
   void (APIENTRY *glGetFloatv) (GLenum pname, GLfloat *params);
   void (APIENTRY *glGetIntegerv) (GLenum pname, GLint *params);
   void (APIENTRY *glGetLightfv) (GLenum light, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetLightiv) (GLenum light, GLenum pname, GLint *params);
   void (APIENTRY *glGetMapdv) (GLenum target, GLenum query, GLdouble *v);
   void (APIENTRY *glGetMapfv) (GLenum target, GLenum query, GLfloat *v);
   void (APIENTRY *glGetMapiv) (GLenum target, GLenum query, GLint *v);
   void (APIENTRY *glGetMaterialfv) (GLenum face, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetMaterialiv) (GLenum face, GLenum pname, GLint *params);
   void (APIENTRY *glGetPixelMapfv) (GLenum map, GLfloat *values);
   void (APIENTRY *glGetPixelMapuiv) (GLenum map, GLuint *values);
   void (APIENTRY *glGetPixelMapusv) (GLenum map, GLushort *values);
   void (APIENTRY *glGetPointerv) (GLenum pname, GLvoid* *params);
   void (APIENTRY *glGetPolygonStipple) (GLubyte *mask);
   const GLstring (APIENTRY *glGetString) (GLenum name);
   void (APIENTRY *glGetTexEnvfv) (GLenum target, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetTexEnviv) (GLenum target, GLenum pname, GLint *params);
   void (APIENTRY *glGetTexGendv) (GLenum coord, GLenum pname, GLdouble *params);
   void (APIENTRY *glGetTexGenfv) (GLenum coord, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetTexGeniv) (GLenum coord, GLenum pname, GLint *params);
   void (APIENTRY *glGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
   void (APIENTRY *glGetTexLevelParameterfv) (GLenum target, GLint level, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
   void (APIENTRY *glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetTexParameteriv) (GLenum target, GLenum pname, GLint *params);
   void (APIENTRY *glHint) (GLenum target, GLenum mode);
   void (APIENTRY *glIndexMask) (GLuint mask);
   void (APIENTRY *glIndexPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
   void (APIENTRY *glIndexd) (GLdouble c);
   void (APIENTRY *glIndexdv) (const GLdouble *c);
   void (APIENTRY *glIndexf) (GLfloat c);
   void (APIENTRY *glIndexfv) (const GLfloat *c);
   void (APIENTRY *glIndexi) (GLint c);
   void (APIENTRY *glIndexiv) (const GLint *c);
   void (APIENTRY *glIndexs) (GLshort c);
   void (APIENTRY *glIndexsv) (const GLshort *c);
   void (APIENTRY *glIndexub) (GLubyte c);
   void (APIENTRY *glIndexubv) (const GLubyte *c);
   void (APIENTRY *glInitNames) (void);
   void (APIENTRY *glInterleavedArrays) (GLenum format, GLsizei stride, const GLvoid *pointer);
   GLboolean (APIENTRY *glIsEnabled) (GLenum cap);
   GLboolean (APIENTRY *glIsList) (GLuint list);
   GLboolean (APIENTRY *glIsTexture) (GLuint texture);
   void (APIENTRY *glLightModelf) (GLenum pname, GLfloat param);
   void (APIENTRY *glLightModelfv) (GLenum pname, const GLfloat *params);
   void (APIENTRY *glLightModeli) (GLenum pname, GLint param);
   void (APIENTRY *glLightModeliv) (GLenum pname, const GLint *params);
   void (APIENTRY *glLightf) (GLenum light, GLenum pname, GLfloat param);
   void (APIENTRY *glLightfv) (GLenum light, GLenum pname, const GLfloat *params);
   void (APIENTRY *glLighti) (GLenum light, GLenum pname, GLint param);
   void (APIENTRY *glLightiv) (GLenum light, GLenum pname, const GLint *params);
   void (APIENTRY *glLineStipple) (GLint factor, GLushort pattern);
   void (APIENTRY *glLineWidth) (GLfloat width);
   void (APIENTRY *glListBase) (GLuint base);
   void (APIENTRY *glLoadIdentity) (void);
   void (APIENTRY *glLoadMatrixd) (const GLdouble *m);
   void (APIENTRY *glLoadMatrixf) (const GLfloat *m);
   void (APIENTRY *glLoadName) (GLuint name);
   void (APIENTRY *glLogicOp) (GLenum opcode);
   void (APIENTRY *glMap1d) (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
   void (APIENTRY *glMap1f) (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
   void (APIENTRY *glMap2d) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
   void (APIENTRY *glMap2f) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
   void (APIENTRY *glMapGrid1d) (GLint un, GLdouble u1, GLdouble u2);
   void (APIENTRY *glMapGrid1f) (GLint un, GLfloat u1, GLfloat u2);
   void (APIENTRY *glMapGrid2d) (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
   void (APIENTRY *glMapGrid2f) (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
   void (APIENTRY *glMaterialf) (GLenum face, GLenum pname, GLfloat param);
   void (APIENTRY *glMaterialfv) (GLenum face, GLenum pname, const GLfloat *params);
   void (APIENTRY *glMateriali) (GLenum face, GLenum pname, GLint param);
   void (APIENTRY *glMaterialiv) (GLenum face, GLenum pname, const GLint *params);
   void (APIENTRY *glMatrixMode) (GLenum mode);
   void (APIENTRY *glMultMatrixd) (const GLdouble *m);
   void (APIENTRY *glMultMatrixf) (const GLfloat *m);
   void (APIENTRY *glNewList) (GLuint list, GLenum mode);
   void (APIENTRY *glNormal3b) (GLbyte nx, GLbyte ny, GLbyte nz);
   void (APIENTRY *glNormal3bv) (const GLbyte *v);
   void (APIENTRY *glNormal3d) (GLdouble nx, GLdouble ny, GLdouble nz);
   void (APIENTRY *glNormal3dv) (const GLdouble *v);
   void (APIENTRY *glNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
   void (APIENTRY *glNormal3fv) (const GLfloat *v);
   void (APIENTRY *glNormal3i) (GLint nx, GLint ny, GLint nz);
   void (APIENTRY *glNormal3iv) (const GLint *v);
   void (APIENTRY *glNormal3s) (GLshort nx, GLshort ny, GLshort nz);
   void (APIENTRY *glNormal3sv) (const GLshort *v);
   void (APIENTRY *glNormalPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
   void (APIENTRY *glOrtho) (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
   void (APIENTRY *glPassThrough) (GLfloat token);
   void (APIENTRY *glPixelMapfv) (GLenum map, GLsizei mapsize, const GLfloat *values);
   void (APIENTRY *glPixelMapuiv) (GLenum map, GLsizei mapsize, const GLuint *values);
   void (APIENTRY *glPixelMapusv) (GLenum map, GLsizei mapsize, const GLushort *values);
   void (APIENTRY *glPixelStoref) (GLenum pname, GLfloat param);
   void (APIENTRY *glPixelStorei) (GLenum pname, GLint param);
   void (APIENTRY *glPixelTransferf) (GLenum pname, GLfloat param);
   void (APIENTRY *glPixelTransferi) (GLenum pname, GLint param);
   void (APIENTRY *glPixelZoom) (GLfloat xfactor, GLfloat yfactor);
   void (APIENTRY *glPointSize) (GLfloat size);
   void (APIENTRY *glPolygonMode) (GLenum face, GLenum mode);
   void (APIENTRY *glPolygonOffset) (GLfloat factor, GLfloat units);
   void (APIENTRY *glPolygonStipple) (const GLubyte *mask);
   void (APIENTRY *glPopAttrib) (void);
   void (APIENTRY *glPopClientAttrib) (void);
   void (APIENTRY *glPopMatrix) (void);
   void (APIENTRY *glPopName) (void);
   void (APIENTRY *glPrioritizeTextures) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
   void (APIENTRY *glPushAttrib) (GLbitfield mask);
   void (APIENTRY *glPushClientAttrib) (GLbitfield mask);
   void (APIENTRY *glPushMatrix) (void);
   void (APIENTRY *glPushName) (GLuint name);
   void (APIENTRY *glRasterPos2d) (GLdouble x, GLdouble y);
   void (APIENTRY *glRasterPos2dv) (const GLdouble *v);
   void (APIENTRY *glRasterPos2f) (GLfloat x, GLfloat y);
   void (APIENTRY *glRasterPos2fv) (const GLfloat *v);
   void (APIENTRY *glRasterPos2i) (GLint x, GLint y);
   void (APIENTRY *glRasterPos2iv) (const GLint *v);
   void (APIENTRY *glRasterPos2s) (GLshort x, GLshort y);
   void (APIENTRY *glRasterPos2sv) (const GLshort *v);
   void (APIENTRY *glRasterPos3d) (GLdouble x, GLdouble y, GLdouble z);
   void (APIENTRY *glRasterPos3dv) (const GLdouble *v);
   void (APIENTRY *glRasterPos3f) (GLfloat x, GLfloat y, GLfloat z);
   void (APIENTRY *glRasterPos3fv) (const GLfloat *v);
   void (APIENTRY *glRasterPos3i) (GLint x, GLint y, GLint z);
   void (APIENTRY *glRasterPos3iv) (const GLint *v);
   void (APIENTRY *glRasterPos3s) (GLshort x, GLshort y, GLshort z);
   void (APIENTRY *glRasterPos3sv) (const GLshort *v);
   void (APIENTRY *glRasterPos4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
   void (APIENTRY *glRasterPos4dv) (const GLdouble *v);
   void (APIENTRY *glRasterPos4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void (APIENTRY *glRasterPos4fv) (const GLfloat *v);
   void (APIENTRY *glRasterPos4i) (GLint x, GLint y, GLint z, GLint w);
   void (APIENTRY *glRasterPos4iv) (const GLint *v);
   void (APIENTRY *glRasterPos4s) (GLshort x, GLshort y, GLshort z, GLshort w);
   void (APIENTRY *glRasterPos4sv) (const GLshort *v);
   void (APIENTRY *glReadBuffer) (GLenum mode);
   void (APIENTRY *glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
   void (APIENTRY *glRectd) (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
   void (APIENTRY *glRectdv) (const GLdouble *v1, const GLdouble *v2);
   void (APIENTRY *glRectf) (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
   void (APIENTRY *glRectfv) (const GLfloat *v1, const GLfloat *v2);
   void (APIENTRY *glRecti) (GLint x1, GLint y1, GLint x2, GLint y2);
   void (APIENTRY *glRectiv) (const GLint *v1, const GLint *v2);
   void (APIENTRY *glRects) (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
   void (APIENTRY *glRectsv) (const GLshort *v1, const GLshort *v2);
   GLint (APIENTRY *glRenderMode) (GLenum mode);
   void (APIENTRY *glRotated) (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
   void (APIENTRY *glRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
   void (APIENTRY *glScaled) (GLdouble x, GLdouble y, GLdouble z);
   void (APIENTRY *glScalef) (GLfloat x, GLfloat y, GLfloat z);
   void (APIENTRY *glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
   void (APIENTRY *glSelectBuffer) (GLsizei size, GLuint *buffer);
   void (APIENTRY *glShadeModel) (GLenum mode);
   void (APIENTRY *glStencilFunc) (GLenum func, GLint ref, GLuint mask);
   void (APIENTRY *glStencilMask) (GLuint mask);
   void (APIENTRY *glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
   void (APIENTRY *glTexCoord1d) (GLdouble s);
   void (APIENTRY *glTexCoord1dv) (const GLdouble *v);
   void (APIENTRY *glTexCoord1f) (GLfloat s);
   void (APIENTRY *glTexCoord1fv) (const GLfloat *v);
   void (APIENTRY *glTexCoord1i) (GLint s);
   void (APIENTRY *glTexCoord1iv) (const GLint *v);
   void (APIENTRY *glTexCoord1s) (GLshort s);
   void (APIENTRY *glTexCoord1sv) (const GLshort *v);
   void (APIENTRY *glTexCoord2d) (GLdouble s, GLdouble t);
   void (APIENTRY *glTexCoord2dv) (const GLdouble *v);
   void (APIENTRY *glTexCoord2f) (GLfloat s, GLfloat t);
   void (APIENTRY *glTexCoord2fv) (const GLfloat *v);
   void (APIENTRY *glTexCoord2i) (GLint s, GLint t);
   void (APIENTRY *glTexCoord2iv) (const GLint *v);
   void (APIENTRY *glTexCoord2s) (GLshort s, GLshort t);
   void (APIENTRY *glTexCoord2sv) (const GLshort *v);
   void (APIENTRY *glTexCoord3d) (GLdouble s, GLdouble t, GLdouble r);
   void (APIENTRY *glTexCoord3dv) (const GLdouble *v);
   void (APIENTRY *glTexCoord3f) (GLfloat s, GLfloat t, GLfloat r);
   void (APIENTRY *glTexCoord3fv) (const GLfloat *v);
   void (APIENTRY *glTexCoord3i) (GLint s, GLint t, GLint r);
   void (APIENTRY *glTexCoord3iv) (const GLint *v);
   void (APIENTRY *glTexCoord3s) (GLshort s, GLshort t, GLshort r);
   void (APIENTRY *glTexCoord3sv) (const GLshort *v);
   void (APIENTRY *glTexCoord4d) (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
   void (APIENTRY *glTexCoord4dv) (const GLdouble *v);
   void (APIENTRY *glTexCoord4f) (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
   void (APIENTRY *glTexCoord4fv) (const GLfloat *v);
   void (APIENTRY *glTexCoord4i) (GLint s, GLint t, GLint r, GLint q);
   void (APIENTRY *glTexCoord4iv) (const GLint *v);
   void (APIENTRY *glTexCoord4s) (GLshort s, GLshort t, GLshort r, GLshort q);
   void (APIENTRY *glTexCoord4sv) (const GLshort *v);
   void (APIENTRY *glTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (APIENTRY *glTexEnvf) (GLenum target, GLenum pname, GLfloat param);
   void (APIENTRY *glTexEnvfv) (GLenum target, GLenum pname, const GLfloat *params);
   void (APIENTRY *glTexEnvi) (GLenum target, GLenum pname, GLint param);
   void (APIENTRY *glTexEnviv) (GLenum target, GLenum pname, const GLint *params);
   void (APIENTRY *glTexGend) (GLenum coord, GLenum pname, GLdouble param);
   void (APIENTRY *glTexGendv) (GLenum coord, GLenum pname, const GLdouble *params);
   void (APIENTRY *glTexGenf) (GLenum coord, GLenum pname, GLfloat param);
   void (APIENTRY *glTexGenfv) (GLenum coord, GLenum pname, const GLfloat *params);
   void (APIENTRY *glTexGeni) (GLenum coord, GLenum pname, GLint param);
   void (APIENTRY *glTexGeniv) (GLenum coord, GLenum pname, const GLint *params);
   void (APIENTRY *glTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
   void (APIENTRY *glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
   void (APIENTRY *glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
   void (APIENTRY *glTexParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
   void (APIENTRY *glTexParameteri) (GLenum target, GLenum pname, GLint param);
   void (APIENTRY *glTexParameteriv) (GLenum target, GLenum pname, const GLint *params);
   void (APIENTRY *glTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
   void (APIENTRY *glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
   void (APIENTRY *glTranslated) (GLdouble x, GLdouble y, GLdouble z);
   void (APIENTRY *glTranslatef) (GLfloat x, GLfloat y, GLfloat z);
   void (APIENTRY *glVertex2d) (GLdouble x, GLdouble y);
   void (APIENTRY *glVertex2dv) (const GLdouble *v);
   void (APIENTRY *glVertex2f) (GLfloat x, GLfloat y);
   void (APIENTRY *glVertex2fv) (const GLfloat *v);
   void (APIENTRY *glVertex2i) (GLint x, GLint y);
   void (APIENTRY *glVertex2iv) (const GLint *v);
   void (APIENTRY *glVertex2s) (GLshort x, GLshort y);
   void (APIENTRY *glVertex2sv) (const GLshort *v);
   void (APIENTRY *glVertex3d) (GLdouble x, GLdouble y, GLdouble z);
   void (APIENTRY *glVertex3dv) (const GLdouble *v);
   void (APIENTRY *glVertex3f) (GLfloat x, GLfloat y, GLfloat z);
   void (APIENTRY *glVertex3fv) (const GLfloat *v);
   void (APIENTRY *glVertex3i) (GLint x, GLint y, GLint z);
   void (APIENTRY *glVertex3iv) (const GLint *v);
   void (APIENTRY *glVertex3s) (GLshort x, GLshort y, GLshort z);
   void (APIENTRY *glVertex3sv) (const GLshort *v);
   void (APIENTRY *glVertex4d) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
   void (APIENTRY *glVertex4dv) (const GLdouble *v);
   void (APIENTRY *glVertex4f) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void (APIENTRY *glVertex4fv) (const GLfloat *v);
   void (APIENTRY *glVertex4i) (GLint x, GLint y, GLint z, GLint w);
   void (APIENTRY *glVertex4iv) (const GLint *v);
   void (APIENTRY *glVertex4s) (GLshort x, GLshort y, GLshort z, GLshort w);
   void (APIENTRY *glVertex4sv) (const GLshort *v);
   void (APIENTRY *glVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (APIENTRY *glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
   int   (APIENTRY *wglChoosePixelFormat)       (HDC a, CONST PIXELFORMATDESCRIPTOR *b);
   BOOL  (APIENTRY *wglCopyContext)             (HGLRC a, HGLRC b, UINT c);
   HGLRC (APIENTRY *wglCreateContext)           (HDC a);
   HGLRC (APIENTRY *wglCreateLayerContext)      (HDC a, int b);
   BOOL  (APIENTRY *wglDeleteContext)           (HGLRC a);
   BOOL  (APIENTRY *wglDescribeLayerPlane)      (HDC a, int b, int c, UINT d, LPLAYERPLANEDESCRIPTOR e);
   int   (APIENTRY *wglDescribePixelFormat)     (HDC a, int b, UINT c, LPPIXELFORMATDESCRIPTOR d);
   HGLRC (APIENTRY *wglGetCurrentContext)       (void);
   HDC   (APIENTRY *wglGetCurrentDC)            (void);
   PROC  (APIENTRY *wglGetDefaultProcAddress)   (LPCSTR a);
   int   (APIENTRY *wglGetLayerPaletteEntries)  (HDC a, int b, int c, int d, COLORREF *e);
   int   (APIENTRY *wglGetPixelFormat)          (HDC a);
   PROC  (APIENTRY *wglGetProcAddress)          (LPCSTR a);
   BOOL  (APIENTRY *wglMakeCurrent)             (HDC a, HGLRC b);
   BOOL  (APIENTRY *wglRealizeLayerPalette)     (HDC a, int b, BOOL c);
   int   (APIENTRY *wglSetLayerPaletteEntries)  (HDC a, int b, int c, int d, CONST COLORREF *e);
   BOOL  (APIENTRY *wglSetPixelFormat)          (HDC a, int b, CONST PIXELFORMATDESCRIPTOR *c);
   BOOL  (APIENTRY *wglShareLists)              (HGLRC a, HGLRC b);
   BOOL  (APIENTRY *wglSwapBuffers)             (HDC a);
   BOOL  (APIENTRY *wglSwapLayerBuffers)        (HDC a, UINT b);
   BOOL  (APIENTRY *wglUseFontBitmapsA)         (HDC a, DWORD b, DWORD c, DWORD d);
   BOOL  (APIENTRY *wglUseFontBitmapsW)         (HDC a, DWORD b, DWORD c, DWORD d);
   BOOL  (APIENTRY *wglUseFontOutlinesA)        (HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);
   BOOL  (APIENTRY *wglUseFontOutlinesW)        (HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);
}
GLV;


struct
{
   const unsigned char * (APIENTRY *gluErrorString) (GLenum   errCode);
   int   (APIENTRY *gluBuild2DMipmaps) 			(GLenum target, GLint components, GLint width, GLint height, GLenum format, GLenum type, const void  *data);
}
GLUV;

//
// OpenGL function handler prototypes
//

extern "C"
{
void APIENTRY glAccum (GLenum op, GLfloat value);
void APIENTRY glAlphaFunc (GLenum func, GLclampf ref);
GLboolean APIENTRY glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences);
void APIENTRY glArrayElement (GLint i);
void APIENTRY glBegin (GLenum mode);
void APIENTRY glBindTexture (GLenum target, GLuint texture);
void APIENTRY glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor);
void APIENTRY glCallList (GLuint list);
void APIENTRY glCallLists (GLsizei n, GLenum type, const GLvoid *lists);
void APIENTRY glClear (GLbitfield mask);
void APIENTRY glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void APIENTRY glClearDepth (GLclampd depth);
void APIENTRY glClearIndex (GLfloat c);
void APIENTRY glClearStencil (GLint s);
void APIENTRY glClipPlane (GLenum plane, const GLdouble *equation);
void APIENTRY glColor3b (GLbyte red, GLbyte green, GLbyte blue);
void APIENTRY glColor3bv (const GLbyte *v);
void APIENTRY glColor3d (GLdouble red, GLdouble green, GLdouble blue);
void APIENTRY glColor3dv (const GLdouble *v);
void APIENTRY glColor3f (GLfloat red, GLfloat green, GLfloat blue);
void APIENTRY glColor3fv (const GLfloat *v);
void APIENTRY glColor3i (GLint red, GLint green, GLint blue);
void APIENTRY glColor3iv (const GLint *v);
void APIENTRY glColor3s (GLshort red, GLshort green, GLshort blue);
void APIENTRY glColor3sv (const GLshort *v);
void APIENTRY glColor3ub (GLubyte red, GLubyte green, GLubyte blue);
void APIENTRY glColor3ubv (const GLubyte *v);
void APIENTRY glColor3ui (GLuint red, GLuint green, GLuint blue);
void APIENTRY glColor3uiv (const GLuint *v);
void APIENTRY glColor3us (GLushort red, GLushort green, GLushort blue);
void APIENTRY glColor3usv (const GLushort *v);
void APIENTRY glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
void APIENTRY glColor4bv (const GLbyte *v);
void APIENTRY glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
void APIENTRY glColor4dv (const GLdouble *v);
void APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void APIENTRY glColor4fv (const GLfloat *v);
void APIENTRY glColor4i (GLint red, GLint green, GLint blue, GLint alpha);
void APIENTRY glColor4iv (const GLint *v);
void APIENTRY glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha);
void APIENTRY glColor4sv (const GLshort *v);
void APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void APIENTRY glColor4ubv (const GLubyte *v);
void APIENTRY glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha);
void APIENTRY glColor4uiv (const GLuint *v);
void APIENTRY glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha);
void APIENTRY glColor4usv (const GLushort *v);
void APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void APIENTRY glColorMaterial (GLenum face, GLenum mode);
void APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void APIENTRY glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void APIENTRY glCopyTexImage1D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
void APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void APIENTRY glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY glCullFace (GLenum mode);
void APIENTRY glDeleteLists (GLuint list, GLsizei range);
void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures);
void APIENTRY glDepthFunc (GLenum func);
void APIENTRY glDepthMask (GLboolean flag);
void APIENTRY glDepthRange (GLclampd zNear, GLclampd zFar);
void APIENTRY glDisable (GLenum cap);
void APIENTRY glDisableClientState (GLenum array);
void APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count);
void APIENTRY glDrawBuffer (GLenum mode);
void APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void APIENTRY glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY glEdgeFlag (GLboolean flag);
void APIENTRY glEdgeFlagPointer (GLsizei stride, const GLvoid *pointer);
void APIENTRY glEdgeFlagv (const GLboolean *flag);
void APIENTRY glEnable (GLenum cap);
void APIENTRY glEnableClientState (GLenum array);
void APIENTRY glEnd (void);
void APIENTRY glEndList (void);
void APIENTRY glEvalCoord1d (GLdouble u);
void APIENTRY glEvalCoord1dv (const GLdouble *u);
void APIENTRY glEvalCoord1f (GLfloat u);
void APIENTRY glEvalCoord1fv (const GLfloat *u);
void APIENTRY glEvalCoord2d (GLdouble u, GLdouble v);
void APIENTRY glEvalCoord2dv (const GLdouble *u);
void APIENTRY glEvalCoord2f (GLfloat u, GLfloat v);
void APIENTRY glEvalCoord2fv (const GLfloat *u);
void APIENTRY glEvalMesh1 (GLenum mode, GLint i1, GLint i2);
void APIENTRY glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void APIENTRY glEvalPoint1 (GLint i);
void APIENTRY glEvalPoint2 (GLint i, GLint j);
void APIENTRY glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer);
void APIENTRY glFinish (void);
void APIENTRY glFlush (void);
void APIENTRY glFogf (GLenum pname, GLfloat param);
void APIENTRY glFogfv (GLenum pname, const GLfloat *params);
void APIENTRY glFogi (GLenum pname, GLint param);
void APIENTRY glFogiv (GLenum pname, const GLint *params);
void APIENTRY glFrontFace (GLenum mode);
void APIENTRY glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLuint APIENTRY glGenLists (GLsizei range);
void APIENTRY glGenTextures (GLsizei n, GLuint *textures);
void APIENTRY glGetBooleanv (GLenum pname, GLboolean *params);
void APIENTRY glGetClipPlane (GLenum plane, GLdouble *equation);
void APIENTRY glGetDoublev (GLenum pname, GLdouble *params);
GLenum APIENTRY glGetError (void);
void APIENTRY glGetFloatv (GLenum pname, GLfloat *params);
void APIENTRY glGetIntegerv (GLenum pname, GLint *params);
void APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params);
void APIENTRY glGetLightiv (GLenum light, GLenum pname, GLint *params);
void APIENTRY glGetMapdv (GLenum target, GLenum query, GLdouble *v);
void APIENTRY glGetMapfv (GLenum target, GLenum query, GLfloat *v);
void APIENTRY glGetMapiv (GLenum target, GLenum query, GLint *v);
void APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params);
void APIENTRY glGetMaterialiv (GLenum face, GLenum pname, GLint *params);
void APIENTRY glGetPixelMapfv (GLenum map, GLfloat *values);
void APIENTRY glGetPixelMapuiv (GLenum map, GLuint *values);
void APIENTRY glGetPixelMapusv (GLenum map, GLushort *values);
void APIENTRY glGetPointerv (GLenum pname, GLvoid* *params);
void APIENTRY glGetPolygonStipple (GLubyte *mask);
const GLstring APIENTRY glGetString (GLenum name);
void APIENTRY glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params);
void APIENTRY glGetTexEnviv (GLenum target, GLenum pname, GLint *params);
void APIENTRY glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params);
void APIENTRY glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params);
void APIENTRY glGetTexGeniv (GLenum coord, GLenum pname, GLint *params);
void APIENTRY glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void APIENTRY glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params);
void APIENTRY glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
void APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
void APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
void APIENTRY glHint (GLenum target, GLenum mode);
void APIENTRY glIndexMask (GLuint mask);
void APIENTRY glIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
void APIENTRY glIndexd (GLdouble c);
void APIENTRY glIndexdv (const GLdouble *c);
void APIENTRY glIndexf (GLfloat c);
void APIENTRY glIndexfv (const GLfloat *c);
void APIENTRY glIndexi (GLint c);
void APIENTRY glIndexiv (const GLint *c);
void APIENTRY glIndexs (GLshort c);
void APIENTRY glIndexsv (const GLshort *c);
void APIENTRY glIndexub (GLubyte c);
void APIENTRY glIndexubv (const GLubyte *c);
void APIENTRY glInitNames (void);
void APIENTRY glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer);
GLboolean APIENTRY glIsEnabled (GLenum cap);
GLboolean APIENTRY glIsList (GLuint list);
GLboolean APIENTRY glIsTexture (GLuint texture);
void APIENTRY glLightModelf (GLenum pname, GLfloat param);
void APIENTRY glLightModelfv (GLenum pname, const GLfloat *params);
void APIENTRY glLightModeli (GLenum pname, GLint param);
void APIENTRY glLightModeliv (GLenum pname, const GLint *params);
void APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param);
void APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params);
void APIENTRY glLighti (GLenum light, GLenum pname, GLint param);
void APIENTRY glLightiv (GLenum light, GLenum pname, const GLint *params);
void APIENTRY glLineStipple (GLint factor, GLushort pattern);
void APIENTRY glLineWidth (GLfloat width);
void APIENTRY glListBase (GLuint base);
void APIENTRY glLoadIdentity (void);
void APIENTRY glLoadMatrixd (const GLdouble *m);
void APIENTRY glLoadMatrixf (const GLfloat *m);
void APIENTRY glLoadName (GLuint name);
void APIENTRY glLogicOp (GLenum opcode);
void APIENTRY glMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
void APIENTRY glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
void APIENTRY glMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
void APIENTRY glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
void APIENTRY glMapGrid1d (GLint un, GLdouble u1, GLdouble u2);
void APIENTRY glMapGrid1f (GLint un, GLfloat u1, GLfloat u2);
void APIENTRY glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void APIENTRY glMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param);
void APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params);
void APIENTRY glMateriali (GLenum face, GLenum pname, GLint param);
void APIENTRY glMaterialiv (GLenum face, GLenum pname, const GLint *params);
void APIENTRY glMatrixMode (GLenum mode);
void APIENTRY glMultMatrixd (const GLdouble *m);
void APIENTRY glMultMatrixf (const GLfloat *m);
void APIENTRY glNewList (GLuint list, GLenum mode);
void APIENTRY glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz);
void APIENTRY glNormal3bv (const GLbyte *v);
void APIENTRY glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz);
void APIENTRY glNormal3dv (const GLdouble *v);
void APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz);
void APIENTRY glNormal3fv (const GLfloat *v);
void APIENTRY glNormal3i (GLint nx, GLint ny, GLint nz);
void APIENTRY glNormal3iv (const GLint *v);
void APIENTRY glNormal3s (GLshort nx, GLshort ny, GLshort nz);
void APIENTRY glNormal3sv (const GLshort *v);
void APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
void APIENTRY glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void APIENTRY glPassThrough (GLfloat token);
void APIENTRY glPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat *values);
void APIENTRY glPixelMapuiv (GLenum map, GLsizei mapsize, const GLuint *values);
void APIENTRY glPixelMapusv (GLenum map, GLsizei mapsize, const GLushort *values);
void APIENTRY glPixelStoref (GLenum pname, GLfloat param);
void APIENTRY glPixelStorei (GLenum pname, GLint param);
void APIENTRY glPixelTransferf (GLenum pname, GLfloat param);
void APIENTRY glPixelTransferi (GLenum pname, GLint param);
void APIENTRY glPixelZoom (GLfloat xfactor, GLfloat yfactor);
void APIENTRY glPointSize (GLfloat size);
void APIENTRY glPolygonMode (GLenum face, GLenum mode);
void APIENTRY glPolygonOffset (GLfloat factor, GLfloat units);
void APIENTRY glPolygonStipple (const GLubyte *mask);
void APIENTRY glPopAttrib (void);
void APIENTRY glPopClientAttrib (void);
void APIENTRY glPopMatrix (void);
void APIENTRY glPopName (void);
void APIENTRY glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities);
void APIENTRY glPushAttrib (GLbitfield mask);
void APIENTRY glPushClientAttrib (GLbitfield mask);
void APIENTRY glPushMatrix (void);
void APIENTRY glPushName (GLuint name);
void APIENTRY glRasterPos2d (GLdouble x, GLdouble y);
void APIENTRY glRasterPos2dv (const GLdouble *v);
void APIENTRY glRasterPos2f (GLfloat x, GLfloat y);
void APIENTRY glRasterPos2fv (const GLfloat *v);
void APIENTRY glRasterPos2i (GLint x, GLint y);
void APIENTRY glRasterPos2iv (const GLint *v);
void APIENTRY glRasterPos2s (GLshort x, GLshort y);
void APIENTRY glRasterPos2sv (const GLshort *v);
void APIENTRY glRasterPos3d (GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glRasterPos3dv (const GLdouble *v);
void APIENTRY glRasterPos3f (GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glRasterPos3fv (const GLfloat *v);
void APIENTRY glRasterPos3i (GLint x, GLint y, GLint z);
void APIENTRY glRasterPos3iv (const GLint *v);
void APIENTRY glRasterPos3s (GLshort x, GLshort y, GLshort z);
void APIENTRY glRasterPos3sv (const GLshort *v);
void APIENTRY glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void APIENTRY glRasterPos4dv (const GLdouble *v);
void APIENTRY glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void APIENTRY glRasterPos4fv (const GLfloat *v);
void APIENTRY glRasterPos4i (GLint x, GLint y, GLint z, GLint w);
void APIENTRY glRasterPos4iv (const GLint *v);
void APIENTRY glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w);
void APIENTRY glRasterPos4sv (const GLshort *v);
void APIENTRY glReadBuffer (GLenum mode);
void APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void APIENTRY glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void APIENTRY glRectdv (const GLdouble *v1, const GLdouble *v2);
void APIENTRY glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void APIENTRY glRectfv (const GLfloat *v1, const GLfloat *v2);
void APIENTRY glRecti (GLint x1, GLint y1, GLint x2, GLint y2);
void APIENTRY glRectiv (const GLint *v1, const GLint *v2);
void APIENTRY glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void APIENTRY glRectsv (const GLshort *v1, const GLshort *v2);
GLint APIENTRY glRenderMode (GLenum mode);
void APIENTRY glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glScaled (GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY glSelectBuffer (GLsizei size, GLuint *buffer);
void APIENTRY glShadeModel (GLenum mode);
void APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask);
void APIENTRY glStencilMask (GLuint mask);
void APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
void APIENTRY glTexCoord1d (GLdouble s);
void APIENTRY glTexCoord1dv (const GLdouble *v);
void APIENTRY glTexCoord1f (GLfloat s);
void APIENTRY glTexCoord1fv (const GLfloat *v);
void APIENTRY glTexCoord1i (GLint s);
void APIENTRY glTexCoord1iv (const GLint *v);
void APIENTRY glTexCoord1s (GLshort s);
void APIENTRY glTexCoord1sv (const GLshort *v);
void APIENTRY glTexCoord2d (GLdouble s, GLdouble t);
void APIENTRY glTexCoord2dv (const GLdouble *v);
void APIENTRY glTexCoord2f (GLfloat s, GLfloat t);
void APIENTRY glTexCoord2fv (const GLfloat *v);
void APIENTRY glTexCoord2i (GLint s, GLint t);
void APIENTRY glTexCoord2iv (const GLint *v);
void APIENTRY glTexCoord2s (GLshort s, GLshort t);
void APIENTRY glTexCoord2sv (const GLshort *v);
void APIENTRY glTexCoord3d (GLdouble s, GLdouble t, GLdouble r);
void APIENTRY glTexCoord3dv (const GLdouble *v);
void APIENTRY glTexCoord3f (GLfloat s, GLfloat t, GLfloat r);
void APIENTRY glTexCoord3fv (const GLfloat *v);
void APIENTRY glTexCoord3i (GLint s, GLint t, GLint r);
void APIENTRY glTexCoord3iv (const GLint *v);
void APIENTRY glTexCoord3s (GLshort s, GLshort t, GLshort r);
void APIENTRY glTexCoord3sv (const GLshort *v);
void APIENTRY glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void APIENTRY glTexCoord4dv (const GLdouble *v);
void APIENTRY glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void APIENTRY glTexCoord4fv (const GLfloat *v);
void APIENTRY glTexCoord4i (GLint s, GLint t, GLint r, GLint q);
void APIENTRY glTexCoord4iv (const GLint *v);
void APIENTRY glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q);
void APIENTRY glTexCoord4sv (const GLshort *v);
void APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param);
void APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params);
void APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param);
void APIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params);
void APIENTRY glTexGend (GLenum coord, GLenum pname, GLdouble param);
void APIENTRY glTexGendv (GLenum coord, GLenum pname, const GLdouble *params);
void APIENTRY glTexGenf (GLenum coord, GLenum pname, GLfloat param);
void APIENTRY glTexGenfv (GLenum coord, GLenum pname, const GLfloat *params);
void APIENTRY glTexGeni (GLenum coord, GLenum pname, GLint param);
void APIENTRY glTexGeniv (GLenum coord, GLenum pname, const GLint *params);
void APIENTRY glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param);
void APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
void APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param);
void APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
void APIENTRY glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY glTranslated (GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glVertex2d (GLdouble x, GLdouble y);
void APIENTRY glVertex2dv (const GLdouble *v);
void APIENTRY glVertex2f (GLfloat x, GLfloat y);
void APIENTRY glVertex2fv (const GLfloat *v);
void APIENTRY glVertex2i (GLint x, GLint y);
void APIENTRY glVertex2iv (const GLint *v);
void APIENTRY glVertex2s (GLshort x, GLshort y);
void APIENTRY glVertex2sv (const GLshort *v);
void APIENTRY glVertex3d (GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glVertex3dv (const GLdouble *v);
void APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glVertex3fv (const GLfloat *v);
void APIENTRY glVertex3i (GLint x, GLint y, GLint z);
void APIENTRY glVertex3iv (const GLint *v);
void APIENTRY glVertex3s (GLshort x, GLshort y, GLshort z);
void APIENTRY glVertex3sv (const GLshort *v);
void APIENTRY glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void APIENTRY glVertex4dv (const GLdouble *v);
void APIENTRY glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void APIENTRY glVertex4fv (const GLfloat *v);
void APIENTRY glVertex4i (GLint x, GLint y, GLint z, GLint w);
void APIENTRY glVertex4iv (const GLint *v);
void APIENTRY glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w);
void APIENTRY glVertex4sv (const GLshort *v);
void APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height);
int   APIENTRY wglChoosePixelFormat       (HDC a, CONST PIXELFORMATDESCRIPTOR *b);
BOOL  APIENTRY wglCopyContext             (HGLRC a, HGLRC b, UINT c);
HGLRC APIENTRY wglCreateContext           (HDC a);
HGLRC APIENTRY wglCreateLayerContext      (HDC a, int b);
BOOL  APIENTRY wglDeleteContext           (HGLRC a);
BOOL  APIENTRY wglDescribeLayerPlane      (HDC a, int b, int c, UINT d, LPLAYERPLANEDESCRIPTOR e);
int   APIENTRY wglDescribePixelFormat     (HDC a, int b, UINT c, LPPIXELFORMATDESCRIPTOR d);
HGLRC APIENTRY wglGetCurrentContext       (void);
HDC   APIENTRY wglGetCurrentDC            (void);
PROC  APIENTRY wglGetDefaultProcAddress   (LPCSTR a);
int   APIENTRY wglGetLayerPaletteEntries  (HDC a, int b, int c, int d, COLORREF *e);
int   APIENTRY wglGetPixelFormat          (HDC a);
PROC  APIENTRY wglGetProcAddress          (LPCSTR a);
BOOL  APIENTRY wglMakeCurrent             (HDC a, HGLRC b);
BOOL  APIENTRY wglRealizeLayerPalette     (HDC a, int b, BOOL c);
int   APIENTRY wglSetLayerPaletteEntries  (HDC a, int b, int c, int d, CONST COLORREF *e);
BOOL  APIENTRY wglSetPixelFormat          (HDC a, int b, CONST PIXELFORMATDESCRIPTOR *c);
BOOL  APIENTRY wglShareLists              (HGLRC a, HGLRC b);
BOOL  APIENTRY wglSwapBuffers             (HDC a);
BOOL  APIENTRY wglSwapLayerBuffers        (HDC a, UINT b);
BOOL  APIENTRY wglUseFontBitmapsA         (HDC a, DWORD b, DWORD c, DWORD d);
BOOL  APIENTRY wglUseFontBitmapsW         (HDC a, DWORD b, DWORD c, DWORD d);
BOOL  APIENTRY wglUseFontOutlinesA        (HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);
BOOL  APIENTRY wglUseFontOutlinesW        (HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);

const GLubyte* APIENTRY gluErrorString 	  (GLenum   errCode);
int   APIENTRY gluBuild2DMipmaps 		  (GLenum target, GLint components, GLint width, GLint height, GLenum format, GLenum type, const void  *data);
}

bool g_initialized = false;

//
// OpenGL function handlers
//

void APIENTRY glAccum (GLenum op, GLfloat value)
{
   GLV.glAccum (op,value);
}

void APIENTRY glAlphaFunc (GLenum func, GLclampf ref)
{
   GLV.glAlphaFunc (func, ref);
}

GLboolean APIENTRY glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences)
{
   GLboolean result = GLV.glAreTexturesResident(n,textures,residences);
   return result;
}

void APIENTRY glArrayElement (GLint i)
{
   GLV.glArrayElement (i);
}

void APIENTRY glBegin (GLenum mode)
{
   GLV.glBegin (mode);
}

void APIENTRY glBindTexture (GLenum target, GLuint texture)
{
   GLV.glBindTexture (target, texture);
}

void APIENTRY glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
   GLV.glBitmap (width, height, xorig, yorig, xmove, ymove, bitmap);
}

void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor)
{
   GLV.glBlendFunc (sfactor, dfactor);
}

void APIENTRY glCallList (GLuint list)
{
   GLV.glCallList (list);
}

void APIENTRY glCallLists (GLsizei n, GLenum type, const GLvoid *lists)
{
   GLV.glCallLists (n, type, lists);
}

void APIENTRY glClear (GLbitfield mask)
{
   GLV.glClear (mask);
}

void APIENTRY glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   GLV.glClearAccum (red, green, blue, alpha);
}

void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   GLV.glClearColor (red, green, blue, alpha);
}

void APIENTRY glClearDepth (GLclampd depth)
{
   GLV.glClearDepth (depth);
}

void APIENTRY glClearIndex (GLfloat c)
{
   GLV.glClearIndex (c);
}

void APIENTRY glClearStencil (GLint s)
{
   GLV.glClearStencil (s);
}

void APIENTRY glClipPlane (GLenum plane, const GLdouble *equation)
{
   GLV.glClipPlane (plane, equation);
}

void APIENTRY glColor3b (GLbyte red, GLbyte green, GLbyte blue)
{
   GLV.glColor3b (red, green, blue);
}

void APIENTRY glColor3bv (const GLbyte *v)
{
   GLV.glColor3bv (v);
}

void APIENTRY glColor3d (GLdouble red, GLdouble green, GLdouble blue)
{
   GLV.glColor3d (red, green, blue);
}

void APIENTRY glColor3dv (const GLdouble *v)
{
   GLV.glColor3dv (v);
}

void APIENTRY glColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
   GLV.glColor3f (red, green, blue);
}

void APIENTRY glColor3fv (const GLfloat *v)
{
   GLV.glColor3fv (v);
}

void APIENTRY glColor3i (GLint red, GLint green, GLint blue)
{
   GLV.glColor3i (red, green, blue);
}

void APIENTRY glColor3iv (const GLint *v)
{
   GLV.glColor3iv (v);
}

void APIENTRY glColor3s (GLshort red, GLshort green, GLshort blue)
{
   GLV.glColor3s (red, green, blue);
}

void APIENTRY glColor3sv (const GLshort *v)
{
   GLV.glColor3sv (v);
}

void APIENTRY glColor3ub (GLubyte red, GLubyte green, GLubyte blue)
{
   GLV.glColor3ub (red, green, blue);
}

void APIENTRY glColor3ubv (const GLubyte *v)
{
   GLV.glColor3ubv (v);
}

void APIENTRY glColor3ui (GLuint red, GLuint green, GLuint blue)
{
   GLV.glColor3ui (red, green, blue);
}

void APIENTRY glColor3uiv (const GLuint *v)
{
   GLV.glColor3uiv (v);
}

void APIENTRY glColor3us (GLushort red, GLushort green, GLushort blue)
{
   GLV.glColor3us (red, green, blue);
}

void APIENTRY glColor3usv (const GLushort *v)
{
   GLV.glColor3usv (v);
}

void APIENTRY glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
   GLV.glColor4b (red, green, blue, alpha);
}

void APIENTRY glColor4bv (const GLbyte *v)
{
   GLV.glColor4bv (v);
}

void APIENTRY glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
   GLV.glColor4d (red, green, blue, alpha);
}

void APIENTRY glColor4dv (const GLdouble *v)
{
   GLV.glColor4dv (v);
}

void APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   GLV.glColor4f (red, green, blue, alpha);
}
                                    
void APIENTRY glColor4fv (const GLfloat *v)
{
   GLV.glColor4fv (v);
}

void APIENTRY glColor4i (GLint red, GLint green, GLint blue, GLint alpha)
{
   GLV.glColor4i (red, green, blue, alpha);
}

void APIENTRY glColor4iv (const GLint *v)
{                  
   GLV.glColor4iv (v);
}

void APIENTRY glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
   GLV.glColor4s (red, green, blue, alpha);
}

void APIENTRY glColor4sv (const GLshort *v)
{
   GLV.glColor4sv (v);
}

void APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   GLV.glColor4ub (red, green, blue, alpha);
}

void APIENTRY glColor4ubv (const GLubyte *v)
{
   GLV.glColor4ubv (v);
}

void APIENTRY glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
   GLV.glColor4ui (red, green, blue, alpha);
}

void APIENTRY glColor4uiv (const GLuint *v)
{
   GLV.glColor4uiv (v);
}

void APIENTRY glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
   GLV.glColor4us (red, green, blue, alpha);
}

void APIENTRY glColor4usv (const GLushort *v)
{
   GLV.glColor4usv (v);
}

void APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
   GLV.glColorMask (red, green, blue, alpha);
}

void APIENTRY glColorMaterial (GLenum face, GLenum mode)
{
   GLV.glColorMaterial (face, mode);
}

void APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLV.glColorPointer (size, type, stride, pointer);
}

void APIENTRY glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
   GLV.glCopyPixels (x, y, width, height, type);
}

void APIENTRY glCopyTexImage1D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
   GLV.glCopyTexImage1D (target, level, internalFormat, x, y, width, border);
}

void APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
   GLV.glCopyTexImage2D (target, level, internalFormat, x, y, width, height, border);
}

void APIENTRY glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
   GLV.glCopyTexSubImage1D (target, level, xoffset, x, y, width);
}

void APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLV.glCopyTexSubImage2D (target, level, xoffset, yoffset, x, y, width, height);
}

void APIENTRY glCullFace (GLenum mode)
{
   GLV.glCullFace (mode);
}

void APIENTRY glDeleteLists (GLuint list, GLsizei range)
{
   GLV.glDeleteLists (list, range);
}

void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures)
{
   GLV.glDeleteTextures (n, textures);
}

void APIENTRY glDepthFunc (GLenum func)
{
   GLV.glDepthFunc (func);
}

void APIENTRY glDepthMask (GLboolean flag)
{
   GLV.glDepthMask (flag);
}

void APIENTRY glDepthRange (GLclampd zNear, GLclampd zFar)
{
   GLV.glDepthRange (zNear, zFar);
}

void APIENTRY glDisable (GLenum cap)
{
   GLV.glDisable (cap);
}

void APIENTRY glDisableClientState (GLenum array)
{
   GLV.glDisableClientState (array);
}

void APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
   GLV.glDrawArrays (mode, first, count);
}

void APIENTRY glDrawBuffer (GLenum mode)
{
   GLV.glDrawBuffer (mode);
}

void APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
   GLV.glDrawElements (mode, count, type, indices);
}

void APIENTRY glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
   GLV.glDrawPixels (width, height, format, type, pixels);
}

void APIENTRY glEdgeFlag (GLboolean flag)
{
   GLV.glEdgeFlag (flag);
}

void APIENTRY glEdgeFlagPointer (GLsizei stride, const GLvoid *pointer)
{
   GLV.glEdgeFlagPointer (stride, pointer);
}

void APIENTRY glEdgeFlagv (const GLboolean *flag)
{
   GLV.glEdgeFlagv (flag);
}

void APIENTRY glEnable (GLenum cap)
{
   GLV.glEnable (cap);
}

void APIENTRY glEnableClientState (GLenum array)
{
   GLV.glEnableClientState (array);
}

void APIENTRY glEnd ()
{
   GLV.glEnd();
}

void APIENTRY glEndList ()
{
   GLV.glEndList();;
}

void APIENTRY glEvalCoord1d (GLdouble u)
{
   GLV.glEvalCoord1d (u);
}

void APIENTRY glEvalCoord1dv (const GLdouble *u)
{
   GLV.glEvalCoord1dv (u);
}

void APIENTRY glEvalCoord1f (GLfloat u)
{
   GLV.glEvalCoord1f (u);
}

void APIENTRY glEvalCoord1fv (const GLfloat *u)
{
   GLV.glEvalCoord1fv (u);
}

void APIENTRY glEvalCoord2d (GLdouble u, GLdouble v)
{
   GLV.glEvalCoord2d (u,v);
}

void APIENTRY glEvalCoord2dv (const GLdouble *u)
{
   GLV.glEvalCoord2dv (u);
}

void APIENTRY glEvalCoord2f (GLfloat u, GLfloat v)
{
   GLV.glEvalCoord2f (u,v);
}

void APIENTRY glEvalCoord2fv (const GLfloat *u)
{
   GLV.glEvalCoord2fv (u);
}

void APIENTRY glEvalMesh1 (GLenum mode, GLint i1, GLint i2)
{
   GLV.glEvalMesh1 (mode, i1, i2);
}

void APIENTRY glEvalMesh2 (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
   GLV.glEvalMesh2 (mode, i1, i2, j1, j2);
}

void APIENTRY glEvalPoint1 (GLint i)
{
   GLV.glEvalPoint1 (i);
}

void APIENTRY glEvalPoint2 (GLint i, GLint j)
{
   GLV.glEvalPoint2 (i, j);
}

void APIENTRY glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer)
{
   GLV.glFeedbackBuffer (size, type, buffer);
}

void APIENTRY glFinish ()
{
   GLV.glFinish();
}

void APIENTRY glFlush ()
{
   GLV.glFlush();
}

void APIENTRY glFogf (GLenum pname, GLfloat param)
{
   GLV.glFogf (pname, param);
}

void APIENTRY glFogfv (GLenum pname, const GLfloat *params)
{
   GLV.glFogfv (pname, params);
}

void APIENTRY glFogi (GLenum pname, GLint param)
{
   GLV.glFogi (pname, param);
}

void APIENTRY glFogiv (GLenum pname, const GLint *params)
{
   GLV.glFogiv (pname, params);
}

void APIENTRY glFrontFace (GLenum mode)
{
   GLV.glFrontFace (mode);
}

void APIENTRY glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
   GLV.glFrustum (left, right, bottom, top, zNear, zFar);
}

GLuint APIENTRY glGenLists (GLsizei range)
{
   GLuint result = GLV.glGenLists(range);
   return result;
}

void APIENTRY glGenTextures (GLsizei n, GLuint *textures)
{
   GLV.glGenTextures (n, textures);
}

void APIENTRY glGetBooleanv (GLenum pname, GLboolean *params)
{
   GLV.glGetBooleanv (pname, params);
}

void APIENTRY glGetClipPlane (GLenum plane, GLdouble *equation)
{
   GLV.glGetClipPlane (plane, equation);
}

void APIENTRY glGetDoublev (GLenum pname, GLdouble *params)
{
   GLV.glGetDoublev (pname, params);
}

GLenum APIENTRY glGetError ()
{
   GLenum result = GLV.glGetError();
   return result;
}

void APIENTRY glGetFloatv (GLenum pname, GLfloat *params)
{
   GLV.glGetFloatv (pname, params);
}

void APIENTRY glGetIntegerv (GLenum pname, GLint *params)
{
   GLV.glGetIntegerv (pname, params);
}

void APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params)
{
   GLV.glGetLightfv (light, pname, params);
}

void APIENTRY glGetLightiv (GLenum light, GLenum pname, GLint *params)
{
   GLV.glGetLightiv (light, pname, params);
}

void APIENTRY glGetMapdv (GLenum target, GLenum query, GLdouble *v)
{
   GLV.glGetMapdv (target, query, v);
}

void APIENTRY glGetMapfv (GLenum target, GLenum query, GLfloat *v)
{
   GLV.glGetMapfv (target, query, v);
}

void APIENTRY glGetMapiv (GLenum target, GLenum query, GLint *v)
{
   GLV.glGetMapiv (target, query, v);
}

void APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params)
{
   GLV.glGetMaterialfv (face, pname, params);
}

void APIENTRY glGetMaterialiv (GLenum face, GLenum pname, GLint *params)
{
   GLV.glGetMaterialiv (face, pname, params);
}

void APIENTRY glGetPixelMapfv (GLenum map, GLfloat *values)
{
   GLV.glGetPixelMapfv (map, values);
}

void APIENTRY glGetPixelMapuiv (GLenum map, GLuint *values)
{
   GLV.glGetPixelMapuiv (map, values);
}

void APIENTRY glGetPixelMapusv (GLenum map, GLushort *values)
{
   GLV.glGetPixelMapusv (map, values);
}

void APIENTRY glGetPointerv (GLenum pname, GLvoid* *params)
{
   GLV.glGetPointerv (pname, params);
}

void APIENTRY glGetPolygonStipple (GLubyte *mask)
{
   GLV.glGetPolygonStipple (mask);
}

const GLstring APIENTRY glGetString (GLenum name)
{
   const GLstring result = GLV.glGetString(name);
   return result;
}

void APIENTRY glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params)
{
   GLV.glGetTexEnvfv (target, pname, params);
}

void APIENTRY glGetTexEnviv (GLenum target, GLenum pname, GLint *params)
{
   GLV.glGetTexEnviv (target, pname, params);
}

void APIENTRY glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params)
{
   GLV.glGetTexGendv (coord, pname, params);
}

void APIENTRY glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params)
{
   GLV.glGetTexGenfv (coord, pname, params);
}

void APIENTRY glGetTexGeniv (GLenum coord, GLenum pname, GLint *params)
{
   GLV.glGetTexGeniv (coord, pname, params);
}

void APIENTRY glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
   GLV.glGetTexImage (target, level, format, type, pixels);
}

void APIENTRY glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params)
{
   GLV.glGetTexLevelParameterfv (target, level, pname, params);
}

void APIENTRY glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params)
{
   GLV.glGetTexLevelParameteriv (target, level, pname, params);
}

void APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params)
{
   GLV.glGetTexParameterfv (target, pname, params);
}

void APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params)
{
   GLV.glGetTexParameteriv (target, pname, params);
}

void APIENTRY glHint (GLenum target, GLenum mode)
{
   GLV.glHint (target, mode);
}

void APIENTRY glIndexMask (GLuint mask)
{
   GLV.glIndexMask (mask);
}

void APIENTRY glIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLV.glIndexPointer (type, stride, pointer);
}

void APIENTRY glIndexd (GLdouble c)
{
   GLV.glIndexd (c);
}

void APIENTRY glIndexdv (const GLdouble *c)
{
   GLV.glIndexdv (c);
}

void APIENTRY glIndexf (GLfloat c)
{
   GLV.glIndexf (c);
}

void APIENTRY glIndexfv (const GLfloat *c)
{
   GLV.glIndexfv (c);
}

void APIENTRY glIndexi (GLint c)
{
   GLV.glIndexi (c);
}

void APIENTRY glIndexiv (const GLint *c)
{
   GLV.glIndexiv (c);
}

void APIENTRY glIndexs (GLshort c)
{
   GLV.glIndexs (c);
}

void APIENTRY glIndexsv (const GLshort *c)
{
   GLV.glIndexsv (c);
}

void APIENTRY glIndexub (GLubyte c)
{
   GLV.glIndexub (c);
}

void APIENTRY glIndexubv (const GLubyte *c)
{
   GLV.glIndexubv (c);
}

void APIENTRY glInitNames ()
{
   GLV.glInitNames();
}

void APIENTRY glInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer)
{
   GLV.glInterleavedArrays (format, stride, pointer);
}

GLboolean APIENTRY glIsEnabled (GLenum cap)
{
   GLboolean result = GLV.glIsEnabled(cap);
   return result;
}

GLboolean APIENTRY glIsList (GLuint list)
{
   GLboolean result = GLV.glIsList(list);
   return result;
}

GLboolean APIENTRY glIsTexture (GLuint texture)
{
   GLboolean result = GLV.glIsTexture(texture);
   return result;
}

void APIENTRY glLightModelf (GLenum pname, GLfloat param)
{
   GLV.glLightModelf (pname, param);
}

void APIENTRY glLightModelfv (GLenum pname, const GLfloat *params)
{
   GLV.glLightModelfv (pname,params);
}

void APIENTRY glLightModeli (GLenum pname, GLint param)
{
   GLV.glLightModeli (pname, param);
}

void APIENTRY glLightModeliv (GLenum pname, const GLint *params)
{
   GLV.glLightModeliv (pname,params);
}

void APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param)
{
   GLV.glLightf (light, pname, param);
}

void APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params)
{
   GLV.glLightfv (light, pname, params);
}

void APIENTRY glLighti (GLenum light, GLenum pname, GLint param)
{
   GLV.glLighti (light, pname, param);
}

void APIENTRY glLightiv (GLenum light, GLenum pname, const GLint *params)
{
   GLV.glLightiv (light, pname, params);
}

void APIENTRY glLineStipple (GLint factor, GLushort pattern)
{
   GLV.glLineStipple (factor, pattern);
}

void APIENTRY glLineWidth (GLfloat width)
{
   GLV.glLineWidth (width);
}

void APIENTRY glListBase (GLuint base)
{
   GLV.glListBase (base);
}

void APIENTRY glLoadIdentity ()
{
   GLV.glLoadIdentity();
}

void APIENTRY glLoadMatrixd (const GLdouble *m)
{
   GLV.glLoadMatrixd(m);
}

void APIENTRY glLoadMatrixf (const GLfloat *m)
{
   GLV.glLoadMatrixf (m);
}

void APIENTRY glLoadName (GLuint name)
{
   GLV.glLoadName (name);
}

void APIENTRY glLogicOp (GLenum opcode)
{
   GLV.glLogicOp (opcode);
}

void APIENTRY glMap1d (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
   GLV.glMap1d (target, u1, u2, stride, order, points);
}

void APIENTRY glMap1f (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
   GLV.glMap1f (target, u1, u2, stride, order, points);
}

void APIENTRY glMap2d (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
   GLV.glMap2d (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

void APIENTRY glMap2f (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
   GLV.glMap2f (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

void APIENTRY glMapGrid1d (GLint un, GLdouble u1, GLdouble u2)
{
   GLV.glMapGrid1d (un, u1, u2);
}

void APIENTRY glMapGrid1f (GLint un, GLfloat u1, GLfloat u2)
{
   GLV.glMapGrid1f (un, u1, u2);
}

void APIENTRY glMapGrid2d (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
   GLV.glMapGrid2d (un, u1, u2, vn, v1, v2);
}

void APIENTRY glMapGrid2f (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
   GLV.glMapGrid2f (un, u1, u2, vn, v1, v2);
}

void APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param)
{
   GLV.glMaterialf (face, pname, param);
}

void APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params)
{
   GLV.glMaterialfv (face, pname, params);
}

void APIENTRY glMateriali (GLenum face, GLenum pname, GLint param)
{
   GLV.glMateriali (face, pname, param);
}

void APIENTRY glMaterialiv (GLenum face, GLenum pname, const GLint *params)
{
   GLV.glMaterialiv (face, pname, params);
}

void APIENTRY glMatrixMode (GLenum mode)
{
   GLV.glMatrixMode (mode);
}

void APIENTRY glMultMatrixd (const GLdouble *m)
{
   GLV.glMultMatrixd (m);
}

void APIENTRY glMultMatrixf (const GLfloat *m)
{
   GLV.glMultMatrixf (m);
}

void APIENTRY glNewList (GLuint list, GLenum mode)
{
   GLV.glNewList (list,mode);
}

void APIENTRY glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz)
{
   GLV.glNormal3b (nx, ny, nz);
}

void APIENTRY glNormal3bv (const GLbyte *v)
{
   GLV.glNormal3bv (v);
}

void APIENTRY glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz)
{
   GLV.glNormal3d (nx, ny, nz);
}

void APIENTRY glNormal3dv (const GLdouble *v)
{
   GLV.glNormal3dv (v);
}

void APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz)
{
   GLV.glNormal3f (nx, ny, nz);
}

void APIENTRY glNormal3fv (const GLfloat *v)
{
   GLV.glNormal3fv (v);
}

void APIENTRY glNormal3i (GLint nx, GLint ny, GLint nz)
{
   GLV.glNormal3i (nx, ny, nz);
}

void APIENTRY glNormal3iv (const GLint *v)
{
   GLV.glNormal3iv (v);
}

void APIENTRY glNormal3s (GLshort nx, GLshort ny, GLshort nz)
{
   GLV.glNormal3s (nx, ny, nz);
}

void APIENTRY glNormal3sv (const GLshort *v)
{
   GLV.glNormal3sv (v);
}

void APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLV.glNormalPointer (type, stride, pointer);
}

void APIENTRY glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
   GLV.glOrtho (left, right, bottom, top, zNear, zFar);
}

void APIENTRY glPassThrough (GLfloat token)
{
   GLV.glPassThrough (token);
}

void APIENTRY glPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat *values)
{
   GLV.glPixelMapfv (map, mapsize, values);
}

void APIENTRY glPixelMapuiv (GLenum map, GLsizei mapsize, const GLuint *values)
{
   GLV.glPixelMapuiv (map, mapsize, values);
}

void APIENTRY glPixelMapusv (GLenum map, GLsizei mapsize, const GLushort *values)
{
   GLV.glPixelMapusv (map, mapsize, values);
}

void APIENTRY glPixelStoref (GLenum pname, GLfloat param)
{
   GLV.glPixelStoref (pname, param);
}

void APIENTRY glPixelStorei (GLenum pname, GLint param)
{
   GLV.glPixelStorei (pname, param);
}

void APIENTRY glPixelTransferf (GLenum pname, GLfloat param)
{
   GLV.glPixelTransferf (pname, param);
}

void APIENTRY glPixelTransferi (GLenum pname, GLint param)
{
   GLV.glPixelTransferi (pname, param);
}

void APIENTRY glPixelZoom (GLfloat xfactor, GLfloat yfactor)
{
   GLV.glPixelZoom (xfactor, yfactor);
}

void APIENTRY glPointSize (GLfloat size)
{
   GLV.glPointSize (size);
}

void APIENTRY glPolygonMode (GLenum face, GLenum mode)
{
   GLV.glPolygonMode (face, mode);
}

void APIENTRY glPolygonOffset (GLfloat factor, GLfloat units)
{
   GLV.glPolygonOffset (factor, units);
}

void APIENTRY glPolygonStipple (const GLubyte *mask)
{
   GLV.glPolygonStipple (mask);
}

void APIENTRY glPopAttrib ()
{
   GLV.glPopAttrib();
}

void APIENTRY glPopClientAttrib ()
{
   GLV.glPopClientAttrib();
}

void APIENTRY glPopMatrix ()
{
   GLV.glPopMatrix();
}

void APIENTRY glPopName ()
{
   GLV.glPopName();
}

void APIENTRY glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
   GLV.glPrioritizeTextures (n, textures, priorities);
}

void APIENTRY glPushAttrib (GLbitfield mask)
{
   GLV.glPushAttrib (mask);
}

void APIENTRY glPushClientAttrib (GLbitfield mask)
{
   GLV.glPushClientAttrib (mask);
}

void APIENTRY glPushMatrix ()
{
   GLV.glPushMatrix();
}

void APIENTRY glPushName (GLuint name)
{
   GLV.glPushName (name);
}

void APIENTRY glRasterPos2d (GLdouble x, GLdouble y)
{
   GLV.glRasterPos2d (x, y);
}

void APIENTRY glRasterPos2dv (const GLdouble *v)
{
   GLV.glRasterPos2dv (v);
}

void APIENTRY glRasterPos2f (GLfloat x, GLfloat y)
{
   GLV.glRasterPos2f (x, y);
}

void APIENTRY glRasterPos2fv (const GLfloat *v)
{
   GLV.glRasterPos2fv (v);
}

void APIENTRY glRasterPos2i (GLint x, GLint y)
{
   GLV.glRasterPos2i (x, y);
}

void APIENTRY glRasterPos2iv (const GLint *v)
{
   GLV.glRasterPos2iv (v);
}

void APIENTRY glRasterPos2s (GLshort x, GLshort y)
{
   GLV.glRasterPos2s (x, y);
}

void APIENTRY glRasterPos2sv (const GLshort *v)
{
   GLV.glRasterPos2sv (v);
}

void APIENTRY glRasterPos3d (GLdouble x, GLdouble y, GLdouble z)
{
   GLV.glRasterPos3d (x, y, z);
}

void APIENTRY glRasterPos3dv (const GLdouble *v)
{
   GLV.glRasterPos3dv (v);
}

void APIENTRY glRasterPos3f (GLfloat x, GLfloat y, GLfloat z)
{
   GLV.glRasterPos3f (x, y, z);
}

void APIENTRY glRasterPos3fv (const GLfloat *v)
{
   GLV.glRasterPos3fv (v);
}

void APIENTRY glRasterPos3i (GLint x, GLint y, GLint z)
{
   GLV.glRasterPos3i (x, y, z);
}

void APIENTRY glRasterPos3iv (const GLint *v)
{
   GLV.glRasterPos3iv (v);
}

void APIENTRY glRasterPos3s (GLshort x, GLshort y, GLshort z)
{
   GLV.glRasterPos3s (x, y, z);
}

void APIENTRY glRasterPos3sv (const GLshort *v)
{
   GLV.glRasterPos3sv (v);
}

void APIENTRY glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   GLV.glRasterPos4d (x, y, z, w);
}

void APIENTRY glRasterPos4dv (const GLdouble *v)
{
   GLV.glRasterPos4dv (v);
}

void APIENTRY glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLV.glRasterPos4f (x, y, z, w);
}

void APIENTRY glRasterPos4fv (const GLfloat *v)
{
   GLV.glRasterPos4fv (v);
}

void APIENTRY glRasterPos4i (GLint x, GLint y, GLint z, GLint w)
{
   GLV.glRasterPos4i (x, y, z, w);
}

void APIENTRY glRasterPos4iv (const GLint *v)
{
   GLV.glRasterPos4iv (v);
}

void APIENTRY glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w)
{
   GLV.glRasterPos4s (x, y, z, w);
}

void APIENTRY glRasterPos4sv (const GLshort *v)
{
   GLV.glRasterPos4sv (v);
}

void APIENTRY glReadBuffer (GLenum mode)
{
   GLV.glReadBuffer (mode);
}

void APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
   GLV.glReadPixels (x, y, width, height, format, type, pixels);
}

void APIENTRY glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
   GLV.glRectd (x1, y1, x2, y2);
}

void APIENTRY glRectdv (const GLdouble *v1, const GLdouble *v2)
{
   GLV.glRectdv (v1, v2);
}

void APIENTRY glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
   GLV.glRectf (x1, y1, x2, y2);
}

void APIENTRY glRectfv (const GLfloat *v1, const GLfloat *v2)
{
   GLV.glRectfv (v1, v2);
}

void APIENTRY glRecti (GLint x1, GLint y1, GLint x2, GLint y2)
{
   GLV.glRecti (x1, y1, x2, y2);
}

void APIENTRY glRectiv (const GLint *v1, const GLint *v2)
{
   GLV.glRectiv (v1, v2);
}

void APIENTRY glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
   GLV.glRects (x1, y1, x2, y2);
}

void APIENTRY glRectsv (const GLshort *v1, const GLshort *v2)
{
   GLV.glRectsv (v1, v2);
}

GLint APIENTRY glRenderMode (GLenum mode)
{
   GLint result = GLV.glRenderMode(mode);
   return result;
}

void APIENTRY glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
   GLV.glRotated (angle, x, y, z);
}

void APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   GLV.glRotatef (angle, x, y, z);
}

void APIENTRY glScaled (GLdouble x, GLdouble y, GLdouble z)
{
   GLV.glScaled (x, y, z);
}

void APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z)
{
   GLV.glScalef (x, y, z);
}

void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLV.glScissor (x, y, width, height);
}

void APIENTRY glSelectBuffer (GLsizei size, GLuint *buffer)
{
   GLV.glSelectBuffer (size, buffer);
}

void APIENTRY glShadeModel (GLenum mode)
{
   GLV.glShadeModel (mode);
}

void APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask)
{
   GLV.glStencilFunc (func, ref, mask);
}

void APIENTRY glStencilMask (GLuint mask)
{
   GLV.glStencilMask (mask);
}

void APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass)
{
   GLV.glStencilOp (fail, zfail, zpass);
}

void APIENTRY glTexCoord1d (GLdouble s)
{
   GLV.glTexCoord1d (s);
}

void APIENTRY glTexCoord1dv (const GLdouble *v)
{
   GLV.glTexCoord1dv (v);
}

void APIENTRY glTexCoord1f (GLfloat s)
{
   GLV.glTexCoord1f (s);
}

void APIENTRY glTexCoord1fv (const GLfloat *v)
{
   GLV.glTexCoord1fv (v);
}

void APIENTRY glTexCoord1i (GLint s)
{
   GLV.glTexCoord1i (s);
}

void APIENTRY glTexCoord1iv (const GLint *v)
{
   GLV.glTexCoord1iv (v);
}

void APIENTRY glTexCoord1s (GLshort s)
{
   GLV.glTexCoord1s (s);
}

void APIENTRY glTexCoord1sv (const GLshort *v)
{
   GLV.glTexCoord1sv (v);
}

void APIENTRY glTexCoord2d (GLdouble s, GLdouble t)
{
   GLV.glTexCoord2d (s, t);
}

void APIENTRY glTexCoord2dv (const GLdouble *v)
{
   GLV.glTexCoord2dv (v);
}

void APIENTRY glTexCoord2f (GLfloat s, GLfloat t)
{
   GLV.glTexCoord2f (s, t);
}

void APIENTRY glTexCoord2fv (const GLfloat *v)
{
   GLV.glTexCoord2fv (v);
}

void APIENTRY glTexCoord2i (GLint s, GLint t)
{
   GLV.glTexCoord2i (s, t);
}

void APIENTRY glTexCoord2iv (const GLint *v)
{
   GLV.glTexCoord2iv (v);
}

void APIENTRY glTexCoord2s (GLshort s, GLshort t)
{
   GLV.glTexCoord2s (s, t);
}

void APIENTRY glTexCoord2sv (const GLshort *v)
{
   GLV.glTexCoord2sv (v);
}

void APIENTRY glTexCoord3d (GLdouble s, GLdouble t, GLdouble r)
{
   GLV.glTexCoord3d (s, t, r);
}

void APIENTRY glTexCoord3dv (const GLdouble *v)
{
   GLV.glTexCoord3dv (v);
}

void APIENTRY glTexCoord3f (GLfloat s, GLfloat t, GLfloat r)
{
   GLV.glTexCoord3f (s, t, r);
}

void APIENTRY glTexCoord3fv (const GLfloat *v)
{
   GLV.glTexCoord3fv (v);
}

void APIENTRY glTexCoord3i (GLint s, GLint t, GLint r)
{
   GLV.glTexCoord3i (s, t, r);
}

void APIENTRY glTexCoord3iv (const GLint *v)
{
   GLV.glTexCoord3iv (v);
}

void APIENTRY glTexCoord3s (GLshort s, GLshort t, GLshort r)
{
   GLV.glTexCoord3s (s, t, r);
}

void APIENTRY glTexCoord3sv (const GLshort *v)
{
   GLV.glTexCoord3sv (v);
}

void APIENTRY glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
   GLV.glTexCoord4d (s, t, r, q);
}

void APIENTRY glTexCoord4dv (const GLdouble *v)
{
   GLV.glTexCoord4dv (v);
}

void APIENTRY glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   GLV.glTexCoord4f (s, t, r, q);
}

void APIENTRY glTexCoord4fv (const GLfloat *v)
{
   GLV.glTexCoord4fv (v);
}

void APIENTRY glTexCoord4i (GLint s, GLint t, GLint r, GLint q)
{
   GLV.glTexCoord4i (s, t, r, q);
}

void APIENTRY glTexCoord4iv (const GLint *v)
{
   GLV.glTexCoord4iv (v);
}

void APIENTRY glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q)
{
   GLV.glTexCoord4s (s, t, r, q);
}

void APIENTRY glTexCoord4sv (const GLshort *v)
{
   GLV.glTexCoord4sv (v);
}

void APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLV.glTexCoordPointer (size, type, stride, pointer);
}

void APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param)
{
   GLV.glTexEnvf (target, pname, param);
}

void APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params)
{
   GLV.glTexEnvfv (target, pname, params);
}

void APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param)
{
   GLV.glTexEnvi (target, pname, param);
}

void APIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params)
{
   GLV.glTexEnviv (target, pname, params);
}

void APIENTRY glTexGend (GLenum coord, GLenum pname, GLdouble param)
{
   GLV.glTexGend (coord, pname, param);
}

void APIENTRY glTexGendv (GLenum coord, GLenum pname, const GLdouble *params)
{
   GLV.glTexGendv (coord, pname, params);
}

void APIENTRY glTexGenf (GLenum coord, GLenum pname, GLfloat param)
{
   GLV.glTexGenf (coord, pname, param);
}

void APIENTRY glTexGenfv (GLenum coord, GLenum pname, const GLfloat *params)
{
   GLV.glTexGenfv (coord, pname, params);
}

void APIENTRY glTexGeni (GLenum coord, GLenum pname, GLint param)
{
   GLV.glTexGeni (coord, pname, param);
}

void APIENTRY glTexGeniv (GLenum coord, GLenum pname, const GLint *params)
{
   GLV.glTexGeniv (coord, pname, params);
}

void APIENTRY glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   GLV.glTexImage1D (target, level, internalformat, width, border, format, type, pixels);
}

void APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   GLV.glTexImage2D (target, level, internalformat, width, height, border, format, type, pixels);
}

void APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
   GLV.glTexParameterf (target, pname, param);
}

void APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params)
{
   GLV.glTexParameterfv (target, pname, params);
}

void APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param)
{
   GLV.glTexParameteri (target, pname, param);
}

void APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params)
{
   GLV.glTexParameteriv (target, pname, params);
}

void APIENTRY glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
   GLV.glTexSubImage1D (target, level, xoffset, width, format, type, pixels);
}

void APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
   GLV.glTexSubImage2D (target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void APIENTRY glTranslated (GLdouble x, GLdouble y, GLdouble z)
{
   GLV.glTranslated (x, y, z);
}

void APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
   GLV.glTranslatef (x, y, z);
}

void APIENTRY glVertex2d (GLdouble x, GLdouble y)
{
   GLV.glVertex2d (x, y);
}

void APIENTRY glVertex2dv (const GLdouble *v)
{
   GLV.glVertex2dv (v);
}

void APIENTRY glVertex2f (GLfloat x, GLfloat y)
{
   GLV.glVertex2f (x, y);
}

void APIENTRY glVertex2fv (const GLfloat *v)
{
   GLV.glVertex2fv (v);
}

void APIENTRY glVertex2i (GLint x, GLint y)
{
   GLV.glVertex2i (x, y);
}

void APIENTRY glVertex2iv (const GLint *v)
{
   GLV.glVertex2iv (v);
}

void APIENTRY glVertex2s (GLshort x, GLshort y)
{
   GLV.glVertex2s (x, y);
}

void APIENTRY glVertex2sv (const GLshort *v)
{
   GLV.glVertex2sv (v);
}

void APIENTRY glVertex3d (GLdouble x, GLdouble y, GLdouble z)
{
   GLV.glVertex3d (x, y, z);
}

void APIENTRY glVertex3dv (const GLdouble *v)
{
   GLV.glVertex3dv (v);
}

void APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
   GLV.glVertex3f (x, y, z);
}

void APIENTRY glVertex3fv (const GLfloat *v)
{
   GLV.glVertex3fv (v);
}

void APIENTRY glVertex3i (GLint x, GLint y, GLint z)
{
   GLV.glVertex3i (x, y, z);
}

void APIENTRY glVertex3iv (const GLint *v)
{
   GLV.glVertex3iv (v);
}

void APIENTRY glVertex3s (GLshort x, GLshort y, GLshort z)
{
   GLV.glVertex3s (x, y, z);
}

void APIENTRY glVertex3sv (const GLshort *v)
{
   GLV.glVertex3sv (v);
}

void APIENTRY glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
   GLV.glVertex4d (x, y, z, w);
}

void APIENTRY glVertex4dv (const GLdouble *v)
{
   GLV.glVertex4dv (v);
}

void APIENTRY glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLV.glVertex4f (x, y, z, w);
}

void APIENTRY glVertex4fv (const GLfloat *v)
{
   GLV.glVertex4fv (v);
}

void APIENTRY glVertex4i (GLint x, GLint y, GLint z, GLint w)
{
   GLV.glVertex4i (x, y, z, w);
}

void APIENTRY glVertex4iv (const GLint *v)
{
   GLV.glVertex4iv (v);
}

void APIENTRY glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w)
{
   GLV.glVertex4s (x, y, z, w);
}

void APIENTRY glVertex4sv (const GLshort *v)
{
   GLV.glVertex4sv (v);
}

void APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLV.glVertexPointer (size, type, stride, pointer);
}

void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
   GLV.glViewport (x, y, width, height);
}

int   APIENTRY wglChoosePixelFormat       (HDC a, CONST PIXELFORMATDESCRIPTOR *b)
{
   int result = GLV.wglChoosePixelFormat(a,b);
   return result;
}

BOOL  APIENTRY wglCopyContext             (HGLRC a, HGLRC b, UINT c)
{
   BOOL result = GLV.wglCopyContext(a,b,c);
   return result;
}

HGLRC APIENTRY wglCreateContext           (HDC a)
{
   HGLRC result = GLV.wglCreateContext(a);
   return result;
}

HGLRC APIENTRY wglCreateLayerContext      (HDC a, int b)
{
   HGLRC result = GLV.wglCreateLayerContext(a,b);
   return result;
}

BOOL  APIENTRY wglDeleteContext           (HGLRC a)
{
   BOOL result = GLV.wglDeleteContext(a);
   return result;
}

BOOL  APIENTRY wglDescribeLayerPlane      (HDC a, int b, int c, UINT d, LPLAYERPLANEDESCRIPTOR e)
{
   BOOL result = GLV.wglDescribeLayerPlane(a,b,c,d,e);
   return result;
}

int   APIENTRY wglDescribePixelFormat     (HDC a, int b, UINT c, LPPIXELFORMATDESCRIPTOR d)
{
   int result = GLV.wglDescribePixelFormat(a,b,c,d);
   return result;
}

HGLRC APIENTRY wglGetCurrentContext       (void)
{
   HGLRC result = GLV.wglGetCurrentContext();
   return result;
}

HDC   APIENTRY wglGetCurrentDC            (void)
{
   HDC result = GLV.wglGetCurrentDC();
   return result;
}

PROC  APIENTRY wglGetDefaultProcAddress   (LPCSTR a)
{
   PROC result = GLV.wglGetDefaultProcAddress(a);
   return result;
}

int   APIENTRY wglGetLayerPaletteEntries  (HDC a, int b, int c, int d, COLORREF *e)
{
   int result = GLV.wglGetLayerPaletteEntries(a,b,c,d,e);
   return result;
}

int   APIENTRY wglGetPixelFormat          (HDC a)
{
   int result = GLV.wglGetPixelFormat(a);
   return result;
}

PROC  APIENTRY wglGetProcAddress          (LPCSTR a)
{
   PROC result = GLV.wglGetProcAddress(a);
   return result;
}

BOOL  APIENTRY wglMakeCurrent             (HDC a, HGLRC b)
{
   BOOL result = GLV.wglMakeCurrent(a,b);
   return result;
}

BOOL  APIENTRY wglRealizeLayerPalette     (HDC a, int b, BOOL c)
{
   BOOL result = GLV.wglRealizeLayerPalette(a,b,c);
   return result;
}

int   APIENTRY wglSetLayerPaletteEntries  (HDC a, int b, int c, int d, CONST COLORREF *e)
{
   int result = GLV.wglSetLayerPaletteEntries(a,b,c,d,e);
   return result;
}

BOOL  APIENTRY wglSetPixelFormat          (HDC a, int b, CONST PIXELFORMATDESCRIPTOR *c)
{
   BOOL result = GLV.wglSetPixelFormat(a,b,c);
   return result;
}

BOOL  APIENTRY wglShareLists              (HGLRC a, HGLRC b)
{
   BOOL result = GLV.wglShareLists(a,b);
   return result;
}

BOOL  APIENTRY wglSwapBuffers             (HDC a)
{
   BOOL result = GLV.wglSwapBuffers(a);
   return result;
}

BOOL  APIENTRY wglSwapLayerBuffers        (HDC a, UINT b)
{
   BOOL result = GLV.wglSwapLayerBuffers(a,b);
   return result;
}

BOOL  APIENTRY wglUseFontBitmapsA         (HDC a, DWORD b, DWORD c, DWORD d)
{
   BOOL result = GLV.wglUseFontBitmapsA(a,b,c,d);
   return result;
}

BOOL  APIENTRY wglUseFontBitmapsW         (HDC a, DWORD b, DWORD c, DWORD d)
{
   BOOL result = GLV.wglUseFontBitmapsW(a,b,c,d);
   return result;
}

BOOL  APIENTRY wglUseFontOutlinesA        (HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h)
{
   BOOL result = GLV.wglUseFontOutlinesA(a,b,c,d,e,f,g,h);
   return result;
}

BOOL  APIENTRY wglUseFontOutlinesW        (HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h)
{
   BOOL result = GLV.wglUseFontOutlinesW(a,b,c,d,e,f,g,h);
   return result;
}

const GLubyte* APIENTRY gluErrorString (GLenum a)
{
   const GLubyte *result = GLUV.gluErrorString(a);
   return result;
}

int APIENTRY gluBuild2DMipmaps (GLenum a, GLint b, GLint c, GLint d, GLenum e, GLenum f, const void *g)
{
	int result = GLUV.gluBuild2DMipmaps(a,b,c,d,e,f,g);
	return result;
}

//-------------------------------------- Revised initialization function
//
extern "C" bool
initLoader()
{
   if (g_initialized == true)
      return true;

   HINSTANCE OpenGL_provider = LoadLibrary("opengl32.dll");
   if (OpenGL_provider == NULL)
      return false;

   HINSTANCE OpenGL_utilities = LoadLibrary("glu32.dll");
   if (OpenGL_utilities == NULL)
      return false;

   //
   // Map Win32 OpenGL calls to our jump table
   //
   int i;
   for (i=0; i < sizeof(GLN)/sizeof(char*); i++) {
      FARPROC OpenGL_proc = GetProcAddress(OpenGL_provider, GLN[i]);

	  if (OpenGL_proc == NULL)
		return false;
//BWA       AssertISV(OpenGL_proc != NULL, "Error, you have an invalid OpenGL Driver.\n"
//BWA                                      "Most likely, the manufacturer has provided you\n"
//BWA                                      "with an OpenGL 1.0 .dll, we require a 1.1 .dll\n");

      ((unsigned long *) (&GLV))[i] = (unsigned long) OpenGL_proc;
   }

   for (i=0; i < sizeof(GLUN)/sizeof(char*); i++) {
      FARPROC OpenGL_proc = GetProcAddress(OpenGL_utilities, GLUN[i]);

      if (OpenGL_proc == NULL)
		return false;
//BWA       AssertISV(OpenGL_proc != NULL, "Error, you have an invalid OpenGL Driver.\n"
//BWA                                      "Most likely, the manufacturer has provided you\n"
//BWA                                      "with an OpenGL 1.0 .dll, we require a 1.1 .dll\n");

      ((unsigned long *) (&GLUV))[i] = (unsigned long) OpenGL_proc;
   }

   return true;
}

