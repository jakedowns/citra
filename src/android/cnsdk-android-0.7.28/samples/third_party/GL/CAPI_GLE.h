/*
 * Copyright (c) 2023 Leia Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// This file provides functionality similar to a reduced version of GLEW, plus some
// additional functionality that's useful to us, such as function hooking.

#ifndef OVR_CAPI_GLE_h
#define OVR_CAPI_GLE_h

//#include "Kernel/OVR_Types.h"
#include "CAPI_GLE_GL.h"

///////////////////////////////////////////////////////////////////////////////
// How to use this functionality
//
// - You #include this header instead of gl.h, glext.h, wglext.h (Windows), gl3.h (Apple), gl3ext.h
// (Apple), glx.h (Unix), and glxext.h (Unix).
//   Currently you still would #include <Windows.h> for the base wgl functions on Windows and
//   OpenGL.h or NSOpenGL for the base Apple cgl functions.
//
// - You call OpenGL functions just like you would if you were directly using OpenGL
//   headers and declarations. The difference is that this module automatically loads
//   extensions on init and so you should never need to use GetProcAddress, wglGetProcAddress, etc.
//
// - OpenGL 1.1 functions can be called unilaterally without checking if they are present,
//   as it's assumed they are always present.
//
// - In order to use an OpenGL 1.2 or later function you can check the GLEContext::WholeVersion
//   variable to tell what version of OpenGL is present and active. Example usage:
//       if(GLEContext::GetCurrentContext()->WholeVersion >= 302) // If OpenGL 3.2 or later...
//
// - In order to use an OpenGL extension, you can check the GLE_ helper macro that exists for each
//   extension. For example, in order to check of the KHR_debug is present you could do this:
//        if(GLE_KHR_debug) ...
//   You cannot check for the presence of extensions by testing the function pointer, because
//   when hooking is enabled then we aren't using function pointers and thus all functions will
//   look like they are present.
//
// - You can test if the OpenGL implementation is OpenGL ES by checking the GLEContext IsGLES
//   member variable. For example: if(GLEContext::GetCurrentContext()->IsGLES) ...
//
// - You can test if the OpenGL implementation is a core profile ES by checking the GLEContext
// IsCoreProfile
//   member variable. For example: if(GLEContext::GetCurrentContext()->IsCoreProfile) ...
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// How to add support for additional functions to this module.
//
// For an example of how to do this, search the source files for all cases of KHR_Debug and just
// copy the things that it does but for your new extension.
//
//     1) Add the appropriate extension declaration to CAPI_GLE_GL.h, preferably by
//        copying it from the standard header file it normally comes from. If it's
//        platform-specific (e.g. a Windows wgl function) then make sure it's declared
//        within the given platform section. Note that there are potentially #defines, typedefs,
//        function typedefs, and function #defines. There is always a GLE_ macro declared which
//        lets the user know at runtime whether the extension is present.
//        Note that entries are alphabetically sorted in these files.
//        e.g.  #ifndef GL_KHR_debug
//                  #define GL_KHR_debug 1
//                  #define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002 etc.
//                  typedef void (GLAPIENTRY * PFNGLPOPDEBUGGROUPPROC) ();
//                  #define glPopDebugGroup GLEGetCurrentFunction(glPopDebugGroup)
//                  #define GLE_KHR_debug GLEGetCurrentVariable(gl_KHR_debug)
//              #endif etc.
//
//     2) Add a hook function for in the hook section of the GLEContext class in this header,
//        ideally in the same order it's declared in the CAPI_GLE_GL.h so it's easily readable.
//        e.g. void glDebugMessageControl_Hook(GLenum source, GLenum type, GLenum severity, GLsizei
//        count, const GLuint* ids, GLboolean enabled); etc.
//
//     3) Add a declaration for each interface function to the GLEContext class in this header.
//        e.g. PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_Impl; etc.
//
//     4) Add code to GLEContext::InitExtensionLoad to load the function pointer.
//        e.g. GLELoadProc(glDebugMessageCallback_Impl, glDebugMessageCallback); etc.
//
//     5) Add code to GLEContext::InitExtensionSupport to detect the extension support.
//        On Mac, core profile functions aren't identified as extensions and so in addition
//        to detecting them you need to unilaterally set them as available when using 3.2+
//        by adding them to the section at the bottom of InitExtensionSupport.
//        e.g. { gl_KHR_debug, "GL_KHR_debug" }, etc.
//
//     6) Implement the GLEContext hook function(s) you declared.
//        e.g.  void OVR::GLEContext::glDebugMessageControl_Hook(GLenum source, GLenum type, GLenum
//        severity, GLsizei count, const GLuint* ids, GLboolean enabled)
//              {
//                 if(glDebugMessageControl_Impl)
//                    glDebugMessageControl_Impl(source, type, severity, count, ids, enabled);
//                 PostHook();
//              }
//
// In order to test this, build with GLE_HOOKING_ENABLED defined and not defined.
//
// Note that if the extension is a WGL-, GLX-, or CGL-specific extension, they are handled like
// above but are in their own section below the section for regular OpenGL extensions.
//
// In some cases the given interface may already be present by currently commented out,
// in which case you can simply un-comment it to enable it.
///////////////////////////////////////////////////////////////////////////////

namespace OVR {
// Generic OpenGL GetProcAddress function interface. Maps to platform-specific functionality
// internally. On Windows this is equivalent to wglGetProcAddress as opposed to global
// GetProcAddress.

typedef void (*fptr)(void);
typedef fptr(*FuncPtr)();

#if defined(__ANDROID__)
fptr GLEGetProcAddress(const char* name);
#else
void* GLEGetProcAddress(const char* name);
#endif

#define GLE_ENABLE 1
#define GLE_DISABLE 0

#define GLE_IMPL(enable, type, name) \
    type name##_Impl; enum { name##_enabled = enable }

// GLEContext
//
// Manages a collection of OpenGL extension interfaces.
// If the application has multiple OpenGL unrelated contexts then you will want to create a
// different instance of this class for each one you intend to use it with.
//
// Example usage:
//     GLEContext gGLEContext;
//
//     GLEContext::SetCurrentContext(&gGLEContext);
//     gGLEContext.PlatformInit(); // Initializes WGL/GLX/etc. platform-specific OpenGL
//     functionality
//
//     if(GLE_WGL_ARB_create_context) // If wglCreateContextAttribsARB is available...
//     {
//         int attribList[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 2,
//         WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB, None }; HGLRC h =
//         wglCreateContextAttribsARB(hDC, 0, attribList);
//         [...]
//     }
//
//     gGLEContext.Init(); // Must be called after an OpenGL context has been created.
//
//     if(GLE_WHOLE_VERSION() >= 302) // If OpenGL 3.2 or later
//     {
//         glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, someTexture, 0); // This is an
//         OpenGL 3.2 function.
//         [...]
//     }
//
//     if(GLE_GL_ARB_texture_multisample) // If the GL_ARB_texture_multisample extension is
//     available...
//     {
//         glEnable(GL_SAMPLE_MASK);
//         glSampleMaski(0, 0x1);
//         [...]
//     }
//
//     [...]
//
//     gGLEContext.Shutdown();
//
class GLE_CLASS_EXPORT GLEContext {

private:
  GLEContext();
  GLEContext* contextInstance = NULL;

public:
  ~GLEContext();

  // Initializes platform-specific functionality (e.g. Windows WGL, Unix GLX, Android EGL, Apple
  // CGL). You would typically call this before creating an OpenGL context and using
  // platform-specific functions.
  void PlatformInit();
  bool IsPlatformInitialized() const;

  // Loads all the extensions from the current OpenGL context. This must be called after an OpenGL
  // context has been created and made current.
  void Init();
  bool IsInitialized() const;

  // Clears all the extensions initialized by PlatformInit and Init.
  void Shutdown();

  void SetEnableHookGetError(bool enabled) {
    EnableHookGetError = enabled;
  }

  // Returns the default instance of this class.
  static GLEContext* GetCurrentContext();



 public:
  // OpenGL version information
  int MajorVersion; // OpenGL major version
  int MinorVersion; // OpenGL minor version
  int WholeVersion; // Equals ((MajorVersion * 100) + MinorVersion). Example usage:
  // if(glv.WholeVersion >= 302) // If OpenGL v3.02+ ...
  bool IsGLES; // Open GL ES?
  bool IsCoreProfile; // Is the current OpenGL context a core profile context? Its trueness may be a
  // false positive but will never be a false negative.
  bool EnableHookGetError; // If enabled then hook functions call glGetError after making the call.

  int PlatformMajorVersion; // GLX/WGL/EGL/CGL version. Not the same as OpenGL version.
  int PlatformMinorVersion;
  int PlatformWholeVersion;

  void InitVersion(); // Initializes the version information (e.g. MajorVersion). Called by the
  // public Init function.
  void InitExtensionLoad(); // Loads the function addresses into the function pointers.
  void InitExtensionSupport(); // Loads the boolean extension support booleans.

  void InitPlatformVersion();
  void InitPlatformExtensionLoad();
  void InitPlatformExtensionSupport();

 public:
// GL_VERSION_1_1
// Not normally included because all OpenGL 1.1 functionality is always present. But if we have
// hooking enabled then we implement our own version of each function.
#if defined(GLE_HOOKING_ENABLED)
  // void PreHook(const char* functionName);             // Called at the beginning of a hook
  // function.
  void PostHook(const char* functionName); // Called at the end of a hook function.

  void glAccum_Hook(GLenum op, GLfloat value);
  void glAlphaFunc_Hook(GLenum func, GLclampf ref);
  GLboolean glAreTexturesResident_Hook(GLsizei n, const GLuint* textures, GLboolean* residences);
  void glArrayElement_Hook(GLint i);
  void glBegin_Hook(GLenum mode);
  void glBindTexture_Hook(GLenum target, GLuint texture);
  void glBitmap_Hook(
      GLsizei width,
      GLsizei height,
      GLfloat xorig,
      GLfloat yorig,
      GLfloat xmove,
      GLfloat ymove,
      const GLubyte* bitmap);
  void glBlendFunc_Hook(GLenum sfactor, GLenum dfactor);
  void glCallList_Hook(GLuint list);
  void glCallLists_Hook(GLsizei n, GLenum type, const void* lists);
  void glClear_Hook(GLbitfield mask);
  void glClearAccum_Hook(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
  void glClearColor_Hook(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
  void glClearDepth_Hook(GLclampd depth);
  void glClearIndex_Hook(GLfloat c);
  void glClearStencil_Hook(GLint s);
  void glClipPlane_Hook(GLenum plane, const GLdouble* equation);
  void glColor3b_Hook(GLbyte red, GLbyte green, GLbyte blue);
  void glColor3bv_Hook(const GLbyte* v);
  void glColor3d_Hook(GLdouble red, GLdouble green, GLdouble blue);
  void glColor3dv_Hook(const GLdouble* v);
  void glColor3f_Hook(GLfloat red, GLfloat green, GLfloat blue);
  void glColor3fv_Hook(const GLfloat* v);
  void glColor3i_Hook(GLint red, GLint green, GLint blue);
  void glColor3iv_Hook(const GLint* v);
  void glColor3s_Hook(GLshort red, GLshort green, GLshort blue);
  void glColor3sv_Hook(const GLshort* v);
  void glColor3ub_Hook(GLubyte red, GLubyte green, GLubyte blue);
  void glColor3ubv_Hook(const GLubyte* v);
  void glColor3ui_Hook(GLuint red, GLuint green, GLuint blue);
  void glColor3uiv_Hook(const GLuint* v);
  void glColor3us_Hook(GLushort red, GLushort green, GLushort blue);
  void glColor3usv_Hook(const GLushort* v);
  void glColor4b_Hook(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
  void glColor4bv_Hook(const GLbyte* v);
  void glColor4d_Hook(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
  void glColor4dv_Hook(const GLdouble* v);
  void glColor4f_Hook(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
  void glColor4fv_Hook(const GLfloat* v);
  void glColor4i_Hook(GLint red, GLint green, GLint blue, GLint alpha);
  void glColor4iv_Hook(const GLint* v);
  void glColor4s_Hook(GLshort red, GLshort green, GLshort blue, GLshort alpha);
  void glColor4sv_Hook(const GLshort* v);
  void glColor4ub_Hook(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
  void glColor4ubv_Hook(const GLubyte* v);
  void glColor4ui_Hook(GLuint red, GLuint green, GLuint blue, GLuint alpha);
  void glColor4uiv_Hook(const GLuint* v);
  void glColor4us_Hook(GLushort red, GLushort green, GLushort blue, GLushort alpha);
  void glColor4usv_Hook(const GLushort* v);
  void glColorMask_Hook(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
  void glColorMaterial_Hook(GLenum face, GLenum mode);
  void glColorPointer_Hook(GLint size, GLenum type, GLsizei stride, const void* pointer);
  void glCopyPixels_Hook(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
  void glCopyTexImage1D_Hook(
      GLenum target,
      GLint level,
      GLenum internalFormat,
      GLint x,
      GLint y,
      GLsizei width,
      GLint border);
  void glCopyTexImage2D_Hook(
      GLenum target,
      GLint level,
      GLenum internalFormat,
      GLint x,
      GLint y,
      GLsizei width,
      GLsizei height,
      GLint border);
  void glCopyTexSubImage1D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLint x,
      GLint y,
      GLsizei width);
  void glCopyTexSubImage2D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLint yoffset,
      GLint x,
      GLint y,
      GLsizei width,
      GLsizei height);
  void glCullFace_Hook(GLenum mode);
  void glDeleteLists_Hook(GLuint list, GLsizei range);
  void glDeleteTextures_Hook(GLsizei n, const GLuint* textures);
  void glDepthFunc_Hook(GLenum func);
  void glDepthMask_Hook(GLboolean flag);
  void glDepthRange_Hook(GLclampd zNear, GLclampd zFar);
  void glDisable_Hook(GLenum cap);
  void glDisableClientState_Hook(GLenum array);
  void glDrawArrays_Hook(GLenum mode, GLint first, GLsizei count);
  void glDrawBuffer_Hook(GLenum mode);
  void glDrawElements_Hook(GLenum mode, GLsizei count, GLenum type, const void* indices);
  void
  glDrawPixels_Hook(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
  void glEdgeFlag_Hook(GLboolean flag);
  void glEdgeFlagPointer_Hook(GLsizei stride, const void* pointer);
  void glEdgeFlagv_Hook(const GLboolean* flag);
  void glEnable_Hook(GLenum cap);
  void glEnableClientState_Hook(GLenum array);
  void glEnd_Hook(void);
  void glEndList_Hook(void);
  void glEvalCoord1d_Hook(GLdouble u);
  void glEvalCoord1dv_Hook(const GLdouble* u);
  void glEvalCoord1f_Hook(GLfloat u);
  void glEvalCoord1fv_Hook(const GLfloat* u);
  void glEvalCoord2d_Hook(GLdouble u, GLdouble v);
  void glEvalCoord2dv_Hook(const GLdouble* u);
  void glEvalCoord2f_Hook(GLfloat u, GLfloat v);
  void glEvalCoord2fv_Hook(const GLfloat* u);
  void glEvalMesh1_Hook(GLenum mode, GLint i1, GLint i2);
  void glEvalMesh2_Hook(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
  void glEvalPoint1_Hook(GLint i);
  void glEvalPoint2_Hook(GLint i, GLint j);
  void glFeedbackBuffer_Hook(GLsizei size, GLenum type, GLfloat* buffer);
  void glFinish_Hook(void);
  void glFlush_Hook(void);
  void glFogf_Hook(GLenum pname, GLfloat param);
  void glFogfv_Hook(GLenum pname, const GLfloat* params);
  void glFogi_Hook(GLenum pname, GLint param);
  void glFogiv_Hook(GLenum pname, const GLint* params);
  void glFrontFace_Hook(GLenum mode);
  void glFrustum_Hook(
      GLdouble left,
      GLdouble right,
      GLdouble bottom,
      GLdouble top,
      GLdouble zNear,
      GLdouble zFar);
  GLuint glGenLists_Hook(GLsizei range);
  void glGenTextures_Hook(GLsizei n, GLuint* textures);
  void glGetBooleanv_Hook(GLenum pname, GLboolean* params);
  void glGetClipPlane_Hook(GLenum plane, GLdouble* equation);
  void glGetDoublev_Hook(GLenum pname, GLdouble* params);
  GLenum glGetError_Hook(void);
  void glGetFloatv_Hook(GLenum pname, GLfloat* params);
  void glGetIntegerv_Hook(GLenum pname, GLint* params);
  void glGetLightfv_Hook(GLenum light, GLenum pname, GLfloat* params);
  void glGetLightiv_Hook(GLenum light, GLenum pname, GLint* params);
  void glGetMapdv_Hook(GLenum target, GLenum query, GLdouble* v);
  void glGetMapfv_Hook(GLenum target, GLenum query, GLfloat* v);
  void glGetMapiv_Hook(GLenum target, GLenum query, GLint* v);
  void glGetMaterialfv_Hook(GLenum face, GLenum pname, GLfloat* params);
  void glGetMaterialiv_Hook(GLenum face, GLenum pname, GLint* params);
  void glGetPixelMapfv_Hook(GLenum map, GLfloat* values);
  void glGetPixelMapuiv_Hook(GLenum map, GLuint* values);
  void glGetPixelMapusv_Hook(GLenum map, GLushort* values);
  void glGetPointerv_Hook(GLenum pname, void** params);
  void glGetPolygonStipple_Hook(GLubyte* mask);
  const GLubyte* glGetString_Hook(GLenum name);
  void glGetTexEnvfv_Hook(GLenum target, GLenum pname, GLfloat* params);
  void glGetTexEnviv_Hook(GLenum target, GLenum pname, GLint* params);
  void glGetTexGendv_Hook(GLenum coord, GLenum pname, GLdouble* params);
  void glGetTexGenfv_Hook(GLenum coord, GLenum pname, GLfloat* params);
  void glGetTexGeniv_Hook(GLenum coord, GLenum pname, GLint* params);
  void glGetTexImage_Hook(GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
  void glGetTexLevelParameterfv_Hook(GLenum target, GLint level, GLenum pname, GLfloat* params);
  void glGetTexLevelParameteriv_Hook(GLenum target, GLint level, GLenum pname, GLint* params);
  void glGetTexParameterfv_Hook(GLenum target, GLenum pname, GLfloat* params);
  void glGetTexParameteriv_Hook(GLenum target, GLenum pname, GLint* params);
  void glHint_Hook(GLenum target, GLenum mode);
  void glIndexMask_Hook(GLuint mask);
  void glIndexPointer_Hook(GLenum type, GLsizei stride, const void* pointer);
  void glIndexd_Hook(GLdouble c);
  void glIndexdv_Hook(const GLdouble* c);
  void glIndexf_Hook(GLfloat c);
  void glIndexfv_Hook(const GLfloat* c);
  void glIndexi_Hook(GLint c);
  void glIndexiv_Hook(const GLint* c);
  void glIndexs_Hook(GLshort c);
  void glIndexsv_Hook(const GLshort* c);
  void glIndexub_Hook(GLubyte c);
  void glIndexubv_Hook(const GLubyte* c);
  void glInitNames_Hook(void);
  void glInterleavedArrays_Hook(GLenum format, GLsizei stride, const void* pointer);
  GLboolean glIsEnabled_Hook(GLenum cap);
  GLboolean glIsList_Hook(GLuint list);
  GLboolean glIsTexture_Hook(GLuint texture);
  void glLightModelf_Hook(GLenum pname, GLfloat param);
  void glLightModelfv_Hook(GLenum pname, const GLfloat* params);
  void glLightModeli_Hook(GLenum pname, GLint param);
  void glLightModeliv_Hook(GLenum pname, const GLint* params);
  void glLightf_Hook(GLenum light, GLenum pname, GLfloat param);
  void glLightfv_Hook(GLenum light, GLenum pname, const GLfloat* params);
  void glLighti_Hook(GLenum light, GLenum pname, GLint param);
  void glLightiv_Hook(GLenum light, GLenum pname, const GLint* params);
  void glLineStipple_Hook(GLint factor, GLushort pattern);
  void glLineWidth_Hook(GLfloat width);
  void glListBase_Hook(GLuint base);
  void glLoadIdentity_Hook(void);
  void glLoadMatrixd_Hook(const GLdouble* m);
  void glLoadMatrixf_Hook(const GLfloat* m);
  void glLoadName_Hook(GLuint name);
  void glLogicOp_Hook(GLenum opcode);
  void glMap1d_Hook(
      GLenum target,
      GLdouble u1,
      GLdouble u2,
      GLint stride,
      GLint order,
      const GLdouble* points);
  void glMap1f_Hook(
      GLenum target,
      GLfloat u1,
      GLfloat u2,
      GLint stride,
      GLint order,
      const GLfloat* points);
  void glMap2d_Hook(
      GLenum target,
      GLdouble u1,
      GLdouble u2,
      GLint ustride,
      GLint uorder,
      GLdouble v1,
      GLdouble v2,
      GLint vstride,
      GLint vorder,
      const GLdouble* points);
  void glMap2f_Hook(
      GLenum target,
      GLfloat u1,
      GLfloat u2,
      GLint ustride,
      GLint uorder,
      GLfloat v1,
      GLfloat v2,
      GLint vstride,
      GLint vorder,
      const GLfloat* points);
  void glMapGrid1d_Hook(GLint un, GLdouble u1, GLdouble u2);
  void glMapGrid1f_Hook(GLint un, GLfloat u1, GLfloat u2);
  void glMapGrid2d_Hook(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
  void glMapGrid2f_Hook(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
  void glMaterialf_Hook(GLenum face, GLenum pname, GLfloat param);
  void glMaterialfv_Hook(GLenum face, GLenum pname, const GLfloat* params);
  void glMateriali_Hook(GLenum face, GLenum pname, GLint param);
  void glMaterialiv_Hook(GLenum face, GLenum pname, const GLint* params);
  void glMatrixMode_Hook(GLenum mode);
  void glMultMatrixd_Hook(const GLdouble* m);
  void glMultMatrixf_Hook(const GLfloat* m);
  void glNewList_Hook(GLuint list, GLenum mode);
  void glNormal3b_Hook(GLbyte nx, GLbyte ny, GLbyte nz);
  void glNormal3bv_Hook(const GLbyte* v);
  void glNormal3d_Hook(GLdouble nx, GLdouble ny, GLdouble nz);
  void glNormal3dv_Hook(const GLdouble* v);
  void glNormal3f_Hook(GLfloat nx, GLfloat ny, GLfloat nz);
  void glNormal3fv_Hook(const GLfloat* v);
  void glNormal3i_Hook(GLint nx, GLint ny, GLint nz);
  void glNormal3iv_Hook(const GLint* v);
  void glNormal3s_Hook(GLshort nx, GLshort ny, GLshort nz);
  void glNormal3sv_Hook(const GLshort* v);
  void glNormalPointer_Hook(GLenum type, GLsizei stride, const void* pointer);
  void glOrtho_Hook(
      GLdouble left,
      GLdouble right,
      GLdouble bottom,
      GLdouble top,
      GLdouble zNear,
      GLdouble zFar);
  void glPassThrough_Hook(GLfloat token);
  void glPixelMapfv_Hook(GLenum map, GLsizei mapsize, const GLfloat* values);
  void glPixelMapuiv_Hook(GLenum map, GLsizei mapsize, const GLuint* values);
  void glPixelMapusv_Hook(GLenum map, GLsizei mapsize, const GLushort* values);
  void glPixelStoref_Hook(GLenum pname, GLfloat param);
  void glPixelStorei_Hook(GLenum pname, GLint param);
  void glPixelTransferf_Hook(GLenum pname, GLfloat param);
  void glPixelTransferi_Hook(GLenum pname, GLint param);
  void glPixelZoom_Hook(GLfloat xfactor, GLfloat yfactor);
  void glPointSize_Hook(GLfloat size);
  void glPolygonMode_Hook(GLenum face, GLenum mode);
  void glPolygonOffset_Hook(GLfloat factor, GLfloat units);
  void glPolygonStipple_Hook(const GLubyte* mask);
  void glPopAttrib_Hook(void);
  void glPopClientAttrib_Hook(void);
  void glPopMatrix_Hook(void);
  void glPopName_Hook(void);
  void glPrioritizeTextures_Hook(GLsizei n, const GLuint* textures, const GLclampf* priorities);
  void glPushAttrib_Hook(GLbitfield mask);
  void glPushClientAttrib_Hook(GLbitfield mask);
  void glPushMatrix_Hook(void);
  void glPushName_Hook(GLuint name);
  void glRasterPos2d_Hook(GLdouble x, GLdouble y);
  void glRasterPos2dv_Hook(const GLdouble* v);
  void glRasterPos2f_Hook(GLfloat x, GLfloat y);
  void glRasterPos2fv_Hook(const GLfloat* v);
  void glRasterPos2i_Hook(GLint x, GLint y);
  void glRasterPos2iv_Hook(const GLint* v);
  void glRasterPos2s_Hook(GLshort x, GLshort y);
  void glRasterPos2sv_Hook(const GLshort* v);
  void glRasterPos3d_Hook(GLdouble x, GLdouble y, GLdouble z);
  void glRasterPos3dv_Hook(const GLdouble* v);
  void glRasterPos3f_Hook(GLfloat x, GLfloat y, GLfloat z);
  void glRasterPos3fv_Hook(const GLfloat* v);
  void glRasterPos3i_Hook(GLint x, GLint y, GLint z);
  void glRasterPos3iv_Hook(const GLint* v);
  void glRasterPos3s_Hook(GLshort x, GLshort y, GLshort z);
  void glRasterPos3sv_Hook(const GLshort* v);
  void glRasterPos4d_Hook(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void glRasterPos4dv_Hook(const GLdouble* v);
  void glRasterPos4f_Hook(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void glRasterPos4fv_Hook(const GLfloat* v);
  void glRasterPos4i_Hook(GLint x, GLint y, GLint z, GLint w);
  void glRasterPos4iv_Hook(const GLint* v);
  void glRasterPos4s_Hook(GLshort x, GLshort y, GLshort z, GLshort w);
  void glRasterPos4sv_Hook(const GLshort* v);
  void glReadBuffer_Hook(GLenum mode);
  void glReadPixels_Hook(
      GLint x,
      GLint y,
      GLsizei width,
      GLsizei height,
      GLenum format,
      GLenum type,
      void* pixels);
  void glRectd_Hook(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
  void glRectdv_Hook(const GLdouble* v1, const GLdouble* v2);
  void glRectf_Hook(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
  void glRectfv_Hook(const GLfloat* v1, const GLfloat* v2);
  void glRecti_Hook(GLint x1, GLint y1, GLint x2, GLint y2);
  void glRectiv_Hook(const GLint* v1, const GLint* v2);
  void glRects_Hook(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
  void glRectsv_Hook(const GLshort* v1, const GLshort* v2);
  GLint glRenderMode_Hook(GLenum mode);
  void glRotated_Hook(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
  void glRotatef_Hook(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
  void glScaled_Hook(GLdouble x, GLdouble y, GLdouble z);
  void glScalef_Hook(GLfloat x, GLfloat y, GLfloat z);
  void glScissor_Hook(GLint x, GLint y, GLsizei width, GLsizei height);
  void glSelectBuffer_Hook(GLsizei size, GLuint* buffer);
  void glShadeModel_Hook(GLenum mode);
  void glStencilFunc_Hook(GLenum func, GLint ref, GLuint mask);
  void glStencilMask_Hook(GLuint mask);
  void glStencilOp_Hook(GLenum fail, GLenum zfail, GLenum zpass);
  void glTexCoord1d_Hook(GLdouble s);
  void glTexCoord1dv_Hook(const GLdouble* v);
  void glTexCoord1f_Hook(GLfloat s);
  void glTexCoord1fv_Hook(const GLfloat* v);
  void glTexCoord1i_Hook(GLint s);
  void glTexCoord1iv_Hook(const GLint* v);
  void glTexCoord1s_Hook(GLshort s);
  void glTexCoord1sv_Hook(const GLshort* v);
  void glTexCoord2d_Hook(GLdouble s, GLdouble t);
  void glTexCoord2dv_Hook(const GLdouble* v);
  void glTexCoord2f_Hook(GLfloat s, GLfloat t);
  void glTexCoord2fv_Hook(const GLfloat* v);
  void glTexCoord2i_Hook(GLint s, GLint t);
  void glTexCoord2iv_Hook(const GLint* v);
  void glTexCoord2s_Hook(GLshort s, GLshort t);
  void glTexCoord2sv_Hook(const GLshort* v);
  void glTexCoord3d_Hook(GLdouble s, GLdouble t, GLdouble r);
  void glTexCoord3dv_Hook(const GLdouble* v);
  void glTexCoord3f_Hook(GLfloat s, GLfloat t, GLfloat r);
  void glTexCoord3fv_Hook(const GLfloat* v);
  void glTexCoord3i_Hook(GLint s, GLint t, GLint r);
  void glTexCoord3iv_Hook(const GLint* v);
  void glTexCoord3s_Hook(GLshort s, GLshort t, GLshort r);
  void glTexCoord3sv_Hook(const GLshort* v);
  void glTexCoord4d_Hook(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
  void glTexCoord4dv_Hook(const GLdouble* v);
  void glTexCoord4f_Hook(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
  void glTexCoord4fv_Hook(const GLfloat* v);
  void glTexCoord4i_Hook(GLint s, GLint t, GLint r, GLint q);
  void glTexCoord4iv_Hook(const GLint* v);
  void glTexCoord4s_Hook(GLshort s, GLshort t, GLshort r, GLshort q);
  void glTexCoord4sv_Hook(const GLshort* v);
  void glTexCoordPointer_Hook(GLint size, GLenum type, GLsizei stride, const void* pointer);
  void glTexEnvf_Hook(GLenum target, GLenum pname, GLfloat param);
  void glTexEnvfv_Hook(GLenum target, GLenum pname, const GLfloat* params);
  void glTexEnvi_Hook(GLenum target, GLenum pname, GLint param);
  void glTexEnviv_Hook(GLenum target, GLenum pname, const GLint* params);
  void glTexGend_Hook(GLenum coord, GLenum pname, GLdouble param);
  void glTexGendv_Hook(GLenum coord, GLenum pname, const GLdouble* params);
  void glTexGenf_Hook(GLenum coord, GLenum pname, GLfloat param);
  void glTexGenfv_Hook(GLenum coord, GLenum pname, const GLfloat* params);
  void glTexGeni_Hook(GLenum coord, GLenum pname, GLint param);
  void glTexGeniv_Hook(GLenum coord, GLenum pname, const GLint* params);
  void glTexImage1D_Hook(
      GLenum target,
      GLint level,
      GLint internalformat,
      GLsizei width,
      GLint border,
      GLenum format,
      GLenum type,
      const void* pixels);
  void glTexImage2D_Hook(
      GLenum target,
      GLint level,
      GLint internalformat,
      GLsizei width,
      GLsizei height,
      GLint border,
      GLenum format,
      GLenum type,
      const void* pixels);
  void glTexParameterf_Hook(GLenum target, GLenum pname, GLfloat param);
  void glTexParameterfv_Hook(GLenum target, GLenum pname, const GLfloat* params);
  void glTexParameteri_Hook(GLenum target, GLenum pname, GLint param);
  void glTexParameteriv_Hook(GLenum target, GLenum pname, const GLint* params);
  void glTexSubImage1D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLsizei width,
      GLenum format,
      GLenum type,
      const void* pixels);
  void glTexSubImage2D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLint yoffset,
      GLsizei width,
      GLsizei height,
      GLenum format,
      GLenum type,
      const void* pixels);
  void glTranslated_Hook(GLdouble x, GLdouble y, GLdouble z);
  void glTranslatef_Hook(GLfloat x, GLfloat y, GLfloat z);
  void glVertex2d_Hook(GLdouble x, GLdouble y);
  void glVertex2dv_Hook(const GLdouble* v);
  void glVertex2f_Hook(GLfloat x, GLfloat y);
  void glVertex2fv_Hook(const GLfloat* v);
  void glVertex2i_Hook(GLint x, GLint y);
  void glVertex2iv_Hook(const GLint* v);
  void glVertex2s_Hook(GLshort x, GLshort y);
  void glVertex2sv_Hook(const GLshort* v);
  void glVertex3d_Hook(GLdouble x, GLdouble y, GLdouble z);
  void glVertex3dv_Hook(const GLdouble* v);
  void glVertex3f_Hook(GLfloat x, GLfloat y, GLfloat z);
  void glVertex3fv_Hook(const GLfloat* v);
  void glVertex3i_Hook(GLint x, GLint y, GLint z);
  void glVertex3iv_Hook(const GLint* v);
  void glVertex3s_Hook(GLshort x, GLshort y, GLshort z);
  void glVertex3sv_Hook(const GLshort* v);
  void glVertex4d_Hook(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void glVertex4dv_Hook(const GLdouble* v);
  void glVertex4f_Hook(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void glVertex4fv_Hook(const GLfloat* v);
  void glVertex4i_Hook(GLint x, GLint y, GLint z, GLint w);
  void glVertex4iv_Hook(const GLint* v);
  void glVertex4s_Hook(GLshort x, GLshort y, GLshort z, GLshort w);
  void glVertex4sv_Hook(const GLshort* v);
  void glVertexPointer_Hook(GLint size, GLenum type, GLsizei stride, const void* pointer);
  void glViewport_Hook(GLint x, GLint y, GLsizei width, GLsizei height);

  // GL_VERSION_1_2
  void glBlendColor_Hook(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
  void glBlendEquation_Hook(GLenum mode);
  void glDrawRangeElements_Hook(
      GLenum mode,
      GLuint start,
      GLuint end,
      GLsizei count,
      GLenum type,
      const GLvoid* indices);
  void glTexImage3D_Hook(
      GLenum target,
      GLint level,
      GLint internalformat,
      GLsizei width,
      GLsizei height,
      GLsizei depth,
      GLint border,
      GLenum format,
      GLenum type,
      const GLvoid* pixels);
  void glTexSubImage3D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLint yoffset,
      GLint zoffset,
      GLsizei width,
      GLsizei height,
      GLsizei depth,
      GLenum format,
      GLenum type,
      const GLvoid* pixels);
  void glCopyTexSubImage3D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLint yoffset,
      GLint zoffset,
      GLint x,
      GLint y,
      GLsizei width,
      GLsizei height);

  // GL_VERSION_1_2 deprecated functions
  /* Not currently supported
  void glColorTable_Hook(GLenum target, GLenum internalformat, GLsizei width,
                          GLenum format, GLenum type, const GLvoid *table);
  void glColorTableParameterfv_Hook(GLenum target, GLenum pname, const GLfloat *params);
  void glColorTableParameteriv_Hook(GLenum target, GLenum pname, const GLint *params);
  void glCopyColorTable_Hook(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
  void glGetColorTable_Hook(GLenum target, GLenum format, GLenum type, GLvoid *table);
  void glGetColorTableParameterfv_Hook(GLenum target, GLenum pname, GLfloat *params);
  void glGetColorTableParameteriv_Hook(GLenum target, GLenum pname, GLint *params);
  void glColorSubTable_Hook(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type,
                            const GLvoid *data);
  void glCopyColorSubTable_Hook(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
  void glConvolutionFilter1D_Hook(GLenum target, GLenum internalformat, GLsizei width, GLenum
                                  format, GLenum type, const GLvoid *image);
  void glConvolutionFilter2D_Hook(GLenum target, GLenum internalformat, GLsizei width, GLsizei
                                  height, GLenum format, GLenum type, const GLvoid *image);
  void glConvolutionParameterf_Hook(GLenum target, GLenum pname, GLfloat params);
  void glConvolutionParameterfv_Hook(GLenum target, GLenum pname, const GLfloat *params);
  void glConvolutionParameteri_Hook(GLenum target, GLenum pname, GLint params);
  void glConvolutionParameteriv_Hook(GLenum target, GLenum pname, const GLint *params);
  void glCopyConvolutionFilter1D_Hook(GLenum target, GLenum internalformat,
                                      GLint x, GLint y, GLsizei width);
  void glCopyConvolutionFilter2D_Hook(GLenum target, GLenum internalformat,
                                      GLint x, GLint y, GLsizei width, GLsizei height);
  void glGetConvolutionFilter_Hook(GLenum target, GLenum format, GLenum type, GLvoid *image);
  void glGetConvolutionParameterfv_Hook(GLenum target, GLenum pname, GLfloat *params);
  void glGetConvolutionParameteriv_Hook(GLenum target, GLenum pname, GLint *params);
  void glGetSeparableFilter_Hook(GLenum target, GLenum format, GLenum type,
                                 GLvoid *row, GLvoid *column, GLvoid *span);
  void glSeparableFilter2D_Hook(GLenum target, GLenum internalformat, GLsizei width, GLsizei height,
                               GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
  void glGetHistogram_Hook(GLenum target, GLboolean reset, GLenum format,
                           GLenum type, GLvoid *values);
  void glGetHistogramParameterfv_Hook(GLenum target, GLenum pname, GLfloat *params);
  void glGetHistogramParameteriv_Hook(GLenum target, GLenum pname, GLint *params);
  void glGetMinmax_Hook(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
  void glGetMinmaxParameterfv_Hook(GLenum target, GLenum pname, GLfloat *params);
  void glGetMinmaxParameteriv_Hook(GLenum target, GLenum pname, GLint *params);
  void glHistogram_Hook(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
  void glMinmax_Hook(GLenum target, GLenum internalformat, GLboolean sink);
  void glResetHistogram_Hook(GLenum target);
  void glResetMinmax_Hook(GLenum target);
  */

  // GL_VERSION_1_3
  void glActiveTexture_Hook(GLenum texture);
  void glSampleCoverage_Hook(GLclampf value, GLboolean invert);
  void glCompressedTexImage3D_Hook(
      GLenum target,
      GLint level,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLsizei depth,
      GLint border,
      GLsizei imageSize,
      const GLvoid* data);
  void glCompressedTexImage2D_Hook(
      GLenum target,
      GLint level,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLint border,
      GLsizei imageSize,
      const GLvoid* data);
  void glCompressedTexImage1D_Hook(
      GLenum target,
      GLint level,
      GLenum internalformat,
      GLsizei width,
      GLint border,
      GLsizei imageSize,
      const GLvoid* data);
  void glCompressedTexSubImage3D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLint yoffset,
      GLint zoffset,
      GLsizei width,
      GLsizei height,
      GLsizei depth,
      GLenum format,
      GLsizei imageSize,
      const GLvoid* data);
  void glCompressedTexSubImage2D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLint yoffset,
      GLsizei width,
      GLsizei height,
      GLenum format,
      GLsizei imageSize,
      const GLvoid* data);
  void glCompressedTexSubImage1D_Hook(
      GLenum target,
      GLint level,
      GLint xoffset,
      GLsizei width,
      GLenum format,
      GLsizei imageSize,
      const GLvoid* data);
  void glGetCompressedTexImage_Hook(GLenum target, GLint level, GLvoid* img);

  // GL_VERSION_1_3 deprecated functions
  void glClientActiveTexture_Hook(GLenum texture);
  void glMultiTexCoord1d_Hook(GLenum target, GLdouble s);
  void glMultiTexCoord1dv_Hook(GLenum target, const GLdouble* v);
  void glMultiTexCoord1f_Hook(GLenum target, GLfloat s);
  void glMultiTexCoord1fv_Hook(GLenum target, const GLfloat* v);
  void glMultiTexCoord1i_Hook(GLenum target, GLint s);
  void glMultiTexCoord1iv_Hook(GLenum target, const GLint* v);
  void glMultiTexCoord1s_Hook(GLenum target, GLshort s);
  void glMultiTexCoord1sv_Hook(GLenum target, const GLshort* v);
  void glMultiTexCoord2d_Hook(GLenum target, GLdouble s, GLdouble t);
  void glMultiTexCoord2dv_Hook(GLenum target, const GLdouble* v);
  void glMultiTexCoord2f_Hook(GLenum target, GLfloat s, GLfloat t);
  void glMultiTexCoord2fv_Hook(GLenum target, const GLfloat* v);
  void glMultiTexCoord2i_Hook(GLenum target, GLint s, GLint t);
  void glMultiTexCoord2iv_Hook(GLenum target, const GLint* v);
  void glMultiTexCoord2s_Hook(GLenum target, GLshort s, GLshort t);
  void glMultiTexCoord2sv_Hook(GLenum target, const GLshort* v);
  void glMultiTexCoord3d_Hook(GLenum target, GLdouble s, GLdouble t, GLdouble r);
  void glMultiTexCoord3dv_Hook(GLenum target, const GLdouble* v);
  void glMultiTexCoord3f_Hook(GLenum target, GLfloat s, GLfloat t, GLfloat r);
  void glMultiTexCoord3fv_Hook(GLenum target, const GLfloat* v);
  void glMultiTexCoord3i_Hook(GLenum target, GLint s, GLint t, GLint r);
  void glMultiTexCoord3iv_Hook(GLenum target, const GLint* v);
  void glMultiTexCoord3s_Hook(GLenum target, GLshort s, GLshort t, GLshort r);
  void glMultiTexCoord3sv_Hook(GLenum target, const GLshort* v);
  void glMultiTexCoord4d_Hook(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
  void glMultiTexCoord4dv_Hook(GLenum target, const GLdouble* v);
  void glMultiTexCoord4f_Hook(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
  void glMultiTexCoord4fv_Hook(GLenum target, const GLfloat* v);
  void glMultiTexCoord4i_Hook(GLenum target, GLint s, GLint t, GLint r, GLint q);
  void glMultiTexCoord4iv_Hook(GLenum target, const GLint* v);
  void glMultiTexCoord4s_Hook(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
  void glMultiTexCoord4sv_Hook(GLenum target, const GLshort* v);
  void glLoadTransposeMatrixf_Hook(const GLfloat* m);
  void glLoadTransposeMatrixd_Hook(const GLdouble* m);
  void glMultTransposeMatrixf_Hook(const GLfloat* m);
  void glMultTransposeMatrixd_Hook(const GLdouble* m);

  // GL_VERSION_1_4
  void glBlendFuncSeparate_Hook(
      GLenum sfactorRGB,
      GLenum dfactorRGB,
      GLenum sfactorAlpha,
      GLenum dfactorAlpha);
  void
  glMultiDrawArrays_Hook(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount);
  void glMultiDrawElements_Hook(
      GLenum mode,
      const GLsizei* count,
      GLenum type,
      const GLvoid** indices,
      GLsizei primcount);
  void glPointParameterf_Hook(GLenum pname, GLfloat param);
  void glPointParameterfv_Hook(GLenum pname, const GLfloat* params);
  void glPointParameteri_Hook(GLenum pname, GLint param);
  void glPointParameteriv_Hook(GLenum pname, const GLint* params);

  // GL_VERSION_1_4 deprecated functions
  void glFogCoordf_Hook(GLfloat coord);
  void glFogCoordfv_Hook(const GLfloat* coord);
  void glFogCoordd_Hook(GLdouble coord);
  void glFogCoorddv_Hook(const GLdouble* coord);
  void glFogCoordPointer_Hook(GLenum type, GLsizei stride, const GLvoid* pointer);
  void glSecondaryColor3b_Hook(GLbyte red, GLbyte green, GLbyte blue);
  void glSecondaryColor3bv_Hook(const GLbyte* v);
  void glSecondaryColor3d_Hook(GLdouble red, GLdouble green, GLdouble blue);
  void glSecondaryColor3dv_Hook(const GLdouble* v);
  void glSecondaryColor3f_Hook(GLfloat red, GLfloat green, GLfloat blue);
  void glSecondaryColor3fv_Hook(const GLfloat* v);
  void glSecondaryColor3i_Hook(GLint red, GLint green, GLint blue);
  void glSecondaryColor3iv_Hook(const GLint* v);
  void glSecondaryColor3s_Hook(GLshort red, GLshort green, GLshort blue);
  void glSecondaryColor3sv_Hook(const GLshort* v);
  void glSecondaryColor3ub_Hook(GLubyte red, GLubyte green, GLubyte blue);
  void glSecondaryColor3ubv_Hook(const GLubyte* v);
  void glSecondaryColor3ui_Hook(GLuint red, GLuint green, GLuint blue);
  void glSecondaryColor3uiv_Hook(const GLuint* v);
  void glSecondaryColor3us_Hook(GLushort red, GLushort green, GLushort blue);
  void glSecondaryColor3usv_Hook(const GLushort* v);
  void glSecondaryColorPointer_Hook(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
  void glWindowPos2d_Hook(GLdouble x, GLdouble y);
  void glWindowPos2dv_Hook(const GLdouble* v);
  void glWindowPos2f_Hook(GLfloat x, GLfloat y);
  void glWindowPos2fv_Hook(const GLfloat* v);
  void glWindowPos2i_Hook(GLint x, GLint y);
  void glWindowPos2iv_Hook(const GLint* v);
  void glWindowPos2s_Hook(GLshort x, GLshort y);
  void glWindowPos2sv_Hook(const GLshort* v);
  void glWindowPos3d_Hook(GLdouble x, GLdouble y, GLdouble z);
  void glWindowPos3dv_Hook(const GLdouble* v);
  void glWindowPos3f_Hook(GLfloat x, GLfloat y, GLfloat z);
  void glWindowPos3fv_Hook(const GLfloat* v);
  void glWindowPos3i_Hook(GLint x, GLint y, GLint z);
  void glWindowPos3iv_Hook(const GLint* v);
  void glWindowPos3s_Hook(GLshort x, GLshort y, GLshort z);
  void glWindowPos3sv_Hook(const GLshort* v);

  // GL_VERSION_1_5
  void glGenQueries_Hook(GLsizei n, GLuint* ids);
  void glDeleteQueries_Hook(GLsizei n, const GLuint* ids);
  GLboolean glIsQuery_Hook(GLuint id);
  void glBeginQuery_Hook(GLenum target, GLuint id);
  void glEndQuery_Hook(GLenum target);
  void glGetQueryiv_Hook(GLenum target, GLenum pname, GLint* params);
  void glGetQueryObjectiv_Hook(GLuint id, GLenum pname, GLint* params);
  void glGetQueryObjectuiv_Hook(GLuint id, GLenum pname, GLuint* params);
  void glBindBuffer_Hook(GLenum target, GLuint buffer);
  void glDeleteBuffers_Hook(GLsizei n, const GLuint* buffers);
  void glGenBuffers_Hook(GLsizei n, GLuint* buffers);
  GLboolean glIsBuffer_Hook(GLuint buffer);
  void glBufferData_Hook(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
  void glBufferSubData_Hook(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
  void glGetBufferSubData_Hook(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
  GLvoid* glMapBuffer_Hook(GLenum target, GLenum access);
  GLboolean glUnmapBuffer_Hook(GLenum target);
  void glGetBufferParameteriv_Hook(GLenum target, GLenum pname, GLint* params);
  void glGetBufferPointerv_Hook(GLenum target, GLenum pname, GLvoid** params);

  // GL_VERSION_2_0
  void glBlendEquationSeparate_Hook(GLenum modeRGB, GLenum modeAlpha);
  void glDrawBuffers_Hook(GLsizei n, const GLenum* bufs);
  void glStencilOpSeparate_Hook(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
  void glStencilFuncSeparate_Hook(GLenum face, GLenum func, GLint ref, GLuint mask);
  void glStencilMaskSeparate_Hook(GLenum face, GLuint mask);
  void glAttachShader_Hook(GLuint program, GLuint shader);
  void glBindAttribLocation_Hook(GLuint program, GLuint index, const GLchar* name);
  void glCompileShader_Hook(GLuint shader);
  GLuint glCreateProgram_Hook(void);
  GLuint glCreateShader_Hook(GLenum type);
  void glDeleteProgram_Hook(GLuint program);
  void glDeleteShader_Hook(GLuint shader);
  void glDetachShader_Hook(GLuint program, GLuint shader);
  void glDisableVertexAttribArray_Hook(GLuint index);
  void glEnableVertexAttribArray_Hook(GLuint index);
  void glGetActiveAttrib_Hook(
      GLuint program,
      GLuint index,
      GLsizei bufSize,
      GLsizei* length,
      GLint* size,
      GLenum* type,
      GLchar* name);
  void glGetActiveUniform_Hook(
      GLuint program,
      GLuint index,
      GLsizei bufSize,
      GLsizei* length,
      GLint* size,
      GLenum* type,
      GLchar* name);
  void glGetAttachedShaders_Hook(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* obj);
  GLint glGetAttribLocation_Hook(GLuint program, const GLchar* name);
  void glGetProgramiv_Hook(GLuint program, GLenum pname, GLint* params);
  void glGetProgramInfoLog_Hook(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
  void glGetShaderiv_Hook(GLuint shader, GLenum pname, GLint* params);
  void glGetShaderInfoLog_Hook(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
  void glGetShaderSource_Hook(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source);
  GLint glGetUniformLocation_Hook(GLuint program, const GLchar* name);
  void glGetUniformfv_Hook(GLuint program, GLint location, GLfloat* params);
  void glGetUniformiv_Hook(GLuint program, GLint location, GLint* params);
  void glGetVertexAttribdv_Hook(GLuint index, GLenum pname, GLdouble* params);
  void glGetVertexAttribfv_Hook(GLuint index, GLenum pname, GLfloat* params);
  void glGetVertexAttribiv_Hook(GLuint index, GLenum pname, GLint* params);
  void glGetVertexAttribPointerv_Hook(GLuint index, GLenum pname, GLvoid** pointer);
  GLboolean glIsProgram_Hook(GLuint program);
  GLboolean glIsShader_Hook(GLuint shader);
  void glLinkProgram_Hook(GLuint program);
  void
  glShaderSource_Hook(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
  void glUseProgram_Hook(GLuint program);
  void glUniform1f_Hook(GLint location, GLfloat v0);
  void glUniform2f_Hook(GLint location, GLfloat v0, GLfloat v1);
  void glUniform3f_Hook(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
  void glUniform4f_Hook(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
  void glUniform1i_Hook(GLint location, GLint v0);
  void glUniform2i_Hook(GLint location, GLint v0, GLint v1);
  void glUniform3i_Hook(GLint location, GLint v0, GLint v1, GLint v2);
  void glUniform4i_Hook(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
  void glUniform1fv_Hook(GLint location, GLsizei count, const GLfloat* value);
  void glUniform2fv_Hook(GLint location, GLsizei count, const GLfloat* value);
  void glUniform3fv_Hook(GLint location, GLsizei count, const GLfloat* value);
  void glUniform4fv_Hook(GLint location, GLsizei count, const GLfloat* value);
  void glUniform1iv_Hook(GLint location, GLsizei count, const GLint* value);
  void glUniform2iv_Hook(GLint location, GLsizei count, const GLint* value);
  void glUniform3iv_Hook(GLint location, GLsizei count, const GLint* value);
  void glUniform4iv_Hook(GLint location, GLsizei count, const GLint* value);
  void
  glUniformMatrix2fv_Hook(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
  void
  glUniformMatrix3fv_Hook(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
  void
  glUniformMatrix4fv_Hook(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
  void glValidateProgram_Hook(GLuint program);
  void glVertexAttrib1d_Hook(GLuint index, GLdouble x);
  void glVertexAttrib1dv_Hook(GLuint index, const GLdouble* v);
  void glVertexAttrib1f_Hook(GLuint index, GLfloat x);
  void glVertexAttrib1fv_Hook(GLuint index, const GLfloat* v);
  void glVertexAttrib1s_Hook(GLuint index, GLshort x);
  void glVertexAttrib1sv_Hook(GLuint index, const GLshort* v);
  void glVertexAttrib2d_Hook(GLuint index, GLdouble x, GLdouble y);
  void glVertexAttrib2dv_Hook(GLuint index, const GLdouble* v);
  void glVertexAttrib2f_Hook(GLuint index, GLfloat x, GLfloat y);
  void glVertexAttrib2fv_Hook(GLuint index, const GLfloat* v);
  void glVertexAttrib2s_Hook(GLuint index, GLshort x, GLshort y);
  void glVertexAttrib2sv_Hook(GLuint index, const GLshort* v);
  void glVertexAttrib3d_Hook(GLuint index, GLdouble x, GLdouble y, GLdouble z);
  void glVertexAttrib3dv_Hook(GLuint index, const GLdouble* v);
  void glVertexAttrib3f_Hook(GLuint index, GLfloat x, GLfloat y, GLfloat z);
  void glVertexAttrib3fv_Hook(GLuint index, const GLfloat* v);
  void glVertexAttrib3s_Hook(GLuint index, GLshort x, GLshort y, GLshort z);
  void glVertexAttrib3sv_Hook(GLuint index, const GLshort* v);
  void glVertexAttrib4Nbv_Hook(GLuint index, const GLbyte* v);
  void glVertexAttrib4Niv_Hook(GLuint index, const GLint* v);
  void glVertexAttrib4Nsv_Hook(GLuint index, const GLshort* v);
  void glVertexAttrib4Nub_Hook(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
  void glVertexAttrib4Nubv_Hook(GLuint index, const GLubyte* v);
  void glVertexAttrib4Nuiv_Hook(GLuint index, const GLuint* v);
  void glVertexAttrib4Nusv_Hook(GLuint index, const GLushort* v);
  void glVertexAttrib4bv_Hook(GLuint index, const GLbyte* v);
  void glVertexAttrib4d_Hook(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
  void glVertexAttrib4dv_Hook(GLuint index, const GLdouble* v);
  void glVertexAttrib4f_Hook(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
  void glVertexAttrib4fv_Hook(GLuint index, const GLfloat* v);
  void glVertexAttrib4iv_Hook(GLuint index, const GLint* v);
  void glVertexAttrib4s_Hook(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
  void glVertexAttrib4sv_Hook(GLuint index, const GLshort* v);
  void glVertexAttrib4ubv_Hook(GLuint index, const GLubyte* v);
  void glVertexAttrib4uiv_Hook(GLuint index, const GLuint* v);
  void glVertexAttrib4usv_Hook(GLuint index, const GLushort* v);
  void glVertexAttribPointer_Hook(
      GLuint index,
      GLint size,
      GLenum type,
      GLboolean normalized,
      GLsizei stride,
      const GLvoid* pointer);

  // GL_VERSION_2_1
  void glUniformMatrix2x3fv_Hook(
      GLint location,
      GLsizei count,
      GLboolean transpose,
      const GLfloat* value);
  void glUniformMatrix3x2fv_Hook(
      GLint location,
      GLsizei count,
      GLboolean transpose,
      const GLfloat* value);
  void glUniformMatrix2x4fv_Hook(
      GLint location,
      GLsizei count,
      GLboolean transpose,
      const GLfloat* value);
  void glUniformMatrix4x2fv_Hook(
      GLint location,
      GLsizei count,
      GLboolean transpose,
      const GLfloat* value);
  void glUniformMatrix3x4fv_Hook(
      GLint location,
      GLsizei count,
      GLboolean transpose,
      const GLfloat* value);
  void glUniformMatrix4x3fv_Hook(
      GLint location,
      GLsizei count,
      GLboolean transpose,
      const GLfloat* value);

  // GL_VERSION_3_0
  void glColorMaski_Hook(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
  void glGetBooleani_v_Hook(GLenum target, GLuint index, GLboolean* data);
  void glGetIntegeri_v_Hook(GLenum target, GLuint index, GLint* data);
  void glEnablei_Hook(GLenum target, GLuint index);
  void glDisablei_Hook(GLenum target, GLuint index);
  GLboolean glIsEnabledi_Hook(GLenum target, GLuint index);
  void glBeginTransformFeedback_Hook(GLenum primitiveMode);
  void glEndTransformFeedback_Hook(void);
  void glBindBufferRange_Hook(
      GLenum target,
      GLuint index,
      GLuint buffer,
      GLintptr offset,
      GLsizeiptr size);
  void glBindBufferBase_Hook(GLenum target, GLuint index, GLuint buffer);
  void glTransformFeedbackVaryings_Hook(
      GLuint program,
      GLsizei count,
      const GLchar** varyings,
      GLenum bufferMode);
  void glGetTransformFeedbackVarying_Hook(
      GLuint program,
      GLuint index,
      GLsizei bufSize,
      GLsizei* length,
      GLsizei* size,
      GLenum* type,
      GLchar* name);
  void glClampColor_Hook(GLenum target, GLenum clamp);
  void glBeginConditionalRender_Hook(GLuint id, GLenum mode);
  void glEndConditionalRender_Hook(void);
  void glVertexAttribIPointer_Hook(
      GLuint index,
      GLint size,
      GLenum type,
      GLsizei stride,
      const GLvoid* pointer);
  void glGetVertexAttribIiv_Hook(GLuint index, GLenum pname, GLint* params);
  void glGetVertexAttribIuiv_Hook(GLuint index, GLenum pname, GLuint* params);
  void glVertexAttribI1i_Hook(GLuint index, GLint x);
  void glVertexAttribI2i_Hook(GLuint index, GLint x, GLint y);
  void glVertexAttribI3i_Hook(GLuint index, GLint x, GLint y, GLint z);
  void glVertexAttribI4i_Hook(GLuint index, GLint x, GLint y, GLint z, GLint w);
  void glVertexAttribI1ui_Hook(GLuint index, GLuint x);
  void glVertexAttribI2ui_Hook(GLuint index, GLuint x, GLuint y);
  void glVertexAttribI3ui_Hook(GLuint index, GLuint x, GLuint y, GLuint z);
  void glVertexAttribI4ui_Hook(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
  void glVertexAttribI1iv_Hook(GLuint index, const GLint* v);
  void glVertexAttribI2iv_Hook(GLuint index, const GLint* v);
  void glVertexAttribI3iv_Hook(GLuint index, const GLint* v);
  void glVertexAttribI4iv_Hook(GLuint index, const GLint* v);
  void glVertexAttribI1uiv_Hook(GLuint index, const GLuint* v);
  void glVertexAttribI2uiv_Hook(GLuint index, const GLuint* v);
  void glVertexAttribI3uiv_Hook(GLuint index, const GLuint* v);
  void glVertexAttribI4uiv_Hook(GLuint index, const GLuint* v);
  void glVertexAttribI4bv_Hook(GLuint index, const GLbyte* v);
  void glVertexAttribI4sv_Hook(GLuint index, const GLshort* v);
  void glVertexAttribI4ubv_Hook(GLuint index, const GLubyte* v);
  void glVertexAttribI4usv_Hook(GLuint index, const GLushort* v);
  void glGetUniformuiv_Hook(GLuint program, GLint location, GLuint* params);
  void glBindFragDataLocation_Hook(GLuint program, GLuint color, const GLchar* name);
  GLint glGetFragDataLocation_Hook(GLuint program, const GLchar* name);
  void glUniform1ui_Hook(GLint location, GLuint v0);
  void glUniform2ui_Hook(GLint location, GLuint v0, GLuint v1);
  void glUniform3ui_Hook(GLint location, GLuint v0, GLuint v1, GLuint v2);
  void glUniform4ui_Hook(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
  void glUniform1uiv_Hook(GLint location, GLsizei count, const GLuint* value);
  void glUniform2uiv_Hook(GLint location, GLsizei count, const GLuint* value);
  void glUniform3uiv_Hook(GLint location, GLsizei count, const GLuint* value);
  void glUniform4uiv_Hook(GLint location, GLsizei count, const GLuint* value);
  void glTexParameterIiv_Hook(GLenum target, GLenum pname, const GLint* params);
  void glTexParameterIuiv_Hook(GLenum target, GLenum pname, const GLuint* params);
  void glGetTexParameterIiv_Hook(GLenum target, GLenum pname, GLint* params);
  void glGetTexParameterIuiv_Hook(GLenum target, GLenum pname, GLuint* params);
  void glClearBufferiv_Hook(GLenum buffer, GLint drawbuffer, const GLint* value);
  void glClearBufferuiv_Hook(GLenum buffer, GLint drawbuffer, const GLuint* value);
  void glClearBufferfv_Hook(GLenum buffer, GLint drawbuffer, const GLfloat* value);
  void glClearBufferfi_Hook(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
  const GLubyte* glGetStringi_Hook(GLenum name, GLuint index);

  // GL_VERSION_3_1
  void glDrawArraysInstanced_Hook(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
  void glDrawElementsInstanced_Hook(
      GLenum mode,
      GLsizei count,
      GLenum type,
      const GLvoid* indices,
      GLsizei primcount);
  void glTexBuffer_Hook(GLenum target, GLenum internalformat, GLuint buffer);
  void glPrimitiveRestartIndex_Hook(GLuint index);

  // GL_VERSION_3_2
  void glGetInteger64i_v_Hook(GLenum target, GLuint index, GLint64* data);
  void glGetBufferParameteri64v_Hook(GLenum target, GLenum pname, GLint64* params);
  void glFramebufferTexture_Hook(GLenum target, GLenum attachment, GLuint texture, GLint level);

  // GL_VERSION_3_3
  void glVertexAttribDivisor_Hook(GLuint index, GLuint divisor);

  // GL_VERSION_4_0
  void glMinSampleShading_Hook(GLclampf value);
  void glBlendEquationi_Hook(GLuint buf, GLenum mode);
  void glBlendEquationSeparatei_Hook(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
  void glBlendFunci_Hook(GLuint buf, GLenum src, GLenum dst);
  void glBlendFuncSeparatei_Hook(
      GLuint buf,
      GLenum srcRGB,
      GLenum dstRGB,
      GLenum srcAlpha,
      GLenum dstAlpha);

  // GL_AMD_debug_output
  void glDebugMessageEnableAMD_Hook(
      GLenum category,
      GLenum severity,
      GLsizei count,
      const GLuint* ids,
      GLboolean enabled);
  void glDebugMessageInsertAMD_Hook(
      GLenum category,
      GLenum severity,
      GLuint id,
      GLsizei length,
      const GLchar* buf);
  void glDebugMessageCallbackAMD_Hook(GLDEBUGPROCAMD callback, GLvoid* userParam);
  GLuint glGetDebugMessageLogAMD_Hook(
      GLuint count,
      GLsizei bufsize,
      GLenum* categories,
      GLuint* severities,
      GLuint* ids,
      GLsizei* lengths,
      GLchar* message);

#if defined(GLE_CGL_ENABLED)
  // GL_APPLE_element_array
  void glElementPointerAPPLE_Hook(GLenum type, const GLvoid* pointer);
  void glDrawElementArrayAPPLE_Hook(GLenum mode, GLint first, GLsizei count);
  void glDrawRangeElementArrayAPPLE_Hook(
      GLenum mode,
      GLuint start,
      GLuint end,
      GLint first,
      GLsizei count);
  void glMultiDrawElementArrayAPPLE_Hook(
      GLenum mode,
      const GLint* first,
      const GLsizei* count,
      GLsizei primcount);
  void glMultiDrawRangeElementArrayAPPLE_Hook(
      GLenum mode,
      GLuint start,
      GLuint end,
      const GLint* first,
      const GLsizei* count,
      GLsizei primcount);

  // GL_APPLE_fence
  void glGenFencesAPPLE_Hook(GLsizei n, GLuint* fences);
  void glDeleteFencesAPPLE_Hook(GLsizei n, const GLuint* fences);
  void glSetFenceAPPLE_Hook(GLuint fence);
  GLboolean glIsFenceAPPLE_Hook(GLuint fence);
  GLboolean glTestFenceAPPLE_Hook(GLuint fence);
  void glFinishFenceAPPLE_Hook(GLuint fence);
  GLboolean glTestObjectAPPLE_Hook(GLenum object, GLuint name);
  void glFinishObjectAPPLE_Hook(GLenum object, GLint name);

  // GL_APPLE_flush_buffer_range
  void glBufferParameteriAPPLE_Hook(GLenum target, GLenum pname, GLint param);
  void glFlushMappedBufferRangeAPPLE_Hook(GLenum target, GLintptr offset, GLsizeiptr size);

  // GL_APPLE_object_purgeable
  GLenum glObjectPurgeableAPPLE_Hook(GLenum objectType, GLuint name, GLenum option);
  GLenum glObjectUnpurgeableAPPLE_Hook(GLenum objectType, GLuint name, GLenum option);
  void
  glGetObjectParameterivAPPLE_Hook(GLenum objectType, GLuint name, GLenum pname, GLint* params);

  // GL_APPLE_texture_range
  void glTextureRangeAPPLE_Hook(GLenum target, GLsizei length, const GLvoid* pointer);
  void glGetTexParameterPointervAPPLE_Hook(GLenum target, GLenum pname, GLvoid** params);

  // GL_APPLE_vertex_array_object
  void glBindVertexArrayAPPLE_Hook(GLuint array);
  void glDeleteVertexArraysAPPLE_Hook(GLsizei n, const GLuint* arrays);
  void glGenVertexArraysAPPLE_Hook(GLsizei n, GLuint* arrays);
  GLboolean glIsVertexArrayAPPLE_Hook(GLuint array);

  // GL_APPLE_vertex_array_range
  void glVertexArrayRangeAPPLE_Hook(GLsizei length, GLvoid* pointer);
  void glFlushVertexArrayRangeAPPLE_Hook(GLsizei length, GLvoid* pointer);
  void glVertexArrayParameteriAPPLE_Hook(GLenum pname, GLint param);

  // GL_APPLE_vertex_program_evaluators
  void glEnableVertexAttribAPPLE_Hook(GLuint index, GLenum pname);
  void glDisableVertexAttribAPPLE_Hook(GLuint index, GLenum pname);
  GLboolean glIsVertexAttribEnabledAPPLE_Hook(GLuint index, GLenum pname);
  void glMapVertexAttrib1dAPPLE_Hook(
      GLuint index,
      GLuint size,
      GLdouble u1,
      GLdouble u2,
      GLint stride,
      GLint order,
      const GLdouble* points);
  void glMapVertexAttrib1fAPPLE_Hook(
      GLuint index,
      GLuint size,
      GLfloat u1,
      GLfloat u2,
      GLint stride,
      GLint order,
      const GLfloat* points);
  void glMapVertexAttrib2dAPPLE_Hook(
      GLuint index,
      GLuint size,
      GLdouble u1,
      GLdouble u2,
      GLint ustride,
      GLint uorder,
      GLdouble v1,
      GLdouble v2,
      GLint vstride,
      GLint vorder,
      const GLdouble* points);
  void glMapVertexAttrib2fAPPLE_Hook(
      GLuint index,
      GLuint size,
      GLfloat u1,
      GLfloat u2,
      GLint ustride,
      GLint uorder,
      GLfloat v1,
      GLfloat v2,
      GLint vstride,
      GLint vorder,
      const GLfloat* points);
#endif // GLE_CGL_ENABLED

  // GL_ARB_copy_buffer
  void glCopyBufferSubData_Hook(
      GLenum readtarget,
      GLenum writetarget,
      GLintptr readoffset,
      GLintptr writeoffset,
      GLsizeiptr size);

  // GL_ARB_debug_output
  void glDebugMessageControlARB_Hook(
      GLenum source,
      GLenum type,
      GLenum severity,
      GLsizei count,
      const GLuint* ids,
      GLboolean enabled);
  void glDebugMessageInsertARB_Hook(
      GLenum source,
      GLenum type,
      GLuint id,
      GLenum severity,
      GLsizei length,
      const GLchar* buf);
  void glDebugMessageCallbackARB_Hook(GLDEBUGPROCARB callback, const GLvoid* userParam);
  GLuint glGetDebugMessageLogARB_Hook(
      GLuint count,
      GLsizei bufsize,
      GLenum* sources,
      GLenum* types,
      GLuint* ids,
      GLenum* severities,
      GLsizei* lengths,
      GLchar* messageLog);

  // GL_ARB_ES2_compatibility
  void glReleaseShaderCompiler_Hook();
  void glShaderBinary_Hook(
      GLsizei count,
      const GLuint* shaders,
      GLenum binaryformat,
      const GLvoid* binary,
      GLsizei length);
  void glGetShaderPrecisionFormat_Hook(
      GLenum shadertype,
      GLenum precisiontype,
      GLint* range,
      GLint* precision);
  void glDepthRangef_Hook(GLclampf n, GLclampf f);
  void glClearDepthf_Hook(GLclampf d);

  // GL_ARB_framebuffer_object
  GLboolean glIsRenderbuffer_Hook(GLuint renderbuffer);
  void glBindRenderbuffer_Hook(GLenum target, GLuint renderbuffer);
  void glDeleteRenderbuffers_Hook(GLsizei n, const GLuint* renderbuffers);
  void glGenRenderbuffers_Hook(GLsizei n, GLuint* renderbuffers);
  void
  glRenderbufferStorage_Hook(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
  void glGetRenderbufferParameteriv_Hook(GLenum target, GLenum pname, GLint* params);
  GLboolean glIsFramebuffer_Hook(GLuint framebuffer);
  void glBindFramebuffer_Hook(GLenum target, GLuint framebuffer);
  void glDeleteFramebuffers_Hook(GLsizei n, const GLuint* framebuffers);
  void glGenFramebuffers_Hook(GLsizei n, GLuint* framebuffers);
  GLenum glCheckFramebufferStatus_Hook(GLenum target);
  void glFramebufferTexture1D_Hook(
      GLenum target,
      GLenum attachment,
      GLenum textarget,
      GLuint texture,
      GLint level);
  void glFramebufferTexture2D_Hook(
      GLenum target,
      GLenum attachment,
      GLenum textarget,
      GLuint texture,
      GLint level);
  void glFramebufferTexture3D_Hook(
      GLenum target,
      GLenum attachment,
      GLenum textarget,
      GLuint texture,
      GLint level,
      GLint zoffset);
  void glFramebufferRenderbuffer_Hook(
      GLenum target,
      GLenum attachment,
      GLenum renderbuffertarget,
      GLuint renderbuffer);
  void glGetFramebufferAttachmentParameteriv_Hook(
      GLenum target,
      GLenum attachment,
      GLenum pname,
      GLint* params);
  void glGenerateMipmap_Hook(GLenum target);
  void glBlitFramebuffer_Hook(
      GLint srcX0,
      GLint srcY0,
      GLint srcX1,
      GLint srcY1,
      GLint dstX0,
      GLint dstY0,
      GLint dstX1,
      GLint dstY1,
      GLbitfield mask,
      GLenum filter);
  void glRenderbufferStorageMultisample_Hook(
      GLenum target,
      GLsizei samples,
      GLenum internalformat,
      GLsizei width,
      GLsizei height);
  void glFramebufferTextureLayer_Hook(
      GLenum target,
      GLenum attachment,
      GLuint texture,
      GLint level,
      GLint layer);

  // GL_ARB_texture_multisample
  void glTexImage2DMultisample_Hook(
      GLenum target,
      GLsizei samples,
      GLint internalformat,
      GLsizei width,
      GLsizei height,
      GLboolean fixedsamplelocations);
  void glTexImage3DMultisample_Hook(
      GLenum target,
      GLsizei samples,
      GLint internalformat,
      GLsizei width,
      GLsizei height,
      GLsizei depth,
      GLboolean fixedsamplelocations);
  void glGetMultisamplefv_Hook(GLenum pname, GLuint index, GLfloat* val);
  void glSampleMaski_Hook(GLuint index, GLbitfield mask);

  // GL_ARB_texture_storage
  void glTexStorage1D_Hook(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
  void glTexStorage2D_Hook(
      GLenum target,
      GLsizei levels,
      GLenum internalformat,
      GLsizei width,
      GLsizei height);
  void glTexStorage3D_Hook(
      GLenum target,
      GLsizei levels,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLsizei depth);
  void glTextureStorage1DEXT_Hook(
      GLuint texture,
      GLenum target,
      GLsizei levels,
      GLenum internalformat,
      GLsizei width);
  void glTextureStorage2DEXT_Hook(
      GLuint texture,
      GLenum target,
      GLsizei levels,
      GLenum internalformat,
      GLsizei width,
      GLsizei height);
  void glTextureStorage3DEXT_Hook(
      GLuint texture,
      GLenum target,
      GLsizei levels,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLsizei depth);

  // GL_ARB_texture_storage_multisample
  void glTexStorage2DMultisample_Hook(
      GLenum target,
      GLsizei samples,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLboolean fixedsamplelocations);
  void glTexStorage3DMultisample_Hook(
      GLenum target,
      GLsizei samples,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLsizei depth,
      GLboolean fixedsamplelocations);
  void glTextureStorage2DMultisampleEXT_Hook(
      GLuint texture,
      GLenum target,
      GLsizei samples,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLboolean fixedsamplelocations);
  void glTextureStorage3DMultisampleEXT_Hook(
      GLuint texture,
      GLenum target,
      GLsizei samples,
      GLenum internalformat,
      GLsizei width,
      GLsizei height,
      GLsizei depth,
      GLboolean fixedsamplelocations);

  // GL_ARB_timer_query
  void glQueryCounter_Hook(GLuint id, GLenum target);
  void glGetQueryObjecti64v_Hook(GLuint id, GLenum pname, GLint64* params);
  void glGetQueryObjectui64v_Hook(GLuint id, GLenum pname, GLuint64* params);

  // GL_ARB_vertex_array_object
  void glBindVertexArray_Hook(GLuint array);
  void glDeleteVertexArrays_Hook(GLsizei n, const GLuint* arrays);
  void glGenVertexArrays_Hook(GLsizei n, GLuint* arrays);
  GLboolean glIsVertexArray_Hook(GLuint array);

  // GL_EXT_draw_buffers2
  void glColorMaskIndexedEXT_Hook(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
  void glGetBooleanIndexedvEXT_Hook(GLenum target, GLuint index, GLboolean* data);
  void glGetIntegerIndexedvEXT_Hook(GLenum target, GLuint index, GLint* data);
  void glEnableIndexedEXT_Hook(GLenum target, GLuint index);
  void glDisableIndexedEXT_Hook(GLenum target, GLuint index);
  GLboolean glIsEnabledIndexedEXT_Hook(GLenum target, GLuint index);

  // GL_KHR_debug
  void glDebugMessageControl_Hook(
      GLenum source,
      GLenum type,
      GLenum severity,
      GLsizei count,
      const GLuint* ids,
      GLboolean enabled);
  void glDebugMessageInsert_Hook(
      GLenum source,
      GLenum type,
      GLuint id,
      GLenum severity,
      GLsizei length,
      const char* buf);
  void glDebugMessageCallback_Hook(GLDEBUGPROC callback, const void* userParam);
  GLuint glGetDebugMessageLog_Hook(
      GLuint count,
      GLsizei bufSize,
      GLenum* sources,
      GLenum* types,
      GLuint* ids,
      GLenum* severities,
      GLsizei* lengths,
      char* messageLog);
  void glPushDebugGroup_Hook(GLenum source, GLuint id, GLsizei length, const char* message);
  void glPopDebugGroup_Hook(void);
  void glObjectLabel_Hook(GLenum identifier, GLuint name, GLsizei length, const char* label);
  void glGetObjectLabel_Hook(
      GLenum identifier,
      GLuint name,
      GLsizei bufSize,
      GLsizei* length,
      char* label);
  void glObjectPtrLabel_Hook(void* ptr, GLsizei length, const char* label);
  void glGetObjectPtrLabel_Hook(void* ptr, GLsizei bufSize, GLsizei* length, char* label);

  // GL_WIN_swap_hint
  void glAddSwapHintRectWIN_Hook(GLint x, GLint y, GLsizei width, GLsizei height);

#if defined(GLE_WGL_ENABLED)
  void PostWGLHook(const char* functionName);

  // WGL
  /* Hooking of these is currently disabled.
  BOOL  wglCopyContext_Hook(HGLRC, HGLRC, UINT);
  HGLRC wglCreateContext_Hook(HDC);
  HGLRC wglCreateLayerContext_Hook(HDC, int);
  BOOL  wglDeleteContext_Hook(HGLRC);
  HGLRC wglGetCurrentContext_Hook(VOID);
  HDC   wglGetCurrentDC_Hook(VOID);
  PROC  wglGetProcAddress_Hook(LPCSTR);
  BOOL  wglMakeCurrent_Hook(HDC, HGLRC);
  BOOL  wglShareLists_Hook(HGLRC, HGLRC);
  BOOL  wglUseFontBitmapsA_Hook(HDC, DWORD, DWORD, DWORD);
  BOOL  wglUseFontBitmapsW_Hook(HDC, DWORD, DWORD, DWORD);
  BOOL  wglUseFontOutlinesA_Hook(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT);
  BOOL  wglUseFontOutlinesW_Hook(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT);
  BOOL  wglDescribeLayerPlane_Hook(HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR);
  int   wglSetLayerPaletteEntries_Hook(HDC, int, int, int, CONST COLORREF *);
  int   wglGetLayerPaletteEntries_Hook(HDC, int, int, int, COLORREF *);
  BOOL  wglRealizeLayerPalette_Hook(HDC, int, BOOL);
  BOOL  wglSwapLayerBuffers_Hook(HDC, UINT);
  DWORD wglSwapMultipleBuffers_Hook(UINT, CONST WGLSWAP *);
  */

  // WGL_ARB_buffer_region
  HANDLE wglCreateBufferRegionARB_Hook(HDC hDC, int iLayerPlane, UINT uType);
  VOID wglDeleteBufferRegionARB_Hook(HANDLE hRegion);
  BOOL wglSaveBufferRegionARB_Hook(HANDLE hRegion, int x, int y, int width, int height);
  BOOL wglRestoreBufferRegionARB_Hook(
      HANDLE hRegion,
      int x,
      int y,
      int width,
      int height,
      int xSrc,
      int ySrc);

  // WGL_ARB_extensions_string
  const char* wglGetExtensionsStringARB_Hook(HDC hdc);

  // WGL_ARB_pixel_format
  BOOL wglGetPixelFormatAttribivARB_Hook(
      HDC hdc,
      int iPixelFormat,
      int iLayerPlane,
      UINT nAttributes,
      const int* piAttributes,
      int* piValues);
  BOOL wglGetPixelFormatAttribfvARB_Hook(
      HDC hdc,
      int iPixelFormat,
      int iLayerPlane,
      UINT nAttributes,
      const int* piAttributes,
      FLOAT* pfValues);
  BOOL wglChoosePixelFormatARB_Hook(
      HDC hdc,
      const int* piAttribIList,
      const FLOAT* pfAttribFList,
      UINT nMaxFormats,
      int* piFormats,
      UINT* nNumFormats);

  // WGL_ARB_make_current_read
  BOOL wglMakeContextCurrentARB_Hook(HDC hDrawDC, HDC hReadDC, HGLRC hglrc);
  HDC wglGetCurrentReadDCARB_Hook(void);

  // WGL_ARB_pbuffer
  HPBUFFERARB wglCreatePbufferARB_Hook(
      HDC hDC,
      int iPixelFormat,
      int iWidth,
      int iHeight,
      const int* piAttribList);
  HDC wglGetPbufferDCARB_Hook(HPBUFFERARB hPbuffer);
  int wglReleasePbufferDCARB_Hook(HPBUFFERARB hPbuffer, HDC hDC);
  BOOL wglDestroyPbufferARB_Hook(HPBUFFERARB hPbuffer);
  BOOL wglQueryPbufferARB_Hook(HPBUFFERARB hPbuffer, int iAttribute, int* piValue);

  // WGL_ARB_render_texture
  BOOL wglBindTexImageARB_Hook(HPBUFFERARB hPbuffer, int iBuffer);
  BOOL wglReleaseTexImageARB_Hook(HPBUFFERARB hPbuffer, int iBuffer);
  BOOL wglSetPbufferAttribARB_Hook(HPBUFFERARB hPbuffer, const int* piAttribList);

  // WGL_NV_present_video
  int wglEnumerateVideoDevicesNV_Hook(HDC hDC, HVIDEOOUTPUTDEVICENV* phDeviceList);
  BOOL wglBindVideoDeviceNV_Hook(
      HDC hDC,
      unsigned int uVideoSlot,
      HVIDEOOUTPUTDEVICENV hVideoDevice,
      const int* piAttribList);
  BOOL wglQueryCurrentContextNV_Hook(int iAttribute, int* piValue);

  // WGL_ARB_create_context
  HGLRC wglCreateContextAttribsARB_Hook(HDC hDC, HGLRC hShareContext, const int* attribList);

  // WGL_EXT_extensions_string
  const char* wglGetExtensionsStringEXT_Hook();

  // WGL_EXT_swap_control
  BOOL wglSwapIntervalEXT_Hook(int interval);
  int wglGetSwapIntervalEXT_Hook();

  // WGL_OML_sync_control
  BOOL wglGetSyncValuesOML_Hook(HDC hdc, INT64* ust, INT64* msc, INT64* sbc);
  BOOL wglGetMscRateOML_Hook(HDC hdc, INT32* numerator, INT32* denominator);
  INT64 wglSwapBuffersMscOML_Hook(HDC hdc, INT64 target_msc, INT64 divisor, INT64 remainder);
  INT64 wglSwapLayerBuffersMscOML_Hook(
      HDC hdc,
      int fuPlanes,
      INT64 target_msc,
      INT64 divisor,
      INT64 remainder);
  BOOL wglWaitForMscOML_Hook(
      HDC hdc,
      INT64 target_msc,
      INT64 divisor,
      INT64 remainder,
      INT64* ust,
      INT64* msc,
      INT64* sbc);
  BOOL wglWaitForSbcOML_Hook(HDC hdc, INT64 target_sbc, INT64* ust, INT64* msc, INT64* sbc);

  // WGL_NV_video_output
  BOOL wglGetVideoDeviceNV_Hook(HDC hDC, int numDevices, HPVIDEODEV* hVideoDevice);
  BOOL wglReleaseVideoDeviceNV_Hook(HPVIDEODEV hVideoDevice);
  BOOL wglBindVideoImageNV_Hook(HPVIDEODEV hVideoDevice, HPBUFFERARB hPbuffer, int iVideoBuffer);
  BOOL wglReleaseVideoImageNV_Hook(HPBUFFERARB hPbuffer, int iVideoBuffer);
  BOOL wglSendPbufferToVideoNV_Hook(
      HPBUFFERARB hPbuffer,
      int iBufferType,
      unsigned long* pulCounterPbuffer,
      BOOL bBlock);
  BOOL wglGetVideoInfoNV_Hook(
      HPVIDEODEV hpVideoDevice,
      unsigned long* pulCounterOutputPbuffer,
      unsigned long* pulCounterOutputVideo);

  // WGL_NV_swap_group
  BOOL wglJoinSwapGroupNV_Hook(HDC hDC, GLuint group);
  BOOL wglBindSwapBarrierNV_Hook(GLuint group, GLuint barrier);
  BOOL wglQuerySwapGroupNV_Hook(HDC hDC, GLuint* group, GLuint* barrier);
  BOOL wglQueryMaxSwapGroupsNV_Hook(HDC hDC, GLuint* maxGroups, GLuint* maxBarriers);
  BOOL wglQueryFrameCountNV_Hook(HDC hDC, GLuint* count);
  BOOL wglResetFrameCountNV_Hook(HDC hDC);

  // WGL_NV_video_capture
  BOOL wglBindVideoCaptureDeviceNV_Hook(UINT uVideoSlot, HVIDEOINPUTDEVICENV hDevice);
  UINT wglEnumerateVideoCaptureDevicesNV_Hook(HDC hDc, HVIDEOINPUTDEVICENV* phDeviceList);
  BOOL wglLockVideoCaptureDeviceNV_Hook(HDC hDc, HVIDEOINPUTDEVICENV hDevice);
  BOOL wglQueryVideoCaptureDeviceNV_Hook(
      HDC hDc,
      HVIDEOINPUTDEVICENV hDevice,
      int iAttribute,
      int* piValue);
  BOOL wglReleaseVideoCaptureDeviceNV_Hook(HDC hDc, HVIDEOINPUTDEVICENV hDevice);

  // WGL_NV_copy_image
  BOOL wglCopyImageSubDataNV_Hook(
      HGLRC hSrcRC,
      GLuint srcName,
      GLenum srcTarget,
      GLint srcLevel,
      GLint srcX,
      GLint srcY,
      GLint srcZ,
      HGLRC hDstRC,
      GLuint dstName,
      GLenum dstTarget,
      GLint dstLevel,
      GLint dstX,
      GLint dstY,
      GLint dstZ,
      GLsizei width,
      GLsizei height,
      GLsizei depth);

  // WGL_NV_DX_interop
  BOOL wglDXSetResourceShareHandleNV_Hook(void* dxObject, HANDLE shareHandle);
  HANDLE wglDXOpenDeviceNV_Hook(void* dxDevice);
  BOOL wglDXCloseDeviceNV_Hook(HANDLE hDevice);
  HANDLE wglDXRegisterObjectNV_Hook(
      HANDLE hDevice,
      void* dxObject,
      GLuint name,
      GLenum type,
      GLenum access);
  BOOL wglDXUnregisterObjectNV_Hook(HANDLE hDevice, HANDLE hObject);
  BOOL wglDXObjectAccessNV_Hook(HANDLE hObject, GLenum access);
  BOOL wglDXLockObjectsNV_Hook(HANDLE hDevice, GLint count, HANDLE* hObjects);
  BOOL wglDXUnlockObjectsNV_Hook(HANDLE hDevice, GLint count, HANDLE* hObjects);
#endif // GLE_WGL_ENABLED

#if defined(GLE_GLX_ENABLED)
  void PostGLXHook(const char* functionName);

  // GLX_VERSION_1_0
  // GLX_VERSION_1_1
  // We don't currently do hooking of these.

  // GLX_VERSION_1_2
  ::Display* glXGetCurrentDisplay_Hook(void);

  // GLX_VERSION_1_3
  GLXFBConfig*
  glXChooseFBConfig_Hook(::Display* dpy, int screen, const int* attrib_list, int* nelements);
  GLXContext glXCreateNewContext_Hook(
      ::Display* dpy,
      GLXFBConfig config,
      int render_type,
      GLXContext share_list,
      Bool direct);
  GLXPbuffer glXCreatePbuffer_Hook(::Display* dpy, GLXFBConfig config, const int* attrib_list);
  GLXPixmap
  glXCreatePixmap_Hook(::Display* dpy, GLXFBConfig config, Pixmap pixmap, const int* attrib_list);
  GLXWindow
  glXCreateWindow_Hook(::Display* dpy, GLXFBConfig config, Window win, const int* attrib_list);
  void glXDestroyPbuffer_Hook(::Display* dpy, GLXPbuffer pbuf);
  void glXDestroyPixmap_Hook(::Display* dpy, GLXPixmap pixmap);
  void glXDestroyWindow_Hook(::Display* dpy, GLXWindow win);
  GLXDrawable glXGetCurrentReadDrawable_Hook(void);
  int glXGetFBConfigAttrib_Hook(::Display* dpy, GLXFBConfig config, int attribute, int* value);
  GLXFBConfig* glXGetFBConfigs_Hook(::Display* dpy, int screen, int* nelements);
  void glXGetSelectedEvent_Hook(::Display* dpy, GLXDrawable draw, unsigned long* event_mask);
  XVisualInfo* glXGetVisualFromFBConfig_Hook(::Display* dpy, GLXFBConfig config);
  Bool glXMakeContextCurrent_Hook(
      ::Display* display,
      GLXDrawable draw,
      GLXDrawable read,
      GLXContext ctx);
  int glXQueryContext_Hook(::Display* dpy, GLXContext ctx, int attribute, int* value);
  void glXQueryDrawable_Hook(::Display* dpy, GLXDrawable draw, int attribute, unsigned int* value);
  void glXSelectEvent_Hook(::Display* dpy, GLXDrawable draw, unsigned long event_mask);

  // GLX_VERSION_1_4
  // We don't do hooking of this.

  // GLX_ARB_create_context
  GLXContext glXCreateContextAttribsARB_Hook(
      Display* dpy,
      GLXFBConfig config,
      GLXContext share_context,
      Bool direct,
      const int* attrib_list);

  // GLX_EXT_swap_control
  void glXSwapIntervalEXT_Hook(::Display* dpy, GLXDrawable drawable, int interval);

  // GLX_OML_sync_control
  Bool glXGetMscRateOML_Hook(
      ::Display* dpy,
      GLXDrawable drawable,
      int32_t* numerator,
      int32_t* denominator);
  Bool glXGetSyncValuesOML_Hook(
      ::Display* dpy,
      GLXDrawable drawable,
      int64_t* ust,
      int64_t* msc,
      int64_t* sbc);
  int64_t glXSwapBuffersMscOML_Hook(
      ::Display* dpy,
      GLXDrawable drawable,
      int64_t target_msc,
      int64_t divisor,
      int64_t remainder);
  Bool glXWaitForMscOML_Hook(
      ::Display* dpy,
      GLXDrawable drawable,
      int64_t target_msc,
      int64_t divisor,
      int64_t remainder,
      int64_t* ust,
      int64_t* msc,
      int64_t* sbc);
  Bool glXWaitForSbcOML_Hook(
      ::Display* dpy,
      GLXDrawable drawable,
      int64_t target_sbc,
      int64_t* ust,
      int64_t* msc,
      int64_t* sbc);

  // GLX_MESA_swap_control
  int glXGetSwapIntervalMESA_Hook();
  int glXSwapIntervalMESA_Hook(unsigned int interval);

#endif // GLE_GLX_ENABLED

#endif // #if defined(GLE_HOOKING_ENABLED)

  // GL_VERSION_1_1
  // These are not represented by function pointers.

  // GL_VERSION_1_2
  GLE_IMPL(GLE_DISABLE, PFNGLCOPYTEXSUBIMAGE3DPROC, glCopyTexSubImage3D);
  GLE_IMPL(GLE_DISABLE, PFNGLDRAWRANGEELEMENTSPROC, glDrawRangeElements);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXIMAGE3DPROC, glTexImage3D);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXSUBIMAGE3DPROC, glTexSubImage3D);

  // GL_VERSION_1_2 deprecated functions
  /* Not currently supported
  GLE_IMPL(GLE_DISABLE, PFNGLCOLORTABLEPROC, glColorTable);
  GLE_IMPL(GLE_DISABLE, PFNGLCOLORTABLEPARAMETERFVPROC, glColorTableParameterfv);
  GLE_IMPL(GLE_DISABLE, PFNGLCOLORTABLEPARAMETERIVPROC, glColorTableParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLCOPYCOLORTABLEPROC, glCopyColorTable);
  GLE_IMPL(GLE_DISABLE, PFNGLGETCOLORTABLEPROC, glGetColorTable);
  GLE_IMPL(GLE_DISABLE, PFNGLGETCOLORTABLEPARAMETERFVPROC, glGetColorTableParameterfv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETCOLORTABLEPARAMETERIVPROC, glGetColorTableParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLCOLORSUBTABLEPROC, glColorSubTable);
  GLE_IMPL(GLE_DISABLE, PFNGLCOPYCOLORSUBTABLEPROC, glCopyColorSubTable);
  GLE_IMPL(GLE_DISABLE, PFNGLCONVOLUTIONFILTER1DPROC, glConvolutionFilter1D);
  GLE_IMPL(GLE_DISABLE, PFNGLCONVOLUTIONFILTER2DPROC, glConvolutionFilter2D);
  GLE_IMPL(GLE_DISABLE, PFNGLCONVOLUTIONPARAMETERFPROC, glConvolutionParameterf);
  GLE_IMPL(GLE_DISABLE, PFNGLCONVOLUTIONPARAMETERFVPROC, glConvolutionParameterfv);
  GLE_IMPL(GLE_DISABLE, PFNGLCONVOLUTIONPARAMETERIPROC, glConvolutionParameteri);
  GLE_IMPL(GLE_DISABLE, PFNGLCONVOLUTIONPARAMETERIVPROC, glConvolutionParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLCOPYCONVOLUTIONFILTER1DPROC, glCopyConvolutionFilter1D);
  GLE_IMPL(GLE_DISABLE, PFNGLCOPYCONVOLUTIONFILTER2DPROC, glCopyConvolutionFilter2D);
  GLE_IMPL(GLE_DISABLE, PFNGLGETCONVOLUTIONFILTERPROC, glGetConvolutionFilter);
  GLE_IMPL(GLE_DISABLE, PFNGLGETCONVOLUTIONPARAMETERFVPROC, glGetConvolutionParameterfv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETCONVOLUTIONPARAMETERIVPROC, glGetConvolutionParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETSEPARABLEFILTERPROC, glGetSeparableFilter);
  GLE_IMPL(GLE_DISABLE, PFNGLSEPARABLEFILTER2DPROC, glSeparableFilter2D);
  GLE_IMPL(GLE_DISABLE, PFNGLGETHISTOGRAMPROC, glGetHistogram);
  GLE_IMPL(GLE_DISABLE, PFNGLGETHISTOGRAMPARAMETERFVPROC, glGetHistogramParameterfv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETHISTOGRAMPARAMETERIVPROC, glGetHistogramParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETMINMAXPROC, glGetMinmax);
  GLE_IMPL(GLE_DISABLE, PFNGLGETMINMAXPARAMETERFVPROC, glGetMinmaxParameterfv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETMINMAXPARAMETERIVPROC, glGetMinmaxParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLHISTOGRAMPROC, glHistogram);
  GLE_IMPL(GLE_DISABLE, PFNGLMINMAXPROC, glMinmax);
  GLE_IMPL(GLE_DISABLE, PFNGLRESETHISTOGRAMPROC, glResetHistogram);
  GLE_IMPL(GLE_DISABLE, PFNGLRESETMINMAXPROC, glResetMinmax);
  */

  // GL_VERSION_1_3
  GLE_IMPL(GLE_ENABLE, PFNGLACTIVETEXTUREPROC, glActiveTexture);
  GLE_IMPL(GLE_DISABLE, PFNGLCLIENTACTIVETEXTUREPROC, glClientActiveTexture);
  GLE_IMPL(GLE_DISABLE, PFNGLCOMPRESSEDTEXIMAGE1DPROC, glCompressedTexImage1D);
  GLE_IMPL(GLE_DISABLE, PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D);
  GLE_IMPL(GLE_DISABLE, PFNGLCOMPRESSEDTEXIMAGE3DPROC, glCompressedTexImage3D);
  GLE_IMPL(GLE_DISABLE, PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC, glCompressedTexSubImage1D);
  GLE_IMPL(GLE_DISABLE, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, glCompressedTexSubImage2D);
  GLE_IMPL(GLE_DISABLE, PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC, glCompressedTexSubImage3D);
  GLE_IMPL(GLE_DISABLE, PFNGLGETCOMPRESSEDTEXIMAGEPROC, glGetCompressedTexImage);
  GLE_IMPL(GLE_DISABLE, PFNGLLOADTRANSPOSEMATRIXDPROC, glLoadTransposeMatrixd);
  GLE_IMPL(GLE_DISABLE, PFNGLLOADTRANSPOSEMATRIXFPROC, glLoadTransposeMatrixf);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTTRANSPOSEMATRIXDPROC, glMultTransposeMatrixd);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTTRANSPOSEMATRIXFPROC, glMultTransposeMatrixf);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1DPROC, glMultiTexCoord1d);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1DVPROC, glMultiTexCoord1dv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1FPROC, glMultiTexCoord1f);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1FVPROC, glMultiTexCoord1fv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1IPROC, glMultiTexCoord1i);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1IVPROC, glMultiTexCoord1iv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1SPROC, glMultiTexCoord1s);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD1SVPROC, glMultiTexCoord1sv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2DPROC, glMultiTexCoord2d);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2DVPROC, glMultiTexCoord2dv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2FPROC, glMultiTexCoord2f);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2FVPROC, glMultiTexCoord2fv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2IPROC, glMultiTexCoord2i);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2IVPROC, glMultiTexCoord2iv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2SPROC, glMultiTexCoord2s);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD2SVPROC, glMultiTexCoord2sv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3DPROC, glMultiTexCoord3d);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3DVPROC, glMultiTexCoord3dv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3FPROC, glMultiTexCoord3f);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3FVPROC, glMultiTexCoord3fv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3IPROC, glMultiTexCoord3i);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3IVPROC, glMultiTexCoord3iv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3SPROC, glMultiTexCoord3s);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD3SVPROC, glMultiTexCoord3sv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4DPROC, glMultiTexCoord4d);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4DVPROC, glMultiTexCoord4dv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4FPROC, glMultiTexCoord4f);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4FVPROC, glMultiTexCoord4fv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4IPROC, glMultiTexCoord4i);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4IVPROC, glMultiTexCoord4iv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4SPROC, glMultiTexCoord4s);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTITEXCOORD4SVPROC, glMultiTexCoord4sv);
  GLE_IMPL(GLE_DISABLE, PFNGLSAMPLECOVERAGEPROC, glSampleCoverage);

  // GL_VERSION_1_4
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDCOLORPROC, glBlendColor);
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDEQUATIONPROC, glBlendEquation);
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate);
  GLE_IMPL(GLE_DISABLE, PFNGLFOGCOORDPOINTERPROC, glFogCoordPointer);
  GLE_IMPL(GLE_DISABLE, PFNGLFOGCOORDDPROC, glFogCoordd);
  GLE_IMPL(GLE_DISABLE, PFNGLFOGCOORDDVPROC, glFogCoorddv);
  GLE_IMPL(GLE_DISABLE, PFNGLFOGCOORDFPROC, glFogCoordf);
  GLE_IMPL(GLE_DISABLE, PFNGLFOGCOORDFVPROC, glFogCoordfv);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTIDRAWARRAYSPROC, glMultiDrawArrays);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTIDRAWELEMENTSPROC, glMultiDrawElements);
  GLE_IMPL(GLE_DISABLE, PFNGLPOINTPARAMETERFPROC, glPointParameterf);
  GLE_IMPL(GLE_DISABLE, PFNGLPOINTPARAMETERFVPROC, glPointParameterfv);
  GLE_IMPL(GLE_DISABLE, PFNGLPOINTPARAMETERIPROC, glPointParameteri);
  GLE_IMPL(GLE_DISABLE, PFNGLPOINTPARAMETERIVPROC, glPointParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3BPROC, glSecondaryColor3b);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3BVPROC, glSecondaryColor3bv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3DPROC, glSecondaryColor3d);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3DVPROC, glSecondaryColor3dv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3FPROC, glSecondaryColor3f);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3FVPROC, glSecondaryColor3fv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3IPROC, glSecondaryColor3i);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3IVPROC, glSecondaryColor3iv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3SPROC, glSecondaryColor3s);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3SVPROC, glSecondaryColor3sv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3UBPROC, glSecondaryColor3ub);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3UBVPROC, glSecondaryColor3ubv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3UIPROC, glSecondaryColor3ui);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3UIVPROC, glSecondaryColor3uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3USPROC, glSecondaryColor3us);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLOR3USVPROC, glSecondaryColor3usv);
  GLE_IMPL(GLE_DISABLE, PFNGLSECONDARYCOLORPOINTERPROC, glSecondaryColorPointer);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2DPROC, glWindowPos2d);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2DVPROC, glWindowPos2dv);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2FPROC, glWindowPos2f);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2FVPROC, glWindowPos2fv);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2IPROC, glWindowPos2i);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2IVPROC, glWindowPos2iv);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2SPROC, glWindowPos2s);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS2SVPROC, glWindowPos2sv);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3DPROC, glWindowPos3d);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3DVPROC, glWindowPos3dv);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3FPROC, glWindowPos3f);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3FVPROC, glWindowPos3fv);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3IPROC, glWindowPos3i);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3IVPROC, glWindowPos3iv);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3SPROC, glWindowPos3s);
  GLE_IMPL(GLE_DISABLE, PFNGLWINDOWPOS3SVPROC, glWindowPos3sv);

  // GL_VERSION_1_5
  GLE_IMPL(GLE_DISABLE, PFNGLBEGINQUERYPROC, glBeginQuery);
  GLE_IMPL(GLE_ENABLE, PFNGLBINDBUFFERPROC, glBindBuffer);
  GLE_IMPL(GLE_ENABLE, PFNGLBUFFERDATAPROC, glBufferData);
  GLE_IMPL(GLE_ENABLE, PFNGLBUFFERSUBDATAPROC, glBufferSubData);
  GLE_IMPL(GLE_ENABLE, PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
  GLE_IMPL(GLE_DISABLE, PFNGLDELETEQUERIESPROC, glDeleteQueries);
  GLE_IMPL(GLE_DISABLE, PFNGLENDQUERYPROC, glEndQuery);
  GLE_IMPL(GLE_ENABLE, PFNGLGENBUFFERSPROC, glGenBuffers);
  GLE_IMPL(GLE_DISABLE, PFNGLGENQUERIESPROC, glGenQueries);
  GLE_IMPL(GLE_ENABLE, PFNGLGETBUFFERPARAMETERIVPROC, glGetBufferParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETBUFFERPOINTERVPROC, glGetBufferPointerv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETBUFFERSUBDATAPROC, glGetBufferSubData);
  GLE_IMPL(GLE_DISABLE, PFNGLGETQUERYOBJECTIVPROC, glGetQueryObjectiv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETQUERYOBJECTUIVPROC, glGetQueryObjectuiv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETQUERYIVPROC, glGetQueryiv);
  GLE_IMPL(GLE_DISABLE, PFNGLISBUFFERPROC, glIsBuffer);
  GLE_IMPL(GLE_DISABLE, PFNGLISQUERYPROC, glIsQuery);
  GLE_IMPL(GLE_ENABLE, PFNGLMAPBUFFERPROC, glMapBuffer);
  GLE_IMPL(GLE_ENABLE, PFNGLUNMAPBUFFERPROC, glUnmapBuffer);

  // GL_VERSION_2_0
  GLE_IMPL(GLE_ENABLE, PFNGLATTACHSHADERPROC, glAttachShader);
  GLE_IMPL(GLE_ENABLE, PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDEQUATIONSEPARATEPROC, glBlendEquationSeparate);
  GLE_IMPL(GLE_ENABLE, PFNGLCOMPILESHADERPROC, glCompileShader);
  GLE_IMPL(GLE_ENABLE, PFNGLCREATEPROGRAMPROC, glCreateProgram);
  GLE_IMPL(GLE_ENABLE, PFNGLCREATESHADERPROC, glCreateShader);
  GLE_IMPL(GLE_ENABLE, PFNGLDELETEPROGRAMPROC, glDeleteProgram);
  GLE_IMPL(GLE_ENABLE, PFNGLDELETESHADERPROC, glDeleteShader);
  GLE_IMPL(GLE_DISABLE, PFNGLDETACHSHADERPROC, glDetachShader);
  GLE_IMPL(GLE_ENABLE, PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray);
  GLE_IMPL(GLE_ENABLE, PFNGLDRAWBUFFERSPROC, glDrawBuffers);
  GLE_IMPL(GLE_ENABLE, PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
  GLE_IMPL(GLE_DISABLE, PFNGLGETACTIVEATTRIBPROC, glGetActiveAttrib);
  GLE_IMPL(GLE_DISABLE, PFNGLGETACTIVEUNIFORMPROC, glGetActiveUniform);
  GLE_IMPL(GLE_DISABLE, PFNGLGETATTACHEDSHADERSPROC, glGetAttachedShaders);
  GLE_IMPL(GLE_DISABLE, PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation);
  GLE_IMPL(GLE_ENABLE, PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
  GLE_IMPL(GLE_ENABLE, PFNGLGETPROGRAMIVPROC, glGetProgramiv);
  GLE_IMPL(GLE_ENABLE, PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
  GLE_IMPL(GLE_DISABLE, PFNGLGETSHADERSOURCEPROC, glGetShaderSource);
  GLE_IMPL(GLE_ENABLE, PFNGLGETSHADERIVPROC, glGetShaderiv);
  GLE_IMPL(GLE_ENABLE, PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
  GLE_IMPL(GLE_DISABLE, PFNGLGETUNIFORMFVPROC, glGetUniformfv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETUNIFORMIVPROC, glGetUniformiv);
  GLE_IMPL(GLE_ENABLE, PFNGLGETVERTEXATTRIBPOINTERVPROC, glGetVertexAttribPointerv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETVERTEXATTRIBDVPROC, glGetVertexAttribdv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETVERTEXATTRIBFVPROC, glGetVertexAttribfv);
  GLE_IMPL(GLE_ENABLE, PFNGLGETVERTEXATTRIBIVPROC, glGetVertexAttribiv);
  GLE_IMPL(GLE_DISABLE, PFNGLISPROGRAMPROC, glIsProgram);
  GLE_IMPL(GLE_DISABLE, PFNGLISSHADERPROC, glIsShader);
  GLE_IMPL(GLE_ENABLE, PFNGLLINKPROGRAMPROC, glLinkProgram);
  GLE_IMPL(GLE_ENABLE, PFNGLSHADERSOURCEPROC, glShaderSource);
  GLE_IMPL(GLE_DISABLE, PFNGLSTENCILFUNCSEPARATEPROC, glStencilFuncSeparate);
  GLE_IMPL(GLE_DISABLE, PFNGLSTENCILMASKSEPARATEPROC, glStencilMaskSeparate);
  GLE_IMPL(GLE_DISABLE, PFNGLSTENCILOPSEPARATEPROC, glStencilOpSeparate);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM1FPROC, glUniform1f);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM1FVPROC, glUniform1fv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM1IPROC, glUniform1i);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM1IVPROC, glUniform1iv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM2FPROC, glUniform2f);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM2FVPROC, glUniform2fv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM2IPROC, glUniform2i);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM2IVPROC, glUniform2iv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM3FPROC, glUniform3f);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM3FVPROC, glUniform3fv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM3IPROC, glUniform3i);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM3IVPROC, glUniform3iv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM4FPROC, glUniform4f);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM4FVPROC, glUniform4fv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM4IPROC, glUniform4i);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORM4IVPROC, glUniform4iv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORMMATRIX2FVPROC, glUniformMatrix2fv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORMMATRIX3FVPROC, glUniformMatrix3fv);
  GLE_IMPL(GLE_ENABLE, PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);
  GLE_IMPL(GLE_ENABLE, PFNGLUSEPROGRAMPROC, glUseProgram);
  GLE_IMPL(GLE_DISABLE, PFNGLVALIDATEPROGRAMPROC, glValidateProgram);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB1DPROC, glVertexAttrib1d);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB1DVPROC, glVertexAttrib1dv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB1FPROC, glVertexAttrib1f);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB1FVPROC, glVertexAttrib1fv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB1SPROC, glVertexAttrib1s);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB1SVPROC, glVertexAttrib1sv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB2DPROC, glVertexAttrib2d);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB2DVPROC, glVertexAttrib2dv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB2FPROC, glVertexAttrib2f);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB2FVPROC, glVertexAttrib2fv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB2SPROC, glVertexAttrib2s);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB2SVPROC, glVertexAttrib2sv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB3DPROC, glVertexAttrib3d);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB3DVPROC, glVertexAttrib3dv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB3FPROC, glVertexAttrib3f);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB3FVPROC, glVertexAttrib3fv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB3SPROC, glVertexAttrib3s);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB3SVPROC, glVertexAttrib3sv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4NBVPROC, glVertexAttrib4Nbv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4NIVPROC, glVertexAttrib4Niv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4NSVPROC, glVertexAttrib4Nsv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4NUBPROC, glVertexAttrib4Nub);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4NUBVPROC, glVertexAttrib4Nubv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4NUIVPROC, glVertexAttrib4Nuiv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4NUSVPROC, glVertexAttrib4Nusv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4BVPROC, glVertexAttrib4bv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4DPROC, glVertexAttrib4d);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4DVPROC, glVertexAttrib4dv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4FPROC, glVertexAttrib4f);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4FVPROC, glVertexAttrib4fv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4IVPROC, glVertexAttrib4iv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4SPROC, glVertexAttrib4s);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4SVPROC, glVertexAttrib4sv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4UBVPROC, glVertexAttrib4ubv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4UIVPROC, glVertexAttrib4uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIB4USVPROC, glVertexAttrib4usv);
  GLE_IMPL(GLE_ENABLE, PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);

  // GL_VERSION_2_1
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORMMATRIX2X3FVPROC, glUniformMatrix2x3fv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORMMATRIX2X4FVPROC, glUniformMatrix2x4fv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORMMATRIX3X2FVPROC, glUniformMatrix3x2fv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORMMATRIX3X4FVPROC, glUniformMatrix3x4fv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORMMATRIX4X2FVPROC, glUniformMatrix4x2fv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORMMATRIX4X3FVPROC, glUniformMatrix4x3fv);

  // GL_VERSION_3_0
  GLE_IMPL(GLE_DISABLE, PFNGLBEGINCONDITIONALRENDERPROC, glBeginConditionalRender);
  GLE_IMPL(GLE_DISABLE, PFNGLBEGINTRANSFORMFEEDBACKPROC, glBeginTransformFeedback);
  GLE_IMPL(GLE_ENABLE, PFNGLBINDFRAGDATALOCATIONPROC, glBindFragDataLocation);
  GLE_IMPL(GLE_DISABLE, PFNGLCLAMPCOLORPROC, glClampColor);
  GLE_IMPL(GLE_DISABLE, PFNGLCLEARBUFFERFIPROC, glClearBufferfi);
  GLE_IMPL(GLE_DISABLE, PFNGLCLEARBUFFERFVPROC, glClearBufferfv);
  GLE_IMPL(GLE_DISABLE, PFNGLCLEARBUFFERIVPROC, glClearBufferiv);
  GLE_IMPL(GLE_DISABLE, PFNGLCLEARBUFFERUIVPROC, glClearBufferuiv);
  GLE_IMPL(GLE_DISABLE, PFNGLCOLORMASKIPROC, glColorMaski);
  GLE_IMPL(GLE_DISABLE, PFNGLDISABLEIPROC, glDisablei);
  GLE_IMPL(GLE_DISABLE, PFNGLENABLEIPROC, glEnablei);
  GLE_IMPL(GLE_DISABLE, PFNGLENDCONDITIONALRENDERPROC, glEndConditionalRender);
  GLE_IMPL(GLE_DISABLE, PFNGLENDTRANSFORMFEEDBACKPROC, glEndTransformFeedback);
  GLE_IMPL(GLE_DISABLE, PFNGLBINDBUFFERRANGEPROC, glBindBufferRange);
  GLE_IMPL(GLE_DISABLE, PFNGLBINDBUFFERBASEPROC, glBindBufferBase);
  GLE_IMPL(GLE_DISABLE, PFNGLGETBOOLEANI_VPROC, glGetBooleani_v);
  GLE_IMPL(GLE_DISABLE, PFNGLGETINTEGERI_VPROC, glGetIntegeri_v);
  GLE_IMPL(GLE_DISABLE, PFNGLGETFRAGDATALOCATIONPROC, glGetFragDataLocation);
  GLE_IMPL(GLE_ENABLE, PFNGLGETSTRINGIPROC, glGetStringi);
  GLE_IMPL(GLE_DISABLE, PFNGLGETTEXPARAMETERIIVPROC, glGetTexParameterIiv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETTEXPARAMETERIUIVPROC, glGetTexParameterIuiv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETTRANSFORMFEEDBACKVARYINGPROC, glGetTransformFeedbackVarying);
  GLE_IMPL(GLE_DISABLE, PFNGLGETUNIFORMUIVPROC, glGetUniformuiv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETVERTEXATTRIBIIVPROC, glGetVertexAttribIiv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETVERTEXATTRIBIUIVPROC, glGetVertexAttribIuiv);
  GLE_IMPL(GLE_DISABLE, PFNGLISENABLEDIPROC, glIsEnabledi);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXPARAMETERIIVPROC, glTexParameterIiv);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXPARAMETERIUIVPROC, glTexParameterIuiv);
  GLE_IMPL(GLE_DISABLE, PFNGLTRANSFORMFEEDBACKVARYINGSPROC, glTransformFeedbackVaryings);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM1UIPROC, glUniform1ui);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM1UIVPROC, glUniform1uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM2UIPROC, glUniform2ui);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM2UIVPROC, glUniform2uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM3UIPROC, glUniform3ui);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM3UIVPROC, glUniform3uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM4UIPROC, glUniform4ui);
  GLE_IMPL(GLE_DISABLE, PFNGLUNIFORM4UIVPROC, glUniform4uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI1IPROC, glVertexAttribI1i);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI1IVPROC, glVertexAttribI1iv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI1UIPROC, glVertexAttribI1ui);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI1UIVPROC, glVertexAttribI1uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI2IPROC, glVertexAttribI2i);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI2IVPROC, glVertexAttribI2iv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI2UIPROC, glVertexAttribI2ui);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI2UIVPROC, glVertexAttribI2uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI3IPROC, glVertexAttribI3i);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI3IVPROC, glVertexAttribI3iv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI3UIPROC, glVertexAttribI3ui);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI3UIVPROC, glVertexAttribI3uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4BVPROC, glVertexAttribI4bv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4IPROC, glVertexAttribI4i);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4IVPROC, glVertexAttribI4iv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4SVPROC, glVertexAttribI4sv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4UBVPROC, glVertexAttribI4ubv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4UIPROC, glVertexAttribI4ui);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4UIVPROC, glVertexAttribI4uiv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBI4USVPROC, glVertexAttribI4usv);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer);

  // GL_VERSION_3_1
  GLE_IMPL(GLE_DISABLE, PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced);
  GLE_IMPL(GLE_DISABLE, PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced);
  GLE_IMPL(GLE_DISABLE, PFNGLPRIMITIVERESTARTINDEXPROC, glPrimitiveRestartIndex);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXBUFFERPROC, glTexBuffer);

  // GL_VERSION_3_2
  GLE_IMPL(GLE_ENABLE, PFNGLFRAMEBUFFERTEXTUREPROC, glFramebufferTexture);
  GLE_IMPL(GLE_DISABLE, PFNGLGETBUFFERPARAMETERI64VPROC, glGetBufferParameteri64v);
  GLE_IMPL(GLE_DISABLE, PFNGLGETINTEGER64I_VPROC, glGetInteger64i_v);
  GLE_IMPL(GLE_DISABLE, PFNGLDRAWELEMENTSBASEVERTEXPROC, glDrawElementsBaseVertex);

  // GL_VERSION_3_3
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor);
  GLE_IMPL(GLE_DISABLE, PFNGLBINDSAMPLERPROC, glBindSampler);

  // GL_VERSION_4_0
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDEQUATIONSEPARATEIPROC, glBlendEquationSeparatei);
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDEQUATIONIPROC, glBlendEquationi);
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDFUNCSEPARATEIPROC, glBlendFuncSeparatei);
  GLE_IMPL(GLE_DISABLE, PFNGLBLENDFUNCIPROC, glBlendFunci);
  GLE_IMPL(GLE_DISABLE, PFNGLMINSAMPLESHADINGPROC, glMinSampleShading);

  // GL_VERSION_4_2
  GLE_IMPL(GLE_DISABLE, PFNGLMEMORYBARRIER, glMemoryBarrier);

  // GL_VERSION_4_3
  GLE_IMPL(GLE_DISABLE, PFNGLDISPATCHCOMPUTEPROC, glDispatchCompute);
  GLE_IMPL(GLE_DISABLE, PFNGLBINDIMAGETEXTUREPROC, glBindImageTexture);

  // GL_AMD_debug_output
  GLE_IMPL(GLE_DISABLE, PFNGLDEBUGMESSAGECALLBACKAMDPROC, glDebugMessageCallbackAMD);
  GLE_IMPL(GLE_DISABLE, PFNGLDEBUGMESSAGEENABLEAMDPROC, glDebugMessageEnableAMD);
  GLE_IMPL(GLE_DISABLE, PFNGLDEBUGMESSAGEINSERTAMDPROC, glDebugMessageInsertAMD);
  GLE_IMPL(GLE_DISABLE, PFNGLGETDEBUGMESSAGELOGAMDPROC, glGetDebugMessageLogAMD);

#if defined(GLE_CGL_ENABLED)
  // GL_APPLE_aux_depth_stencil
  // (no functions)

  // GL_APPLE_client_storage
  // (no functions)

  // GL_APPLE_element_array
  GLE_IMPL(GLE_DISABLE, PFNGLDRAWELEMENTARRAYAPPLEPROC, glDrawElementArrayAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLDRAWRANGEELEMENTARRAYAPPLEPROC, glDrawRangeElementArrayAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLELEMENTPOINTERAPPLEPROC, glElementPointerAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTIDRAWELEMENTARRAYAPPLEPROC, glMultiDrawElementArrayAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLMULTIDRAWRANGEELEMENTARRAYAPPLEPROC, glMultiDrawRangeElementArrayAPPLE);

  // GL_APPLE_fence
  GLE_IMPL(GLE_DISABLE, PFNGLDELETEFENCESAPPLEPROC, glDeleteFencesAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLFINISHFENCEAPPLEPROC, glFinishFenceAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLFINISHOBJECTAPPLEPROC, glFinishObjectAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLGENFENCESAPPLEPROC, glGenFencesAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLISFENCEAPPLEPROC, glIsFenceAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLSETFENCEAPPLEPROC, glSetFenceAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLTESTFENCEAPPLEPROC, glTestFenceAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLTESTOBJECTAPPLEPROC, glTestObjectAPPLE);

  // GL_APPLE_float_pixels
  // (no functions)

  // GL_APPLE_flush_buffer_range
  GLE_IMPL(GLE_DISABLE, PFNGLBUFFERPARAMETERIAPPLEPROC, glBufferParameteriAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLFLUSHMAPPEDBUFFERRANGEAPPLEPROC, glFlushMappedBufferRangeAPPLE);

  // GL_APPLE_object_purgeable
  GLE_IMPL(GLE_DISABLE, PFNGLGETOBJECTPARAMETERIVAPPLEPROC, glGetObjectParameterivAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLOBJECTPURGEABLEAPPLEPROC, glObjectPurgeableAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLOBJECTUNPURGEABLEAPPLEPROC, glObjectUnpurgeableAPPLE);

  // GL_APPLE_pixel_buffer
  // (no functions)

  // GL_APPLE_rgb_422
  // (no functions)

  // GL_APPLE_row_bytes
  // (no functions)

  // GL_APPLE_specular_vector
  // (no functions)

  // GL_APPLE_texture_range
  GLE_IMPL(GLE_DISABLE, PFNGLGETTEXPARAMETERPOINTERVAPPLEPROC, glGetTexParameterPointervAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXTURERANGEAPPLEPROC, glTextureRangeAPPLE);

  // GL_APPLE_transform_hint
  // (no functions)

  // GL_APPLE_vertex_array_object
  GLE_IMPL(GLE_DISABLE, PFNGLBINDVERTEXARRAYAPPLEPROC, glBindVertexArrayAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLDELETEVERTEXARRAYSAPPLEPROC, glDeleteVertexArraysAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLGENVERTEXARRAYSAPPLEPROC, glGenVertexArraysAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLISVERTEXARRAYAPPLEPROC, glIsVertexArrayAPPLE);

  // GL_APPLE_vertex_array_range
  GLE_IMPL(GLE_DISABLE, PFNGLFLUSHVERTEXARRAYRANGEAPPLEPROC, glFlushVertexArrayRangeAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXARRAYPARAMETERIAPPLEPROC, glVertexArrayParameteriAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLVERTEXARRAYRANGEAPPLEPROC, glVertexArrayRangeAPPLE);

  // GL_APPLE_vertex_program_evaluators
  GLE_IMPL(GLE_DISABLE, PFNGLDISABLEVERTEXATTRIBAPPLEPROC, glDisableVertexAttribAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLENABLEVERTEXATTRIBAPPLEPROC, glEnableVertexAttribAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLISVERTEXATTRIBENABLEDAPPLEPROC, glIsVertexAttribEnabledAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLMAPVERTEXATTRIB1DAPPLEPROC, glMapVertexAttrib1dAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLMAPVERTEXATTRIB1FAPPLEPROC, glMapVertexAttrib1fAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLMAPVERTEXATTRIB2DAPPLEPROC, glMapVertexAttrib2dAPPLE);
  GLE_IMPL(GLE_DISABLE, PFNGLMAPVERTEXATTRIB2FAPPLEPROC, glMapVertexAttrib2fAPPLE);
#endif // GLE_CGL_ENABLED

  // GL_ARB_copy_buffer
  GLE_IMPL(GLE_DISABLE, PFNGLCOPYBUFFERSUBDATAPROC, glCopyBufferSubData);

  // GL_ARB_debug_output
  GLE_IMPL(GLE_ENABLE, PFNGLDEBUGMESSAGECALLBACKARBPROC, glDebugMessageCallbackARB);
  GLE_IMPL(GLE_ENABLE, PFNGLDEBUGMESSAGECONTROLARBPROC, glDebugMessageControlARB);
  GLE_IMPL(GLE_DISABLE, PFNGLDEBUGMESSAGEINSERTARBPROC, glDebugMessageInsertARB);
  GLE_IMPL(GLE_DISABLE, PFNGLGETDEBUGMESSAGELOGARBPROC, glGetDebugMessageLogARB);

  // GL_ARB_ES2_compatibility
  GLE_IMPL(GLE_ENABLE, PFNGLCLEARDEPTHFPROC, glClearDepthf);
  GLE_IMPL(GLE_DISABLE, PFNGLDEPTHRANGEFPROC, glDepthRangef);
  GLE_IMPL(GLE_DISABLE, PFNGLGETSHADERPRECISIONFORMATPROC, glGetShaderPrecisionFormat);
  GLE_IMPL(GLE_DISABLE, PFNGLRELEASESHADERCOMPILERPROC, glReleaseShaderCompiler);
  GLE_IMPL(GLE_DISABLE, PFNGLSHADERBINARYPROC, glShaderBinary);

  // GL_ARB_framebuffer_object
  GLE_IMPL(GLE_ENABLE, PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
  GLE_IMPL(GLE_ENABLE, PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);
  GLE_IMPL(GLE_DISABLE, PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer);
  GLE_IMPL(GLE_ENABLE, PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
  GLE_IMPL(GLE_ENABLE, PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
  GLE_IMPL(GLE_ENABLE, PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);
  GLE_IMPL(GLE_ENABLE, PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);
  GLE_IMPL(GLE_DISABLE, PFNGLFRAMEBUFFERTEXTURE1DPROC, glFramebufferTexture1D);
  GLE_IMPL(GLE_DISABLE, PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
  GLE_IMPL(GLE_DISABLE, PFNGLFRAMEBUFFERTEXTURE3DPROC, glFramebufferTexture3D);
  GLE_IMPL(GLE_DISABLE, PFNGLFRAMEBUFFERTEXTURELAYERPROC, glFramebufferTextureLayer);
  GLE_IMPL(GLE_ENABLE, PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
  GLE_IMPL(GLE_ENABLE, PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);
  GLE_IMPL(GLE_DISABLE, PFNGLGENERATEMIPMAPPROC, glGenerateMipmap);
  GLE_IMPL(GLE_DISABLE, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC, glGetFramebufferAttachmentParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLGETRENDERBUFFERPARAMETERIVPROC, glGetRenderbufferParameteriv);
  GLE_IMPL(GLE_DISABLE, PFNGLISFRAMEBUFFERPROC, glIsFramebuffer);
  GLE_IMPL(GLE_DISABLE, PFNGLISRENDERBUFFERPROC, glIsRenderbuffer);
  GLE_IMPL(GLE_ENABLE, PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);
  GLE_IMPL(GLE_DISABLE, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample);

  // GL_ARB_framebuffer_sRGB
  // (no functions)

  // GL_ARB_texture_multisample
  GLE_IMPL(GLE_DISABLE, PFNGLGETMULTISAMPLEFVPROC, glGetMultisamplefv);
  GLE_IMPL(GLE_DISABLE, PFNGLSAMPLEMASKIPROC, glSampleMaski);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXIMAGE2DMULTISAMPLEPROC, glTexImage2DMultisample);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXIMAGE3DMULTISAMPLEPROC, glTexImage3DMultisample);

  // GL_ARB_texture_non_power_of_two
  // (no functions)

  // GL_ARB_texture_rectangle
  // (no functions)

  // GL_ARB_texture_storage
  GLE_IMPL(GLE_DISABLE, PFNGLTEXSTORAGE1DPROC, glTexStorage1D);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXSTORAGE2DPROC, glTexStorage2D);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXSTORAGE3DPROC, glTexStorage3D);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXTURESTORAGE1DEXTPROC, glTextureStorage1DEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXTURESTORAGE2DEXTPROC, glTextureStorage2DEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXTURESTORAGE3DEXTPROC, glTextureStorage3DEXT);

  // GL_ARB_texture_storage_multisample
  GLE_IMPL(GLE_DISABLE, PFNGLTEXSTORAGE2DMULTISAMPLEPROC, glTexStorage2DMultisample);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXSTORAGE3DMULTISAMPLEPROC, glTexStorage3DMultisample);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC, glTextureStorage2DMultisampleEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLTEXTURESTORAGE3DMULTISAMPLEEXTPROC, glTextureStorage3DMultisampleEXT);

  // GL_ARB_timer_query
  GLE_IMPL(GLE_DISABLE, PFNGLGETQUERYOBJECTI64VPROC, glGetQueryObjecti64v);
  GLE_IMPL(GLE_DISABLE, PFNGLGETQUERYOBJECTUI64VPROC, glGetQueryObjectui64v);
  GLE_IMPL(GLE_DISABLE, PFNGLQUERYCOUNTERPROC, glQueryCounter);

  // GL_ARB_vertex_array_object
  GLE_IMPL(GLE_ENABLE, PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
  GLE_IMPL(GLE_ENABLE, PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);
  GLE_IMPL(GLE_ENABLE, PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
  GLE_IMPL(GLE_DISABLE, PFNGLISVERTEXARRAYPROC, glIsVertexArray);

  // GL_EXT_draw_buffers2
  GLE_IMPL(GLE_DISABLE, PFNGLCOLORMASKINDEXEDEXTPROC, glColorMaskIndexedEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLDISABLEINDEXEDEXTPROC, glDisableIndexedEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLENABLEINDEXEDEXTPROC, glEnableIndexedEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLGETBOOLEANINDEXEDVEXTPROC, glGetBooleanIndexedvEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLGETINTEGERINDEXEDVEXTPROC, glGetIntegerIndexedvEXT);
  GLE_IMPL(GLE_DISABLE, PFNGLISENABLEDINDEXEDEXTPROC, glIsEnabledIndexedEXT);

  // GL_EXT_texture_filter_anisotropic
  // (no functions)

  // GL_KHR_debug
  GLE_IMPL(GLE_DISABLE, PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback);
  GLE_IMPL(GLE_DISABLE, PFNGLDEBUGMESSAGECONTROLPROC, glDebugMessageControl);
  GLE_IMPL(GLE_DISABLE, PFNGLDEBUGMESSAGEINSERTPROC, glDebugMessageInsert);
  GLE_IMPL(GLE_DISABLE, PFNGLGETDEBUGMESSAGELOGPROC, glGetDebugMessageLog);
  GLE_IMPL(GLE_DISABLE, PFNGLGETOBJECTLABELPROC, glGetObjectLabel);
  GLE_IMPL(GLE_DISABLE, PFNGLGETOBJECTPTRLABELPROC, glGetObjectPtrLabel);
  GLE_IMPL(GLE_DISABLE, PFNGLOBJECTLABELPROC, glObjectLabel);
  GLE_IMPL(GLE_DISABLE, PFNGLOBJECTPTRLABELPROC, glObjectPtrLabel);
  GLE_IMPL(GLE_ENABLE, PFNGLPOPDEBUGGROUPPROC, glPopDebugGroup);
  GLE_IMPL(GLE_ENABLE, PFNGLPUSHDEBUGGROUPPROC, glPushDebugGroup);

  // GL_KHR_robust_buffer_access_behavior

  // GL_WIN_swap_hint
  GLE_IMPL(GLE_DISABLE, PFNGLADDSWAPHINTRECTWINPROC, glAddSwapHintRectWIN);

#if defined(GLE_WGL_ENABLED)
  // WGL
  // We don't declare pointers for these because we statically link to the implementations, same as
  // with the OpenGL 1.1 functions.
  // BOOL  wglCopyContext_Hook(HGLRC, HGLRC, UINT);
  // HGLRC wglCreateContext_Hook(HDC);
  // HGLRC wglCreateLayerContext_Hook(HDC, int);
  // BOOL  wglDeleteContext_Hook(HGLRC);
  // HGLRC wglGetCurrentContext_Hook(VOID);
  // HDC   wglGetCurrentDC_Hook(VOID);
  // PROC  wglGetProcAddress_Hook(LPCSTR);
  // BOOL  wglMakeCurrent_Hook(HDC, HGLRC);
  // BOOL  wglShareLists_Hook(HGLRC, HGLRC);
  // BOOL  wglUseFontBitmapsA_Hook(HDC, DWORD, DWORD, DWORD);
  // BOOL  wglUseFontBitmapsW_Hook(HDC, DWORD, DWORD, DWORD);
  // BOOL  wglUseFontOutlinesA_Hook(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int,
  // LPGLYPHMETRICSFLOAT); BOOL  wglUseFontOutlinesW_Hook(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT,
  // int, LPGLYPHMETRICSFLOAT); BOOL  wglDescribeLayerPlane_Hook(HDC, int, int, UINT,
  // LPLAYERPLANEDESCRIPTOR); int   wglSetLayerPaletteEntries_Hook(HDC, int, int, int, CONST
  // COLORREF *); int   wglGetLayerPaletteEntries_Hook(HDC, int, int, int, COLORREF *); BOOL
  // wglRealizeLayerPalette_Hook(HDC, int, BOOL); BOOL  wglSwapLayerBuffers_Hook(HDC, UINT); DWORD
  // wglSwapMultipleBuffers_Hook(UINT, CONST WGLSWAP *);

#if 0
        GLE_IMPL(GLE_DISABLE, PFNWGLCOPYCONTEXTPROC, wglCopyContext);
        GLE_IMPL(GLE_DISABLE, PFNWGLCREATECONTEXTPROC, wglCreateContext);
        GLE_IMPL(GLE_DISABLE, PFNWGLCREATELAYERCONTEXTPROC, wglCreateLayerContext);
        GLE_IMPL(GLE_DISABLE, PFNWGLDELETECONTEXTPROC, wglDeleteContext);
        GLE_IMPL(GLE_DISABLE, PFNWGLGETCURRENTCONTEXTPROC, wglGetCurrentContext);
        GLE_IMPL(GLE_DISABLE, PFNWGLGETCURRENTDCPROC, wglGetCurrentDC);
        GLE_IMPL(GLE_DISABLE, PFNWGLGETPROCADDRESSPROC, wglGetProcAddress);
        GLE_IMPL(GLE_DISABLE, PFNWGLMAKECURRENTPROC, wglMakeCurrent);
        GLE_IMPL(GLE_DISABLE, PFNWGLSHARELISTSPROC, wglShareLists);
        GLE_IMPL(GLE_DISABLE, PFNWGLUSEFONTBITMAPSAPROC, wglUseFontBitmapsA);
        GLE_IMPL(GLE_DISABLE, PFNWGLUSEFONTBITMAPSWPROC, wglUseFontBitmapsW);
        GLE_IMPL(GLE_DISABLE, PFNWGLUSEFONTOUTLINESAPROC, wglUseFontOutlinesA);
        GLE_IMPL(GLE_DISABLE, PFNWGLUSEFONTOUTLINESWPROC, wglUseFontOutlinesW);
        GLE_IMPL(GLE_DISABLE, PFNWGLDESCRIBELAYERPLANEPROC, wglDescribeLayerPlane);
        GLE_IMPL(GLE_DISABLE, PFNWGLSETLAYERPALETTEENTRIESPROC, wglSetLayerPaletteEntries);
        GLE_IMPL(GLE_DISABLE, PFNWGLGETLAYERPALETTEENTRIESPROC, wglGetLayerPaletteEntries);
        GLE_IMPL(GLE_DISABLE, PFNWGLREALIZELAYERPALETTEPROC, wglRealizeLayerPalette);
        GLE_IMPL(GLE_DISABLE, PFNWGLSWAPLAYERBUFFERSPROC, wglSwapLayerBuffers);
        GLE_IMPL(GLE_DISABLE, PFNWGLSWAPMULTIPLEBUFFERSPROC, wglSwapMultipleBuffers);
#endif

  // WGL_ARB_buffer_region
  GLE_IMPL(GLE_DISABLE, PFNWGLCREATEBUFFERREGIONARBPROC, wglCreateBufferRegionARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLDELETEBUFFERREGIONARBPROC, wglDeleteBufferRegionARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLSAVEBUFFERREGIONARBPROC, wglSaveBufferRegionARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLRESTOREBUFFERREGIONARBPROC, wglRestoreBufferRegionARB);

  // WGL_ARB_extensions_string
  GLE_IMPL(GLE_DISABLE, PFNWGLGETEXTENSIONSSTRINGARBPROC, wglGetExtensionsStringARB);

  // WGL_ARB_pixel_format
  GLE_IMPL(GLE_DISABLE, PFNWGLGETPIXELFORMATATTRIBIVARBPROC, wglGetPixelFormatAttribivARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLGETPIXELFORMATATTRIBFVARBPROC, wglGetPixelFormatAttribfvARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB);

  // WGL_ARB_make_current_read
  GLE_IMPL(GLE_DISABLE, PFNWGLMAKECONTEXTCURRENTARBPROC, wglMakeContextCurrentARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLGETCURRENTREADDCARBPROC, wglGetCurrentReadDCARB);

  // WGL_ARB_pbuffer
  GLE_IMPL(GLE_DISABLE, PFNWGLCREATEPBUFFERARBPROC, wglCreatePbufferARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLGETPBUFFERDCARBPROC, wglGetPbufferDCARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLRELEASEPBUFFERDCARBPROC, wglReleasePbufferDCARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLDESTROYPBUFFERARBPROC, wglDestroyPbufferARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLQUERYPBUFFERARBPROC, wglQueryPbufferARB);

  // WGL_ARB_render_texture
  GLE_IMPL(GLE_DISABLE, PFNWGLBINDTEXIMAGEARBPROC, wglBindTexImageARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLRELEASETEXIMAGEARBPROC, wglReleaseTexImageARB);
  GLE_IMPL(GLE_DISABLE, PFNWGLSETPBUFFERATTRIBARBPROC, wglSetPbufferAttribARB);

  // WGL_ARB_pixel_format_float
  // (no functions)

  // WGL_ARB_framebuffer_sRGB
  // (no functions)

  // WGL_NV_present_video
  GLE_IMPL(GLE_DISABLE, PFNWGLENUMERATEVIDEODEVICESNVPROC, wglEnumerateVideoDevicesNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLBINDVIDEODEVICENVPROC, wglBindVideoDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLQUERYCURRENTCONTEXTNVPROC, wglQueryCurrentContextNV);

  // WGL_ARB_create_context
  GLE_IMPL(GLE_DISABLE, PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB);

  // WGL_ARB_create_context_profile
  // (no functions)

  // WGL_ARB_create_context_robustness
  // (no functions)

  // WGL_EXT_extensions_string
  GLE_IMPL(GLE_DISABLE, PFNWGLGETEXTENSIONSSTRINGEXTPROC, wglGetExtensionsStringEXT);

  // WGL_EXT_swap_control
  GLE_IMPL(GLE_DISABLE, PFNWGLGETSWAPINTERVALEXTPROC, wglGetSwapIntervalEXT);
  GLE_IMPL(GLE_DISABLE, PFNWGLSWAPINTERVALEXTPROC, wglSwapIntervalEXT);

  // WGL_OML_sync_control
  GLE_IMPL(GLE_DISABLE, PFNWGLGETSYNCVALUESOMLPROC, wglGetSyncValuesOML);
  GLE_IMPL(GLE_DISABLE, PFNWGLGETMSCRATEOMLPROC, wglGetMscRateOML);
  GLE_IMPL(GLE_DISABLE, PFNWGLSWAPBUFFERSMSCOMLPROC, wglSwapBuffersMscOML);
  GLE_IMPL(GLE_DISABLE, PFNWGLSWAPLAYERBUFFERSMSCOMLPROC, wglSwapLayerBuffersMscOML);
  GLE_IMPL(GLE_DISABLE, PFNWGLWAITFORMSCOMLPROC, wglWaitForMscOML);
  GLE_IMPL(GLE_DISABLE, PFNWGLWAITFORSBCOMLPROC, wglWaitForSbcOML);

  // WGL_NV_video_output
  GLE_IMPL(GLE_DISABLE, PFNWGLGETVIDEODEVICENVPROC, wglGetVideoDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLRELEASEVIDEODEVICENVPROC, wglReleaseVideoDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLBINDVIDEOIMAGENVPROC, wglBindVideoImageNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLRELEASEVIDEOIMAGENVPROC, wglReleaseVideoImageNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLSENDPBUFFERTOVIDEONVPROC, wglSendPbufferToVideoNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLGETVIDEOINFONVPROC, wglGetVideoInfoNV);

  // WGL_NV_swap_group
  GLE_IMPL(GLE_DISABLE, PFNWGLJOINSWAPGROUPNVPROC, wglJoinSwapGroupNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLBINDSWAPBARRIERNVPROC, wglBindSwapBarrierNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLQUERYSWAPGROUPNVPROC, wglQuerySwapGroupNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLQUERYMAXSWAPGROUPSNVPROC, wglQueryMaxSwapGroupsNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLQUERYFRAMECOUNTNVPROC, wglQueryFrameCountNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLRESETFRAMECOUNTNVPROC, wglResetFrameCountNV);

  // WGL_NV_video_capture
  GLE_IMPL(GLE_DISABLE, PFNWGLBINDVIDEOCAPTUREDEVICENVPROC, wglBindVideoCaptureDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLENUMERATEVIDEOCAPTUREDEVICESNVPROC, wglEnumerateVideoCaptureDevicesNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLLOCKVIDEOCAPTUREDEVICENVPROC, wglLockVideoCaptureDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLQUERYVIDEOCAPTUREDEVICENVPROC, wglQueryVideoCaptureDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLRELEASEVIDEOCAPTUREDEVICENVPROC, wglReleaseVideoCaptureDeviceNV);

  // WGL_NV_copy_image
  GLE_IMPL(GLE_DISABLE, PFNWGLCOPYIMAGESUBDATANVPROC, wglCopyImageSubDataNV);

  // WGL_NV_DX_interop
  GLE_IMPL(GLE_DISABLE, PFNWGLDXCLOSEDEVICENVPROC, wglDXCloseDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLDXLOCKOBJECTSNVPROC, wglDXLockObjectsNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLDXOBJECTACCESSNVPROC, wglDXObjectAccessNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLDXOPENDEVICENVPROC, wglDXOpenDeviceNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLDXREGISTEROBJECTNVPROC, wglDXRegisterObjectNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLDXSETRESOURCESHAREHANDLENVPROC, wglDXSetResourceShareHandleNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLDXUNLOCKOBJECTSNVPROC, wglDXUnlockObjectsNV);
  GLE_IMPL(GLE_DISABLE, PFNWGLDXUNREGISTEROBJECTNVPROC, wglDXUnregisterObjectNV);

#endif // GLE_WGL_ENABLED

#if defined(GLE_GLX_ENABLED)
  // GLX_VERSION_1_1
  // We don't create any pointers, because we assume these functions are always present.

  // GLX_VERSION_1_2
  GLE_IMPL(GLE_DISABLE, PFNGLXGETCURRENTDISPLAYPROC, glXGetCurrentDisplay);

  // GLX_VERSION_1_3
  GLE_IMPL(GLE_DISABLE, PFNGLXCHOOSEFBCONFIGPROC, glXChooseFBConfig);
  GLE_IMPL(GLE_DISABLE, PFNGLXCREATENEWCONTEXTPROC, glXCreateNewContext);
  GLE_IMPL(GLE_DISABLE, PFNGLXCREATEPBUFFERPROC, glXCreatePbuffer);
  GLE_IMPL(GLE_DISABLE, PFNGLXCREATEPIXMAPPROC, glXCreatePixmap);
  GLE_IMPL(GLE_DISABLE, PFNGLXCREATEWINDOWPROC, glXCreateWindow);
  GLE_IMPL(GLE_DISABLE, PFNGLXDESTROYPBUFFERPROC, glXDestroyPbuffer);
  GLE_IMPL(GLE_DISABLE, PFNGLXDESTROYPIXMAPPROC, glXDestroyPixmap);
  GLE_IMPL(GLE_DISABLE, PFNGLXDESTROYWINDOWPROC, glXDestroyWindow);
  GLE_IMPL(GLE_DISABLE, PFNGLXGETCURRENTREADDRAWABLEPROC, glXGetCurrentReadDrawable);
  GLE_IMPL(GLE_DISABLE, PFNGLXGETFBCONFIGATTRIBPROC, glXGetFBConfigAttrib);
  GLE_IMPL(GLE_DISABLE, PFNGLXGETFBCONFIGSPROC, glXGetFBConfigs);
  GLE_IMPL(GLE_DISABLE, PFNGLXGETSELECTEDEVENTPROC, glXGetSelectedEvent);
  GLE_IMPL(GLE_DISABLE, PFNGLXGETVISUALFROMFBCONFIGPROC, glXGetVisualFromFBConfig);
  GLE_IMPL(GLE_DISABLE, PFNGLXMAKECONTEXTCURRENTPROC, glXMakeContextCurrent);
  GLE_IMPL(GLE_DISABLE, PFNGLXQUERYCONTEXTPROC, glXQueryContext);
  GLE_IMPL(GLE_DISABLE, PFNGLXQUERYDRAWABLEPROC, glXQueryDrawable);
  GLE_IMPL(GLE_DISABLE, PFNGLXSELECTEVENTPROC, glXSelectEvent);

  // GLX_VERSION_1_4
  // Nothing to declare

  // GLX_ARB_create_context
  GLE_IMPL(GLE_DISABLE, PFNGLXCREATECONTEXTATTRIBSARBPROC, glXCreateContextAttribsARB);

  // GLX_EXT_swap_control
  GLE_IMPL(GLE_DISABLE, PFNGLXSWAPINTERVALEXTPROC, glXSwapIntervalEXT);

  // GLX_OML_sync_control
  GLE_IMPL(GLE_DISABLE, PFNGLXGETMSCRATEOMLPROC, glXGetMscRateOML);
  GLE_IMPL(GLE_DISABLE, PFNGLXGETSYNCVALUESOMLPROC, glXGetSyncValuesOML);
  GLE_IMPL(GLE_DISABLE, PFNGLXSWAPBUFFERSMSCOMLPROC, glXSwapBuffersMscOML);
  GLE_IMPL(GLE_DISABLE, PFNGLXWAITFORMSCOMLPROC, glXWaitForMscOML);
  GLE_IMPL(GLE_DISABLE, PFNGLXWAITFORSBCOMLPROC, glXWaitForSbcOML);

  // GLX_MESA_swap_control
  GLE_IMPL(GLE_DISABLE, PFNGLXGETSWAPINTERVALMESAPROC, glXGetSwapIntervalMESA);
  GLE_IMPL(GLE_DISABLE, PFNGLXSWAPINTERVALMESAPROC, glXSwapIntervalMESA);

#endif // GLE_GLX_ENABLED

  // Boolean extension support indicators. Each of these identifies the
  // presence or absence of the given extension. A better solution here
  // might be to use an STL map<const char*, bool>.
  bool gle_AMD_debug_output;
  // bool gle_AMD_performance_monitor;
  bool gle_APPLE_aux_depth_stencil;
  bool gle_APPLE_client_storage;
  bool gle_APPLE_element_array;
  bool gle_APPLE_fence;
  bool gle_APPLE_float_pixels;
  bool gle_APPLE_flush_buffer_range;
  bool gle_APPLE_object_purgeable;
  bool gle_APPLE_pixel_buffer;
  bool gle_APPLE_rgb_422;
  bool gle_APPLE_row_bytes;
  bool gle_APPLE_specular_vector;
  bool gle_APPLE_texture_range;
  bool gle_APPLE_transform_hint;
  bool gle_APPLE_vertex_array_object;
  bool gle_APPLE_vertex_array_range;
  bool gle_APPLE_vertex_program_evaluators;
  bool gle_APPLE_ycbcr_422;
  bool gle_ARB_copy_buffer;
  bool gle_ARB_debug_output;
  bool gle_ARB_depth_buffer_float;
  // bool gle_ARB_direct_state_access;
  bool gle_ARB_ES2_compatibility;
  bool gle_ARB_framebuffer_object;
  bool gle_ARB_framebuffer_sRGB;
  bool gle_ARB_texture_multisample;
  bool gle_ARB_texture_non_power_of_two;
  bool gle_ARB_texture_rectangle;
  bool gle_ARB_texture_storage;
  bool gle_ARB_texture_storage_multisample;
  bool gle_ARB_timer_query;
  bool gle_ARB_vertex_array_object;
  // bool gle_ARB_vertex_attrib_binding;
  bool gle_EXT_draw_buffers2;
  bool gle_EXT_texture_compression_s3tc;
  bool gle_EXT_texture_filter_anisotropic;
  bool gle_EXT_texture_sRGB;
  // bool gle_KHR_context_flush_control;
  bool gle_KHR_debug;
  // bool gle_KHR_robust_buffer_access_behavior;
  // bool gle_KHR_robustness;
  bool gle_WIN_swap_hint;

#if defined(GLE_WGL_ENABLED)
  bool gle_WGL_ARB_buffer_region;
  bool gle_WGL_ARB_create_context;
  bool gle_WGL_ARB_create_context_profile;
  bool gle_WGL_ARB_create_context_robustness;
  bool gle_WGL_ARB_extensions_string;
  bool gle_WGL_ARB_framebuffer_sRGB;
  bool gle_WGL_ARB_make_current_read;
  bool gle_WGL_ARB_pbuffer;
  bool gle_WGL_ARB_pixel_format;
  bool gle_WGL_ARB_pixel_format_float;
  bool gle_WGL_ARB_render_texture;
  bool gle_WGL_ATI_render_texture_rectangle;
  bool gle_WGL_EXT_extensions_string;
  bool gle_WGL_EXT_swap_control;
  bool gle_WGL_NV_copy_image;
  bool gle_WGL_NV_DX_interop;
  bool gle_WGL_NV_DX_interop2;
  bool gle_WGL_NV_present_video;
  bool gle_WGL_NV_render_texture_rectangle;
  bool gle_WGL_NV_swap_group;
  bool gle_WGL_NV_video_capture;
  bool gle_WGL_NV_video_output;
  bool gle_WGL_OML_sync_control;
#elif defined(GLE_GLX_ENABLED)
  bool gle_GLX_ARB_create_context;
  bool gle_GLX_ARB_create_context_profile;
  bool gle_GLX_ARB_create_context_robustness;
  bool gle_GLX_EXT_swap_control;
  bool gle_GLX_OML_sync_control;
  bool gle_MESA_swap_control;
#endif

}; // class GLEContext

} // namespace OVR

#endif // OVR_CAPI_GLE_h
