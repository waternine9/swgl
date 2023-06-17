#ifndef SOFTWARE_GL_H
#define SOFTWARE_GL_H

/*
* DEPENDENCIES
*/

#include <cstdint>

/*
* CONSTANTS
*/

const uint32_t GL_COLOR_BUFFER_BIT = 0b01;
const uint32_t GL_DEPTH_BUFFER_BIT = 0b10;

#define GL_TRUE true
#define GL_FALSE false

/*
* TYPES
*/

typedef uint32_t GLuint;
typedef uint32_t GLsizei;
typedef int32_t GLint;
typedef char GLchar;
typedef bool GLboolean;
typedef float GLfloat;

/*
* ENUMS
*/

enum GLenum
{
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_COMPILE_STATUS,
	GL_LINK_STATUS,
	GL_ARRAY_BUFFER,

	GL_STATIC_DRAW,
	GL_STREAM_DRAW,
	GL_DYNAMIC_DRAW,

	GL_FLOAT,
	GL_INT,

	GL_RGB,
	GL_RGBA,

	GL_TRIANGLES,
	GL_POINTS,
	GL_LINES
};

/*
* NON-OPENGL HELPER FUNCTION DECLS
*/

void glInit(GLsizei width, GLsizei height);
uint32_t* glGetFramePtr();

/*
* SHADER FUNCTION DECLS
*/

GLuint glCreateShader(GLenum type);
void glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
void glCompileShader(GLuint shader);
void glDeleteShader(GLuint shader);

GLuint glCreateProgram();
void glAttachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);
void glUseProgram(GLuint program);

/*
* VERTEX ARRAY DECLS
*/

// Only supports 1 vertex array per call for now
GLuint glGenVertexArrays(GLsizei n, GLuint* arrays);
void glBindVertexArray(GLuint array);
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
void glEnableVertexAttribArray(GLuint index); // Doesn't do anything, here for backwards compatibility

/*
* BUFFER FUNCTION DECLS
*/

// Only supports 1 buffer per call for now
GLuint glGenBuffers(GLsizei n, GLuint* buffers);
void glBindBuffer(GLenum type, GLuint buffer);
void glBufferData(GLenum target, GLsizei size, const void* data, GLenum usage);

/*
* DRAW FUNCTION DECLS
*/

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClear(GLuint flags);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

void glDrawArrays(GLenum mode, GLint first, GLsizei count);

/*
* UNIFORM FUNCTIONS
*/

GLint glGetUniformLocation(GLuint program, const GLchar* name);

void glUniform1f(GLint location, GLfloat v0);
void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

void glUniform1i(GLint location, GLint v0);

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

#endif // SOFTWARE_GL_H
