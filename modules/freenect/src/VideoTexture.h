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

class VideoTexture
{
	public:
			VideoTexture(int w, int h, int f);
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

	protected:
			void gen_texture();

			void upload(unsigned char *pixels);

			bool mipmapping_enabled;
			int width, height; // pixel buffer resolution of image
			int tex_width, tex_height; // texture resolution (power of 2)
			int format; // texture format, GL_RGB or GL_LUMINANCE

			unsigned texture_id;
};

#endif

