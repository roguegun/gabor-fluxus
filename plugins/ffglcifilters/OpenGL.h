#ifndef OPENGL_H
#define OPENGL_H

extern "C" {
#include "GL/glew.h"

#ifndef __APPLE__
#include "GL/glut.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#else
#include "GLUT/glut.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#endif
}

#endif

