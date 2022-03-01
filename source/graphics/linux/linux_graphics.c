
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <X11/Xlib-xcb.h>
#include <GL/glx.h>
#include "khrplatform.h"
#include "glxext.h"
#include "glcorearb.h"

typedef void ( *PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void ( *PFNGLDELETETEXTURESPROC) (GLsizei n, const GLuint *textures);
typedef void ( *PFNGLGENTEXTURESPROC) (GLsizei n, GLuint *textures);
typedef void ( *PFNGLDRAWELEMENTSPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices);

typedef void ( *PFNGLXDESTROYCONTEXTPROC) (Display *dpy, GLXContext ctx);
typedef const char *( *PFNGLXQUERYEXTENSIONSSTRINGPROC) (Display *dpy, int screen);
typedef void ( *PFNGLXSWAPBUFFERSPROC) (Display *dpy, GLXDrawable drawable);

#include "linux_graphics__window.h"
#include "linux_graphics__renderer.h"
#include "linux_graphics__app.h"

#include "linux_graphics__window.c"
#include "linux_graphics__renderer.c"
#include "linux_graphics__app.c"
