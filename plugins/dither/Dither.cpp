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
#include <math.h>

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
uniform sampler2D txt; \
uniform float width; \
uniform float height; \
uniform float pwidth; \
uniform float pheight; \
uniform float thr; \
\
void main(void) \
{ \
    const vec4 grey = vec4(.3, .59, .11, .0); \
    vec2 step = vec2(pwidth, pheight) / vec2(width, height); \
    vec2 uv = step * floor(gl_TexCoord[0].xy / step); \
    float c = dot(texture2D(txt, uv), grey); \
    vec2 xy = mod(uv * vec2(width, height), 4.); \
    \
    mat4 dither = mat4(1, 9, 3, 11, \
                    13, 5, 15, 7, \
                    4, 12,  2, 10, \
                    16, 8, 14, 6) / thr; \
    if (c <= dither[int(xy.x)][int(xy.y)]) \
        gl_FragColor = vec4(0, 0, 0, 1); \
    else \
        gl_FragColor = vec4(1); \
} \
";

GLSLProg *Dither::shader = NULL;

Dither::Dither(FFGLViewportStruct *vps) : FFGLPlugin(vps)
{
	/* this is called when the plugin is instantiated */
	if (shader == NULL)
		shader = new GLSLProg(vertex_shader, fragment_shader);
}

Dither::Dither()
{
	/* initialise plugin, set information */
	set_name("Dither");
	set_id("DTHR");

	set_type(FF_EFFECT);
	set_description("Dither plugin");
	set_about("by Gabor Papp");
	set_version(1.0);
	set_minimum_input_frames(1);
	set_maximum_input_frames(1);

	add_parameter("thr", 31.0, 0.0, 256.0, FF_TYPE_STANDARD);
	add_parameter("pixel-x", 0, 0, 1, FF_TYPE_XPOS);
	add_parameter("pixel-y", 0, 0, 1, FF_TYPE_YPOS);

	if (!glewIsSupported("GL_VERSION_2_0 ") || (glCreateProgram == NULL) ||
			(glCreateShader == NULL) || (glAttachShader == NULL))
	{
		std::cerr << "OpenGL 2.0 required with GLSL support." << std::endl;
		throw FFGLError();
	}

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glEnable(GL_TEXTURE_2D);

	float s = texture->Width / (float)texture->HardwareWidth;
	float t = texture->Height / (float)texture->HardwareHeight;

	shader->bind();
	CHECK_GL_ERRORS("bind");

	shader->uniform("txt", 0);
	shader->uniform("width", (float)texture->HardwareWidth);
	shader->uniform("height", (float)texture->HardwareHeight);
	float thr = parameters[PARAM_THR].fvalue;
	if (thr < 0)
		thr = 0;
	else if (thr > 256)
		thr = 256;
	shader->uniform("thr", thr);

	float width_log2 = log2(texture->Width);
	float height_log2 = log2(texture->Height);

	float px = parameters[PARAM_PIXEL_WIDTH].fvalue;
	px = powf(2, px * width_log2);
	float py = parameters[PARAM_PIXEL_HEIGHT].fvalue;
	py = powf(2, py * height_log2);
	shader->uniform("pwidth", px);
	shader->uniform("pheight", py);

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
	try
	{
		/* creates the main plugin instance - this must not be global, because
		 * static initialization order fiasco can occur */
		static Dither *plugin = new Dither();
		plugin = plugin; // gets rid of unused variable warning
		return plug_main<Dither>(function_code, param, instance_id);
	}
	catch (...)
	{
		static plugMainUnion p;
		p.ivalue = FF_FAIL;

		return p;
	}
}

