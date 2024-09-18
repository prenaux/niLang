NI_DLL_PROC(GLXContext, __cdecl, glXCreateContext, (Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct));
NI_DLL_PROC(Bool, __cdecl, glXMakeCurrent, (Display *dpy, GLXDrawable drawable, GLXContext ctx));
NI_DLL_PROC(void, __cdecl, glXDestroyContext, (Display *dpy, GLXContext ctx));
NI_DLL_PROC(XVisualInfo*, __cdecl, glXChooseVisual, (Display *dpy, int screen, int *attribList));
NI_DLL_PROC(void, __cdecl, glXSwapBuffers, (Display *dpy, GLXDrawable drawable));
