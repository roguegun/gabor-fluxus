/*
 * Copyright (C) 2010 Gabor Papp
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

#include "OpenGL.h"
#include "Pixelize.h"

using namespace std;

Pixelize::Pixelize(FFGLViewportStruct *vps) : FFGLPlugin(vps)
{
	/* this is called when the plugin is instantiated */
	pixelize_surface = new Surface(viewport.width, viewport.height);

	pixelize_surface->bind_texture();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Pixelize::Pixelize()
{
	/* initialise plugin, set information */

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		throw FFGLError();
	}

	set_name("pixelize");
	set_id("PXLZ");

	set_description("Pixelize plugin");
	set_about("by Gabor Papp");

	add_parameter("resolution", 0.5, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("linear", 0.0, FF_TYPE_BOOLEAN);
}

Pixelize::~Pixelize()
{
	/* called when the plugin is deinstantiated */
	delete pixelize_surface;
}

/*
static void check_gl_errors(const char *call)
{
    GLenum status = glGetError();
    if (status == GL_NO_ERROR)
        return;

    const char *status_msg = (const char *)gluErrorString(status);
    if (status_msg == NULL)
        status_msg = "unknown gl error";

    cerr << call << " - " << status_msg << " (" << status << ")" << endl;
}
*/

void Pixelize::view_ortho()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport.width, 0, viewport.height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void Pixelize::view_perspective()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

unsigned Pixelize::process_opengl(ProcessOpenGLStruct *pgl)
{
	if ((pgl->numInputTextures < 1) || (pgl->inputTextures[0] == NULL))
		return FF_FAIL;

	view_ortho();

	float v = 1 - parameters[PARAM_RESOLUTION].fvalue;
	float m = powf(2, -v*10); /* max multiplier is 1/2^10, this gives one pixel
								 for a 1024 pixel wide screen, pow provides smooth zooming */
	FFGLTextureStruct *texture = pgl->inputTextures[0];

	/* maximum texture coordinates on surface */
	float s_s = (float)texture->Width/(float)texture->HardwareWidth;
	float s_t = (float)texture->Height/(float)texture->HardwareHeight;

	glEnable(GL_TEXTURE_2D);
	/* render small rectange version of the surface on pixelize_surface */
	/* input is used as a texture */
	glBindTexture(GL_TEXTURE_2D, texture->Handle);
	pixelize_surface->bind();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	glTexCoord2f(s_s, 0);
	glVertex2f(m * pixelize_surface->width, 0);
	glTexCoord2f(s_s, s_t);
	glVertex2f(m * pixelize_surface->width, m * pixelize_surface->height);
	glTexCoord2f(0, s_t);
	glVertex2f(0, m * pixelize_surface->height);
	glEnd();

	pixelize_surface->unbind();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pgl->HostFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	pixelize_surface->bind_texture();

	GLint filter = (parameters[PARAM_LINEAR].fvalue > 0.0) ? GL_LINEAR : GL_NEAREST;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

	view_perspective();

	/* stretch the small rectangle to cover the whole screen */
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2i(-1, -1);
	glTexCoord2f(m * pixelize_surface->max_s, 0);
	glVertex2i(1, -1);
	glTexCoord2f(m * pixelize_surface->max_s, m * pixelize_surface->max_t);
	glVertex2i(1, 1);
	glTexCoord2f(0, m * pixelize_surface->max_t);
	glVertex2i(-1, 1);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glViewport(viewport.x, viewport.y, viewport.width, viewport.height);

	return FF_SUCCESS;
}

plugMainUnion plugMain(unsigned function_code, unsigned param, unsigned instance_id)
{
	/* creates the main plugin instance - this must not be global, because
	 * static initialization order fiasco can occur */
	static Pixelize *plugin = new Pixelize();
	plugin = plugin; // gets rid of unused variable warning
	return plug_main<Pixelize>(function_code, param, instance_id);
}

