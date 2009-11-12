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

#include <GL/glew.h>

#include "CIObjC.h"
#include "CITemplate.h"

using namespace std;

CITemplate::CITemplate(FFGLViewportStruct *vps) : FFGLPlugin(vps)
{
	/* this is called when the plugin is instantiated */
	rect_surfaces = new Surface* [minimum_input_frames];
	textures = new unsigned [minimum_input_frames];
	for (unsigned i = 0; i < minimum_input_frames; i++)
	{
		rect_surfaces[i] = new Surface(vps->width, vps->height, GL_TEXTURE_RECTANGLE_ARB);
		textures[i] = rect_surfaces[i]->get_texture();
	}
}

CITemplate::CITemplate()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		  cerr << "Error: " << glewGetErrorString(err) << endl;
	}

	/* initialise the core image filter */
	ci_init();

	/* initialise plugin, set information */
	char *name = ci_get_name();
	char *id = ci_get_id();
	set_name(name);
	set_id(id);

	set_description("FFGL " FILTER_NAME " CoreImage filter");
	set_about("by Gabor Papp");

	/* set the parameters */
	int n = ci_get_parameter_count();
	for (int i = 0; i < n; i++)
	{
		float def, min, max;
		int type;
		char *pname;
		ci_get_parameter_defaults(i, &pname, &def, &min, &max, &type);
		add_parameter(pname, def, min, max, type);
	}

	int frame_count = ci_get_input_frame_count();
	set_minimum_input_frames(frame_count);
	set_maximum_input_frames(frame_count);
	set_type(frame_count ? FF_EFFECT : FF_SOURCE);
}

CITemplate::~CITemplate()
{
	for (unsigned i = 0; i < minimum_input_frames; i++)
	{
		delete rect_surfaces[i];
	}
	delete [] rect_surfaces;
	delete [] textures;
}

unsigned CITemplate::process_opengl(ProcessOpenGLStruct *pgl)
{
	if (pgl->numInputTextures < minimum_input_frames)
		return FF_FAIL;

	for (unsigned i = 0; i < minimum_input_frames; i++)
	{
		FFGLTextureStruct *texture = pgl->inputTextures[i];

		/* render the texture to a rectangular fbo */
		rect_surfaces[i]->bind();

		glBindTexture(GL_TEXTURE_2D, texture->Handle);

		glEnable(GL_TEXTURE_2D);

		float s = texture->Width / (float)texture->HardwareWidth;
		float t = texture->Height / (float)texture->HardwareHeight;

		glBegin(GL_QUADS);
		glTexCoord2d(0, 0);
		glVertex2f(-1, -1);
		glTexCoord2d(0, t);
		glVertex2f(-1, 1);
		glTexCoord2d(s, t);
		glVertex2f(1, 1);
		glTexCoord2d(s, 0);
		glVertex2f(1, -1);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);
		rect_surfaces[i]->unbind();
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);

	/* bind the host fbo */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pgl->HostFBO);

	for (unsigned i = 0; i < parameter_count; i++)
	{
		ci_set_parameter(i, parameters[i].fvalue);
	}
	ci_process(textures, viewport.width, viewport.height);

	return FF_SUCCESS;
}

plugMainUnion plugMain(unsigned function_code, unsigned param, unsigned instance_id)
{
	/* creates the main plugin instance - this must not be global, because
	 * static initialization order fiasco can occur */
	static CITemplate *plugin = new CITemplate();
	plugin = plugin; // gets rid of unused variable warning
	return plug_main<CITemplate>(function_code, param, instance_id);
}

