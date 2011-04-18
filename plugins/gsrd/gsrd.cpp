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
#include "gsrd.h"

using namespace std;

const char *GSRD::vertex_shader ="\n\
void main(void) \n\
{ \n\
	gl_Position = ftransform(); \n\
	gl_TexCoord[0] = gl_MultiTexCoord0; \n\
	gl_FrontColor = gl_Color; \n\
} \n\
";

const char *GSRD::fragment_shader = "\n\
#version 120 \n\
\n\
uniform sampler2D tex; // u = r, v = b \n\
uniform float dU; \n\
uniform float dV; \n\
uniform float f; \n\
uniform float k; \n\
uniform float width; \n\
\n\
void main(void) \n\
{ \n\
	vec2 tidx = gl_TexCoord[0].st; \n\
	float s = 1.0 / width; \n\
\n\
	vec2 txt_clr = texture2D(tex, tidx).rb;\n\
\n\
	vec2 top_idx = tidx + vec2(0., -s); \n\
	vec2 bottom_idx = tidx + vec2(0, s); \n\
	vec2 left_idx = tidx + vec2(-s, 0); \n\
	vec2 right_idx = tidx + vec2(s, 0); \n\
\n\
	float currF = f; \n\
	float currK = k; \n\
	float currU = txt_clr.r; \n\
	float currV = txt_clr.g; \n\
	float d2 = currU * currV * currV; \n\
\n\
	vec2 uv = clamp(txt_clr + \n\
			vec2(dU, dV) * \n\
				 (texture2D(tex, right_idx).rb + \n\
				  texture2D(tex, left_idx).rb + \n\
				  texture2D(tex, bottom_idx).rb + \n\
				  texture2D(tex, top_idx).rb - \n\
				  4. * txt_clr) + \n\
			vec2(-d2 + currF * (1. - currU), \n\
			      d2 - currK * currV), 0., 1.); \n\
\n\
	gl_FragColor = vec4(uv.x, 0.0, uv.y, 1.0);\n\
} \n\
";

const char *GSRD::seed_fragment_shader = "\n\
uniform sampler2D tex; \n\
void main(void) \n\
{ \n\
	vec4 c = texture2D(tex, gl_TexCoord[0].st); \n\
	float br = c.a * dot(vec3(.3, .59, .11), c.rgb); \n\
	gl_FragColor = gl_Color * br; \n\
} \n\
";

GLSLProg *GSRD::shader = NULL;
GLSLProg *GSRD::seed_shader = NULL;

GSRD::GSRD(FFGLViewportStruct *vps) : FFGLPlugin(vps)
{
	/* this is called when the plugin is instantiated */
	if (shader == NULL)
		shader = new GLSLProg(vertex_shader, fragment_shader);
	if (seed_shader == NULL)
		seed_shader = new GLSLProg(vertex_shader, seed_fragment_shader);

	FBO::Format format;
	format.set_num_color_buffers(2);
	format.set_color_internal_format(GL_RGBA32F_ARB);

	fbo = new FBO(viewport.width, viewport.height, format);

	// FIXME: do this in FBO.cpp
	for (int i = 0; i < 2; i++)
	{
		fbo->bind_texture(i);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	reset();

	current_fbo_txt = 0;
}

GSRD::GSRD()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		throw FFGLExc();
	}

	/* initialise plugin, set information */
	set_name("Gray-Scott RD");
	set_id("GSRD");

	set_type(FF_EFFECT);
	set_description("Gray-Scott Reaction Diffusion plugin");
	set_about("by Gabor Papp");
	set_version(1.0);
	set_minimum_input_frames(1);
	set_maximum_input_frames(1);

	add_parameter("dU", 0.16, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("dV", 0.08, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("k", 0.077, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("f", 0.023, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("iterations", 25., 1.0, 50.0, FF_TYPE_STANDARD);
	add_parameter("reset", 0.0, FF_TYPE_EVENT);

	if (!glewIsSupported("GL_VERSION_2_0 ") || (glCreateProgram == NULL) ||
			(glCreateShader == NULL) || (glAttachShader == NULL))
	{
		std::cerr << "OpenGL 2.0 required with GLSL support." << std::endl;
		throw FFGLExc();
	}
}

GSRD::~GSRD()
{
	/* called when the plugin is deinstantiated */
	delete fbo;
}

void GSRD::reset()
{
	glClearColor(1, 0, 0, 0);
	for (int i = 0; i < 2; i++)
	{
		fbo->bind(i);
		glClear(GL_COLOR_BUFFER_BIT);
		fbo->unbind();
	}
	glClearColor(0, 0, 0, 0);
}

void GSRD::seed(unsigned handle)
{
	glBindTexture(GL_TEXTURE_2D, handle);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (int i = 0; i < 2; i++)
	{
		fbo->bind(i);
		seed_shader->bind();
		seed_shader->uniform("tex", 0);
		glColor4f(.5, 0, .25, 1);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(-1, -1);
		glTexCoord2f(fbo->max_s, 0);
		glVertex2f(1, -1);
		glTexCoord2f(fbo->max_s, fbo->max_t);
		glVertex2f(1, 1);
		glTexCoord2f(0, fbo->max_t);
		glVertex2f(-1, 1);
		glEnd();
		glColor4f(1, 1, 1, 1);
		seed_shader->unbind();

		fbo->unbind();
	}

	glBlendFunc(GL_ONE, GL_ZERO); // default blend func
	glDisable(GL_BLEND);
}

unsigned GSRD::process_opengl(ProcessOpenGLStruct *pgl)
{
	if ((pgl->numInputTextures < 1) || (pgl->inputTextures[0] == NULL))
		return FF_FAIL;

	FFGLTextureStruct *texture = pgl->inputTextures[0];

	glEnable(GL_TEXTURE_2D);

	if (parameters[PARAM_RESET].fvalue > 0)
	{
		reset();
	}

	seed(texture->Handle);

	shader->bind();
	shader->uniform("tex", 0);
	shader->uniform("dU", parameters[PARAM_DU].fvalue);
	shader->uniform("dV", parameters[PARAM_DV].fvalue);
	shader->uniform("k", parameters[PARAM_K].fvalue);
	shader->uniform("f", parameters[PARAM_F].fvalue);
	shader->uniform("width", float(fbo->width));
	shader->unbind();

	for (int i = 0; i < (int)parameters[PARAM_ITERATIONS].fvalue; i++)
	{
		current_fbo_txt ^= 1;

		fbo->bind(current_fbo_txt);
		CHECK_GL_ERRORS("fbo bind");

		glActiveTexture(GL_TEXTURE0);
		fbo->bind_texture(current_fbo_txt ^ 1);
		CHECK_GL_ERRORS("fbo bind texture");

		shader->bind();
		CHECK_GL_ERRORS("shader bind");

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glColor4f(1, 1, 1, 1);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(-1, -1);
		glTexCoord2f(fbo->max_s, 0);
		glVertex2f(1, -1);
		glTexCoord2f(fbo->max_s, fbo->max_t);
		glVertex2f(1, 1);
		glTexCoord2f(0, fbo->max_t);
		glVertex2f(-1, 1);
		glEnd();

		fbo->unbind_texture();
		shader->unbind();

		fbo->unbind();
	}

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pgl->HostFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	fbo->bind_texture(current_fbo_txt);

	glColor4f(1, 1, 1, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(fbo->max_s, 0);
	glVertex2f(1, -1);
	glTexCoord2f(fbo->max_s, fbo->max_t);
	glVertex2f(1, 1);
	glTexCoord2f(0, fbo->max_t);
	glVertex2f(-1, 1);
	glEnd();

	fbo->unbind_texture();

	glDisable(GL_TEXTURE_2D);

	// restore default texcoord
	glTexCoord2f(0, 0);
	return FF_SUCCESS;
}

plugMainUnion plugMain(unsigned function_code, unsigned param, unsigned instance_id)
{
	try
	{
		/* creates the main plugin instance - this must not be global, because
		 * static initialization order fiasco can occur */
		static GSRD *plugin = new GSRD();
		plugin = plugin; // gets rid of unused variable warning
		return plug_main<GSRD>(function_code, param, instance_id);
	}
	catch (FFGLPlugin::FFGLExc &e)
	{
		static plugMainUnion p;
		p.ivalue = FF_FAIL;

		return p;
	}
}

