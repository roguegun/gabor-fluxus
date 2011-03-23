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

/* shaders are based on
 * rdex by Claude Heiland-Allen (http://rdex.goto10.org/)
 * and RDiffusion libcinder sample by Robert Hodgin
 * (https://github.com/cinder/Cinder/tree/master/samples/RDiffusion)
 */

const char *GSRD::vertex_shader ="\n\
void main(void) \n\
{ \n\
    gl_Position = ftransform(); \n\
    gl_TexCoord[0] = gl_MultiTexCoord0; \n\
} \n\
";

const char *GSRD::fragment_shader = "\n\
#version 120 \n\
#define KERNEL_SIZE 9 \n\
\n\
float kernel[KERNEL_SIZE]; \n\
uniform float width; \n\
\n\
vec2 offset[KERNEL_SIZE]; \n\
\n\
uniform sampler2D srcTexture; \n\
uniform sampler2D texture; // U := r, V := b, other channels ignored \n\
uniform float ru; // rate of diffusion of U \n\
uniform float rv; // rate of diffusion of V \n\
uniform float f; // some coupling parameter \n\
uniform float k; // another coupling parameter \n\
\n\
void main(void) \n\
{ \n\
	vec2 texCoord = gl_TexCoord[0].st;		// center coordinates \n\
	float w = 1.0 / width; \n\
	float h = 1.0 / width; \n\
\n\
	kernel[0] = 0.707106781; \n\
	kernel[1] = 1.0; \n\
	kernel[2] = 0.707106781; \n\
	kernel[3] = 1.0; \n\
	kernel[4] = -6.82842712; \n\
	kernel[5] = 1.0; \n\
	kernel[6] = 0.707106781; \n\
	kernel[7] = 1.0; \n\
	kernel[8] = 0.707106781; \n\
\n\
	offset[0] = vec2( -w, -h); \n\
	offset[1] = vec2(0.0, -h); \n\
	offset[2] = vec2(  w, -h); \n\
\n\
	offset[3] = vec2( -w, 0.0); \n\
	offset[4] = vec2(0.0, 0.0); \n\
	offset[5] = vec2(  w, 0.0); \n\
\n\
	offset[6] = vec2( -w, h); \n\
	offset[7] = vec2(0.0, h); \n\
	offset[8] = vec2(  w, h); \n\
\n\
	vec2 texColor = texture2D(texture, texCoord).rb; \n\
	float srcTexColor = texture2D(srcTexture, texCoord).r; \n\
\n\
	vec2 sum = vec2(0.0, 0.0); \n\
\n\
	for (int i = 0; i < KERNEL_SIZE; i++) \n\
	{ \n\
		vec2 tmp = texture2D(texture, texCoord + offset[i]).rb; \n\
		sum += tmp * kernel[i]; \n\
	} \n\
\n\
	float F	= f + srcTexColor * 0.025 - 0.0005; \n\
	float K	= k + srcTexColor * 0.025 - 0.0005; \n\
\n\
	float u	= texColor.r; \n\
	float v	= texColor.g; \n\
	float uvv = u * v * v; \n\
//============================================================================ \n\
	float du = ru * sum.r - uvv + F * (1.0 - u);	// Gray-Scott equation \n\
	float dv = rv * sum.g + uvv - (F + K) * v;		// diffusion+-reaction \n\
//============================================================================ \n\
	u += du * 0.6; \n\
	v += dv * 0.6; \n\
	gl_FragColor = vec4(clamp(u, 0.0, 1.0), 1.0 - u / v, clamp(v, 0.0, 1.0), 1.0); \n\
	//gl_FragColor = vec4(texColor.r, texColor.r, texColor.r, 1.0); \n\
} \n\
";

GLSLProg *GSRD::shader = NULL;

GSRD::GSRD(FFGLViewportStruct *vps) : FFGLPlugin(vps)
{
	/* this is called when the plugin is instantiated */
	if (shader == NULL)
		shader = new GLSLProg(vertex_shader, fragment_shader);

	fbo = new Surface(viewport.width, viewport.height, GL_TEXTURE_2D, 2);
	// FIXME: do this in Surface.cpp
	for (int i = 0; i < 2; i++)
	{
		fbo->bind_texture(i);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

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

	add_parameter("ru", 0.25, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("rv", 0.04, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("k", 0.047, 0.0, 1.0, FF_TYPE_STANDARD);
	add_parameter("f", 0.1, 0.0, 1.0, FF_TYPE_STANDARD);
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

void GSRD::reset(unsigned handle)
{
	glBindTexture(GL_TEXTURE_2D, handle);

	for (int i = 0; i < 2; i++)
	{
		fbo->bind(i);
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

		fbo->unbind();
	}
}

unsigned GSRD::process_opengl(ProcessOpenGLStruct *pgl)
{
	static bool inited = false;

	if ((pgl->numInputTextures < 1) || (pgl->inputTextures[0] == NULL))
		return FF_FAIL;

	FFGLTextureStruct *texture = pgl->inputTextures[0];

	/* maximum texture coordinates on surface */
	//float s_s = (float)texture->Width/(float)texture->HardwareWidth;
	//float s_t = (float)texture->Height/(float)texture->HardwareHeight;

	glEnable(GL_TEXTURE_2D);

	if ((parameters[PARAM_RESET].fvalue > 0) || !inited)
	{
		reset(texture->Handle);
		inited = true;
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture->Handle);

	fbo->bind(current_fbo_txt);
	CHECK_GL_ERRORS("fbo bind");

	glActiveTexture(GL_TEXTURE0);
	fbo->bind_texture(current_fbo_txt ^ 1);
	CHECK_GL_ERRORS("fbo bind texture");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	shader->bind();
	CHECK_GL_ERRORS("shader bind");

	shader->uniform("texture", 0);
	shader->uniform("srcTexture", 1);
	shader->uniform("ru", parameters[PARAM_RU].fvalue);
	shader->uniform("rv", parameters[PARAM_RV].fvalue);
	shader->uniform("k", parameters[PARAM_K].fvalue);
	shader->uniform("f", parameters[PARAM_F].fvalue);
	shader->uniform("width", float(fbo->width));

	/*
	   cerr << "params " << parameters[PARAM_RU].fvalue << " " <<
	   parameters[PARAM_RV].fvalue << " " <<
	   parameters[PARAM_K].fvalue << " " <<
	   parameters[PARAM_F].fvalue << endl;
	   */

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

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

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pgl->HostFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	fbo->bind_texture(current_fbo_txt);

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

	current_fbo_txt ^= 1;
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

