#ifndef DEBUGGL
#define DEBUGGL

#include <string>

// define DEBUG_GL to use the CHECK_GL_ERRORS macro
//#define DEBUG_GL

void check_gl_errors(const std::string &call);

#ifdef DEBUG_GL

#define CHECK_GL_ERRORS(call) check_gl_errors(call)

#else

#define CHECK_GL_ERRORS(call)

#endif

#endif

