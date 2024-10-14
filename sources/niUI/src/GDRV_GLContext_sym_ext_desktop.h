TSGL_EXT_PROC(void, glGenBuffers, (GLsizei n, GLuint* buffers));
TSGL_EXT_PROC(void, glBindBuffer, (GLenum target, GLuint buffer));
TSGL_EXT_PROC(void*, glMapBuffer, (GLenum target, GLenum access));
TSGL_EXT_PROC(GLboolean, glUnmapBuffer, (GLenum target));
TSGL_EXT_PROC(void, glBufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage));
TSGL_EXT_PROC(void, glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data));
TSGL_EXT_PROC(void, glDeleteBuffers, (GLsizei n, const GLuint* buffers));
TSGL_EXT_PROC(void, glActiveTexture, (GLenum texture));

TSGL_ARB_EXT_PROC(GLhandle, glCreateShaderObject, (GLenum));
TSGL_ARB_EXT_PROC(GLhandle, glCreateProgramObject, (void));
TSGL_ARB_EXT_PROC(void, glDeleteObject, (GLhandle obj));
TSGL_ARB_EXT_PROC(void, glAttachObject, (GLhandle program, GLhandle obj));
TSGL_ARB_EXT_PROC(void, glUseProgramObject, (GLhandle program));
TSGL_ARB_EXT_PROC(void, glGetObjectParameteriv, (GLhandle shader, GLenum pname, GLint* params));
TSGL_ARB_EXT_PROC(void, glGetInfoLog, (GLhandle object, GLsizei maxLength, GLsizei* length, GLchar* infoLog));

TSGL_EXT_PROC(GLuint, glCreateShader, (GLenum type));
TSGL_EXT_PROC(void, glShaderSource, (GLuint shader, GLsizei count, const char* const* src, const GLint* length));
TSGL_EXT_PROC(void, glCompileShader, (GLuint shader));
TSGL_EXT_PROC(void, glAttachShader, (GLuint program, GLuint shader));
TSGL_EXT_PROC(void, glLinkProgram, (GLuint program));
TSGL_EXT_PROC(int, glGetUniformLocation, (GLuint program, const char* name));
TSGL_EXT_PROC(void, glUniform1f, (GLint location, GLfloat v0));
TSGL_EXT_PROC(void, glUniform2f, (GLint location, GLfloat v0, GLfloat v1));
TSGL_EXT_PROC(void, glUniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2));
TSGL_EXT_PROC(void, glUniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
TSGL_EXT_PROC(void, glUniform1fv, (GLint location, GLsizei count, const GLfloat* value));
TSGL_EXT_PROC(void, glUniform2fv, (GLint location, GLsizei count, const GLfloat* value));
TSGL_EXT_PROC(void, glUniform3fv, (GLint location, GLsizei count, const GLfloat* value));
TSGL_EXT_PROC(void, glUniform4fv, (GLint location, GLsizei count, const GLfloat* value));
TSGL_EXT_PROC(void, glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value));
TSGL_EXT_PROC(void, glUniform1i, (GLint location, GLint v0));
TSGL_EXT_PROC(void, glBindAttribLocation, (GLuint program, GLuint index, const char* name));
TSGL_EXT_PROC(void, glGetActiveUniform, (GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, char* name));
TSGL_EXT_PROC(void, glEnableVertexAttribArray, (GLuint index));
TSGL_EXT_PROC(void, glDisableVertexAttribArray, (GLuint index));
TSGL_EXT_PROC(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer));
TSGL_EXT_PROC(void, glGetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog));
TSGL_EXT_PROC(void, glGetProgramiv, (GLuint program, GLenum pname, GLint* params));
TSGL_EXT_PROC(void, glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei* length, char* infoLog));

#ifdef USE_FBO
TSGL_EXT_PROC(GLboolean, glIsRenderbuffer, (GLuint renderbuffer));
TSGL_EXT_PROC(void, glBindRenderbuffer, (GLenum target, GLuint renderbuffer));
TSGL_EXT_PROC(void, glDeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers));
TSGL_EXT_PROC(void, glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers));
TSGL_EXT_PROC(void, glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height));
TSGL_EXT_PROC(void, glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint *params));

TSGL_EXT_PROC(GLboolean, glIsFramebuffer, (GLuint framebuffer));
TSGL_EXT_PROC(void, glBindFramebuffer, (GLenum target, GLuint framebuffer));
TSGL_EXT_PROC(void, glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers));
TSGL_EXT_PROC(void, glGenFramebuffers, (GLsizei n, GLuint *framebuffers));
TSGL_EXT_PROC(GLenum, glCheckFramebufferStatus, (GLenum target));

TSGL_EXT_PROC(void, glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));

TSGL_EXT_PROC(void, glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer));
TSGL_EXT_PROC(void, glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint *params));
TSGL_EXT_PROC(void, glGenerateMipmap, (GLenum target));
#endif

#if 0
TSGL_EXT_PROC(void, glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter));
TSGL_EXT_PROC(void, glFramebufferTexture1D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));
TSGL_EXT_PROC(void, glFramebufferTexture3D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset));
TSGL_EXT_PROC(void, glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height));
TSGL_EXT_PROC(void, glFramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer));
#endif

#ifdef USE_OQ
TSGL_OPT_EXT_PROC(void, glGetQueryiv, (GLenum target, GLenum pname, GLint* params));
TSGL_OPT_EXT_PROC(void, glGenQueries, (GLsizei n, GLuint* ids));
TSGL_OPT_EXT_PROC(void, glDeleteQueries, (GLsizei n, const GLuint* ids));
TSGL_OPT_EXT_PROC(void, glBeginQuery, (GLenum target, GLuint id));
TSGL_OPT_EXT_PROC(void, glEndQuery, (GLenum target));
TSGL_OPT_EXT_PROC(void, glGetQueryObjectiv, (GLuint id, GLenum pname, GLint* params));
TSGL_OPT_EXT_PROC(void, glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint* params));
#endif
