/*
 * Copyright (C) 2009 Gabor Papp
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

#ifndef SURFACE_H
#define SURFACE_H

#include "OpenGL.h"

class Viewport
{
	public:
		int x, y;
		int width, height;
};

class Surface
{
	public:
		Surface(int w, int h, unsigned target = GL_TEXTURE_2D,
				int texture_count = 1, int depth_needed = 0);
		~Surface();

		class Error { };

		void bind(int texture_idx = 0);
		void unbind(void);
		void bind_texture(int texture_idx = 0);
		void unbind_texture(int texture_idx = 0);
		unsigned get_texture(int texture_idx = 0) { return textures[texture_idx]; };

		int texture_count;
		GLuint *textures;
		float max_s, max_t;

		int width, height;		/* size of surface */
		int fbo_width, fbo_height;	/* size of the framebuffer that contains the
								   surface, width and height are power of 2 */
	private:
		GLuint target;

		GLuint fbo;

		GLuint depth_buffer;

		Viewport viewport;

		GLfloat clear_color[4];
};

#endif

