// Copyright (C) 2009 Gabor Papp
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef VIDEOTEXTURE_H
#define VIDEOTEXTURE_H

#include <string>

#include "OpenGL.h"

class VideoTexture
{
	public:
			class Format;

			VideoTexture(int w, int h, Format format);
			virtual ~VideoTexture();

			float* get_tcoords();

			unsigned get_texture_id()
			{
				return texture_id;
			}

			int get_width()
			{
				return width;
			}

			int get_height()
			{
				return height;
			}

			void upload(void *pixels);

			class Format
			{
				public:
					Format();

					void set_target(GLenum t) { target = t; }
					void set_internal_format(GLint intf) { internal_format = intf; }
					void set_datatype(GLenum t) { datatype = t; }
					void set_dataformat(GLenum format) { dataformat = format; }

				protected:
					GLenum target;
					GLenum datatype;
					GLenum wrap_s, wrap_t;
					GLenum min_filter, mag_filter;
					bool mipmapping;
					GLint internal_format;
					GLenum dataformat;

					friend class VideoTexture;
			};

	protected:
			void gen_texture(Format format);

			bool mipmapping_enabled;
			bool npot_enabled; // support for non power of two textures
			int width, height; // pixel buffer resolution of image
			int tex_width, tex_height; // texture resolution (power of 2)

			GLenum target;
			GLenum dataformat; // format of the pixel data, usually GL_RGB or GL_LUMINANCE
			GLenum datatype; // data type of the pixel data

			unsigned texture_id;
};

#endif

