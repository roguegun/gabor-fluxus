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

#ifndef GSRD_H
#define GSRD_H

#include "FFGLPlugin.h"
//#define DEBUG_GL
#include "DebugGL.h"
#include "GLSLProg.h"
#include "FBO.h"

class GSRD : public FFGLPlugin
{
	public:
		GSRD();
		GSRD(FFGLViewportStruct *vps);
		~GSRD();

		unsigned process_opengl(ProcessOpenGLStruct *pgl);

	private:
		void reset(unsigned handle);

		int current_fbo_txt;
		FBO *fbo;

		static GLSLProg *shader;
		static const char *vertex_shader;
		static const char *fragment_shader;

		static GLSLProg *seed_shader;
		static const char *seed_fragment_shader;

		enum {
			PARAM_DU = 0,
			PARAM_DV,
			PARAM_K,
			PARAM_F,
			PARAM_ITERATIONS,
			PARAM_RESET
		};
};

#endif

