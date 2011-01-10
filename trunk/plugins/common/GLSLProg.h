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

#ifndef GLSLPROG_H
#define GLSLPROG_H

#include <string>
#include <map>

#include "OpenGL.h"
#define DEBUG_GL
#include "DebugGL.h"

class GLSLProg
{
	public:
		GLSLProg(const char *vertex_src = NULL, const char *fragment_src = NULL);
		~GLSLProg();

		void bind();
		void unbind();

		void uniform(const std::string &name, int data);
		void uniform(const std::string &name, float data);

	private:
		unsigned load_shader(const char *source, int type);

		unsigned vertex_handle;
		unsigned fragment_handle;
		unsigned program_handle;

		int get_uniform_location(const std::string &name);
		std::map<std::string, int> uniform_locs;
};

#endif

