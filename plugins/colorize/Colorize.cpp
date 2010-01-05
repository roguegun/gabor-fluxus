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
#include "OpenGL.h"
#include "Colorize.h"

using namespace std;

Colorize::Colorize(FFGLViewportStruct *vps) : FFGLPlugin(vps)
{
	/* this is called when the plugin is instantiated */
}

Colorize::Colorize()
{
	/* initialise plugin, set information */
	set_name("colorize");
	set_id("CLRZ");

	/* these are not required */
	set_type(FF_EFFECT); // set to FF_EFFECT by default
	set_description("Colorize example plugin"); // "" by default
	set_about("by Gabor Papp"); // "" by default
	set_version(1.0); // 1.0 by default
	set_minimum_input_frames(1); // 1 by default
	set_maximum_input_frames(1); // 1 by default

	/* add parameters */
	add_parameter("red", 1, FF_TYPE_RED);
	add_parameter("green", 1, FF_TYPE_GREEN);
	add_parameter("blue", 1, FF_TYPE_BLUE);
}

Colorize::~Colorize()
{
	/* called when the plugin is deinstantiated */
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

unsigned Colorize::process_opengl(ProcessOpenGLStruct *pgl)
{
	if ((pgl->numInputTextures < 1) || (pgl->inputTextures[0] == NULL))
		return FF_FAIL;

	FFGLTextureStruct *texture = pgl->inputTextures[0];

	float r = parameters[0].fvalue;
	float g = parameters[1].fvalue;
	float b = parameters[2].fvalue;

	glBindTexture(GL_TEXTURE_2D, texture->Handle);

	glEnable(GL_TEXTURE_2D);

	float s = texture->Width / (float)texture->HardwareWidth;
	float t = texture->Height / (float)texture->HardwareHeight;

	glColor4f(r, g, b, 1);

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

	glColor4f(1, 1, 1, 1);

	return FF_SUCCESS;
}

plugMainUnion plugMain(unsigned function_code, unsigned param, unsigned instance_id)
{
	/* creates the main plugin instance - this must not be global, because
	 * static initialization order fiasco can occur */
	static Colorize *plugin = new Colorize();
	plugin = plugin; // gets rid of unused variable warning
	return plug_main<Colorize>(function_code, param, instance_id);
}

