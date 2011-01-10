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
#include "OpenGL.h"
#include "Dither.h"

using namespace std;

const char *Dither::vertex_shader ="\
void main(void) \
{ \
    gl_Position = ftransform(); \
    gl_TexCoord[0] = gl_MultiTexCoord0; \
} \
";

const char *Dither::fragment_shader = "\
uniform sampler2D color; \
uniform float width; \
uniform float height; \
\
void main(void) \
{ \
    const vec4 grey = vec4(.3, .59, .11, .0); \
    float c = dot(texture2D(color, gl_TexCoord[0].xy), grey); \
    vec2 xy = mod(gl_TexCoord[0].xy * vec2(width, height), 4.); \
    \
    float D = 31.0; \
    mat4 dither0 = mat4(1, 9, 3, 11, \
                    13, 5, 15, 7, \
                    4, 12,  2, 10, \
                    16, 8, 14, 6); \
    mat4 dither = dither0 / D; \
/*   \
    mat4 dither = mat4(...) / D \
    on osx results in: \
    (0) : fatal error C9999: Non scalar or vector type in ConvertNamedConstantsExpr() \
    Cg compiler terminated due to fatal error \
*/ \
    if (c <= dither[int(xy.x)][int(xy.y)]) \
        gl_FragColor = vec4(0, 0, 0, 1); \
    else \
        gl_FragColor = vec4(1); \
} \
";

GLSLProg *Dither::shader;

Dither::Dither(FFGLViewportStruct *vps) : FFGLPlugin(vps)
{
	/* this is called when the plugin is instantiated */
}

Dither::Dither()
{
	/* initialise plugin, set information */
	set_name("dither");
	set_id("DTHR");

	set_type(FF_EFFECT);
	set_description("Dither plugin");
	set_about("by Gabor Papp");
	set_version(1.0);
	set_minimum_input_frames(1);
	set_maximum_input_frames(1);

	shader = new GLSLProg(vertex_shader, fragment_shader);
}

Dither::~Dither()
{
	/* called when the plugin is deinstantiated */
}

unsigned Dither::process_opengl(ProcessOpenGLStruct *pgl)
{
	if ((pgl->numInputTextures < 1) || (pgl->inputTextures[0] == NULL))
		return FF_FAIL;

	FFGLTextureStruct *texture = pgl->inputTextures[0];

	glBindTexture(GL_TEXTURE_2D, texture->Handle);

	glEnable(GL_TEXTURE_2D);

	float s = texture->Width / (float)texture->HardwareWidth;
	float t = texture->Height / (float)texture->HardwareHeight;

	shader->bind();
	CHECK_GL_ERRORS("bind");

	shader->uniform("width", (float)texture->HardwareWidth);
	CHECK_GL_ERRORS("uniform width");
	shader->uniform("height", (float)texture->HardwareHeight);
	CHECK_GL_ERRORS("uniform height");

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

	shader->unbind();
	glBindTexture(GL_TEXTURE_2D, 0);

	return FF_SUCCESS;
}

plugMainUnion plugMain(unsigned function_code, unsigned param, unsigned instance_id)
{
	/* creates the main plugin instance - this must not be global, because
	 * static initialization order fiasco can occur */
	static Dither *plugin = new Dither();
	plugin = plugin; // gets rid of unused variable warning
	return plug_main<Dither>(function_code, param, instance_id);
}

