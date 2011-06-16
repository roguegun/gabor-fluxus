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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sstream>

#include <iostream>

#include "OpenGL.h"

#include "VideoTexture.h"

using namespace std;

//#define DEBUG_GL

#ifdef DEBUG_GL
static int check_gl_errors(const char *call)
{
	GLenum status = glGetError();
	if (status == GL_NO_ERROR)
		return 0;

	const char *status_msg = (const char *)gluErrorString(status);
	if (status_msg == NULL)
		status_msg = "unknown gl error";

	cerr << call << " - " << status_msg << " (" << status << ")" << endl;
	return 1;
}

#define CHECK_GL_ERRORS(call) check_gl_errors(call)

#else

#define CHECK_GL_ERRORS(call)

#endif

VideoTexture::VideoTexture(int w, int h, Format format) :
	texture_id(0)
{
	if (glewInit() != GLEW_OK)
	{
		cerr << "ERROR Unable to check OpenGL extensions" << endl;
	}

	mipmapping_enabled = (glGenerateMipmapEXT != NULL);
	npot_enabled = glewIsSupported("GL_ARB_texture_non_power_of_two");

	width = w;
	height = h;
	gen_texture(format);
}

VideoTexture::~VideoTexture()
{
	if (texture_id != 0)
	{
		glDeleteTextures(1, &texture_id);
	}
}

void VideoTexture::gen_texture(Format format)
{
	if (npot_enabled)
	{
		tex_width = width;
		tex_height = height;
	}
	else
	{
		tex_width = 1 << (unsigned)ceil(log2(width));
		tex_height = 1 << (unsigned)ceil(log2(height));
	}

	target = format.target;
	dataformat = format.dataformat;
	datatype = format.datatype;

	glEnable(format.target);
	glGenTextures(1, &texture_id);

	glBindTexture(format.target, texture_id);

	glTexImage2D(format.target, 0, format.internal_format, tex_width, tex_height, 0,
			format.dataformat, format.datatype, NULL);
	CHECK_GL_ERRORS("glTexImage2d");

	if (format.mipmapping && mipmapping_enabled)
	{
		glGenerateMipmapEXT(GL_TEXTURE_2D);
		CHECK_GL_ERRORS("glGenerateMipmapEXT");
	}
	else
	{
		mipmapping_enabled = false;
	}

	glTexParameteri(format.target, GL_TEXTURE_WRAP_S, format.wrap_s);
	glTexParameteri(format.target, GL_TEXTURE_WRAP_T, format.wrap_t);
	glTexParameteri(format.target, GL_TEXTURE_MIN_FILTER, format.min_filter);
	glTexParameteri(format.target, GL_TEXTURE_MAG_FILTER, format.mag_filter);
	CHECK_GL_ERRORS("glTexParameteri");

	glBindTexture(format.target, 0);
	glDisable(format.target);
}

void VideoTexture::upload(void *pixels)
{
	glEnable(target);
	glBindTexture(target, texture_id);

	glTexSubImage2D(target, 0, 0, 0, width, height,
			dataformat, datatype, pixels);
	CHECK_GL_ERRORS("upload: glTexSubImage2d");

	if (mipmapping_enabled)
	{
		glGenerateMipmapEXT(target);
		CHECK_GL_ERRORS("upload: glGenerateMipmapEXT");
	}

	glBindTexture(target, 0);
	glDisable(target);
}

/**
 * Returns video texture coordinates
 * \return 2x3 floats (top-left, bottom-right)
 **/
float *VideoTexture::get_tcoords()
{
	static float t[6];

	t[0] = 0.0;
	t[1] = 0.0;
	t[2] = 0.0;
	t[3] = npot_enabled ? 1.0 : (float)(width - 1)/(float)tex_width;
	t[4] = npot_enabled ? 1.0 : (float)(height - 1)/(float)tex_height;
	t[5] = 0.0;

	return t;
}

VideoTexture::Format::Format()
{
	target = GL_TEXTURE_2D;
	wrap_s = GL_CLAMP_TO_EDGE;
	wrap_t = GL_CLAMP_TO_EDGE;
	min_filter = GL_NEAREST;
	mag_filter = GL_NEAREST;
	mipmapping = true;
	internal_format = GL_RGBA;
	dataformat = GL_RGB;
	datatype = GL_UNSIGNED_BYTE;
}

