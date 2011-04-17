/*
 * Copyright (C) 2011 Gabor Papp
 * http://mndl.hu/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include "GLSLProg.h"

GLSLProg::GLSLProg(const char *vertex_src, const char *fragment_src /* = NULL */) :
	vertex_handle(0),
	fragment_handle(0),
	program_handle(0)
{
	if (vertex_src != NULL)
		vertex_handle = load_shader(vertex_src, GL_VERTEX_SHADER);
	if (fragment_src != NULL)
		fragment_handle = load_shader(fragment_src, GL_FRAGMENT_SHADER);

	if (!vertex_handle && !fragment_handle)
		return;

	program_handle = glCreateProgram();
	if (vertex_handle)
		glAttachShader(program_handle, vertex_handle);
	if (fragment_handle)
		glAttachShader(program_handle, fragment_handle);
	glLinkProgram(program_handle);
	CHECK_GL_ERRORS("glLinkProgram");

	GLint status;
	glGetProgramiv(program_handle, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		char log[1024];
		glGetProgramInfoLog(program_handle, 1024, NULL, log);
		std::cerr << "GLSLProg compile error: " << log << std::endl;
	}

	glValidateProgram(program_handle);
	glGetProgramiv(program_handle, GL_VALIDATE_STATUS, &status);
	if (status != GL_TRUE)
	{
		char log[1024];
		glGetProgramInfoLog(program_handle, 1024, NULL, log);
		std::cerr << "GLSLProg compile error: " << log << std::endl;
	}
	CHECK_GL_ERRORS("GLSLProg constr");
	//std::cerr << "GLSLProg constr OK! " << program_handle << std::endl;
}

GLSLProg::~GLSLProg()
{
	if (program_handle)
		glDeleteProgram(program_handle);
	if (vertex_handle)
		glDeleteShader(vertex_handle);
	if (fragment_handle)
		glDeleteShader(fragment_handle);
}

unsigned GLSLProg::load_shader(const char *source, int type)
{
	unsigned shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	CHECK_GL_ERRORS("glCompileShader");

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
	{
		char log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, log);
		std::cerr << "GLSLProg compile error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void GLSLProg::bind()
{
	glUseProgram(program_handle);
	//std::cerr << "ph: " << program_handle << std::endl;
	//CHECK_GL_ERRORS("GLSLProg bind");
}

void GLSLProg::unbind()
{
	glUseProgram(0);
}

void GLSLProg::uniform(const std::string &name, int data)
{
	int loc = get_uniform_location(name);
	glUniform1i(loc, data);
}

void GLSLProg::uniform(const std::string &name, float data)
{
	int loc = get_uniform_location(name);
	glUniform1f(loc, data);
}

int GLSLProg::get_uniform_location(const std::string &name)
{
	std::map<std::string, int>::const_iterator i = uniform_locs.find(name);
    if (i == uniform_locs.end())
	{
        int loc = glGetUniformLocation(program_handle, name.c_str());
        uniform_locs[name] = loc;
        return loc;
    }
    else
	{
        return i->second;
	}
}

