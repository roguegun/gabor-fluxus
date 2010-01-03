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

#include <iostream>
#include <math.h>

#include <GL/glew.h>

#include "Surface.h"

using namespace std;

static void check_fbo_errors(void)
{
	const char *fbo_status_msg[] =
	{
		"GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT",
		"GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT",
		"unknown GL_FRAMEBUFFER error",
		"GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT",
		"GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT",
		"GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT",
		"GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT",
		"GL_FRAMEBUFFER_UNSUPPORTED_EXT",
		"unknown GL_FRAMEBUFFER error"
	};
	const unsigned fbo_status_msg_count =
		sizeof(fbo_status_msg)/sizeof(fbo_status_msg[0]);

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status == GL_FRAMEBUFFER_COMPLETE_EXT)
		return;

	status -= GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
	if (status >= fbo_status_msg_count)
		status = fbo_status_msg_count-1;
	cerr << "fbo - %s" << fbo_status_msg[status];
}

Surface::Surface(int w, int h, unsigned target /* = GL_TEXTURE_2D */,
		int texture_count /* = 1*/, int depth_needed/* = 0*/)
{
	if (!glewIsSupported("GL_EXT_framebuffer_object"))
	{
		cerr << "framebuffer_object extension is required" << endl;
		throw Error();
	}
	if ((target == GL_TEXTURE_RECTANGLE_ARB) &&
		!glewIsSupported("GL_ARB_texture_rectangle"))
	{
		cerr << "texture_rectangle extension is required" << endl;
		throw Error();
	}

	GLint maxbuffers;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxbuffers);
	if (texture_count > maxbuffers)
	{
		cerr << "fbo: " << texture_count << " required attachments, maximum: " <<
			maxbuffers << " supported" << endl;
		throw Error();
	}

	width = w;
	height = h;

	this->target = target;

	if (target == GL_TEXTURE_RECTANGLE_ARB)
	{
		fbo_width = w;
		fbo_height = h;
	}
	else
	{
		fbo_width = 1 << (unsigned)ceil(log2(w));
		fbo_height = 1 << (unsigned)ceil(log2(h));
	}

	/* setup textures to render to */
	this->texture_count = texture_count;
	textures = new GLuint[texture_count];

	glGenTextures(texture_count, textures);

	for (int i = 0; i < texture_count; i++)
	{
		glBindTexture(target, textures[i]);
		glTexImage2D(target, 0, GL_RGBA, fbo_width, fbo_height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, NULL);

		glBindTexture(target, 0);
		//check_gl_errors("fbo texture");
	}

	/* create the depth buffer if requested */
	if (depth_needed)
	{
		glGenRenderbuffersEXT(1, &depth_buffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_buffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
				fbo_width, fbo_height);
		//check_gl_errors("fbo depth buffer");
	}
	else
	{
		depth_buffer = 0;
	}

	/* setup the framebuffer */
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	/* attach the textures to the fbo as GL_COLOR_ATTACHMENTx_EXT */
	for (int i = 0; i < texture_count; i++)
	{
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT + i,
				target, textures[i], 0);
	}

	/* attach the depth buffer to the fbo */
	if (depth_needed)
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_buffer);
	check_fbo_errors();
	/* unbind the fbo */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	//check_gl_errors("fbo");

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = width;
	viewport.height = height;

	if (target == GL_TEXTURE_RECTANGLE_ARB)
	{
		max_s = width;
		max_t = height;
	}
	else
	{
		max_s = float(width)/float(fbo_width);
		max_t = float(height)/float(fbo_height);
	}
}

void Surface::bind(int texture_idx /*=0*/)
{
	/* saving the viewport and colour buffer configuration */
	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
	/* set rendering to GL_COLOR_ATTACHMENTx_EXT */
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + texture_idx);
}

void Surface::unbind(void)
{
	/* restoring the viewport and colour buffer configuration */
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void Surface::bind_texture(int texture_idx /*=0*/)
{
	glBindTexture(target, textures[texture_idx]);
}

void Surface::unbind_texture(int texture_idx /*=0*/)
{
	glBindTexture(target, 0);
}

Surface::~Surface()
{
	glDeleteFramebuffersEXT(1, &fbo);
	glDeleteRenderbuffersEXT(1, &depth_buffer);
	for (int i=0; i<texture_count; i++)
	{
		glDeleteTextures(1, &(textures[i]));
	}
	delete [] textures;
}

