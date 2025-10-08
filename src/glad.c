/*
 * GLAD - OpenGL loader generator
 * This is a minimal GLAD implementation for OpenGL 3.3 Core
 * For a complete implementation, visit https://glad.dav1d.de/
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Function pointer definitions
PFNGLCLEARPROC glClear;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLVIEWPORTPROC glViewport;
PFNGLENABLEPROC glEnable;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETSTRINGPROC glGetString;
PFNGLGETERRORPROC glGetError;
PFNGLDELETESHADERPROC glDeleteShader;

int gladLoadGLLoader(GLADloadproc load)
{
    if (!load) return 0;
    
    // Load OpenGL functions
    glClear = (PFNGLCLEARPROC)load("glClear");
    glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
    glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
    glEnable = (PFNGLENABLEPROC)load("glEnable");
    glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
    glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)load("glGenVertexArrays");
    glGenBuffers = (PFNGLGENBUFFERSPROC)load("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
    glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");
    glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
    glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
    glGetError = (PFNGLGETERRORPROC)load("glGetError");
    glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
    
    return 1; // Return 1 for success
}
