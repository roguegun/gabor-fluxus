// Copyright (C) 2011 Gabor Papp
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

#include <escheme.h>
#include <sstream>
#include <string>
#include <map>

#include "OpenNI.h"
#include "SchemeHelper.h"

using namespace std;
using namespace SchemeHelper;

static OpenNI openni;

Scheme_Object *openni_update(int argc, Scheme_Object **argv)
{
	openni.update();
	return scheme_void;
}


Scheme_Object *openni_depth_texture(int argc, Scheme_Object **argv)
{
    Scheme_Object *ret = scheme_void;
    MZ_GC_DECL_REG(2);
    MZ_GC_VAR_IN_REG(0, argv);
    MZ_GC_VAR_IN_REG(1, ret);
    MZ_GC_REG();

	ret = scheme_make_integer_value(openni.get_depth_texture_id());

    MZ_GC_UNREG();
    return ret;
}


static Scheme_Object *scheme_vector(float v0, float v1, float v2)
{
	Scheme_Object *ret = NULL;
	Scheme_Object *tmp = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, ret);
	MZ_GC_VAR_IN_REG(1, tmp);
	MZ_GC_REG();
	ret = scheme_make_vector(3, scheme_void);
	SCHEME_VEC_ELS(ret)[0] = scheme_make_double(v0);
	SCHEME_VEC_ELS(ret)[1] = scheme_make_double(v1);
	SCHEME_VEC_ELS(ret)[2] = scheme_make_double(v2);

	MZ_GC_UNREG();
	return ret;
}

// StartFunctionDoc-en
// openni-tcoords
// Returns: list-of-texture-coordinates
// Description:
// Returns the texture coordinates of the video textures. This is necessary,
// because video images are rectangular non-power-of-two textures, while
// fluxus uses GL_TEXTURE_2D power-of-two textures.
// Example:
// EndFunctionDoc

Scheme_Object *openni_tcoords(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = NULL;
	Scheme_Object **coord_list = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, coord_list);
	MZ_GC_REG();

		coord_list = (Scheme_Object **)scheme_malloc(4 *
				sizeof(Scheme_Object *));

		float *coords  = openni.get_tcoords();

		coord_list[0] = scheme_vector(coords[0], coords[4], coords[2]);
		coord_list[1] = scheme_vector(coords[3], coords[4], coords[5]);
		coord_list[2] = scheme_vector(coords[3], coords[1], coords[5]);
		coord_list[3] = scheme_vector(coords[0], coords[1], coords[2]);

		ret = scheme_build_list(4, coord_list);

	MZ_GC_UNREG();
	return ret;
}


Scheme_Object *scheme_reload(Scheme_Env *env)
{
	Scheme_Env *menv = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, env);
	MZ_GC_VAR_IN_REG(1, menv);
	MZ_GC_REG();

	menv = scheme_primitive_module(scheme_intern_symbol("fluxus-openni"), env);

	scheme_add_global("openni-update", scheme_make_prim_w_arity(openni_update, "openni-update", 0, 0), menv);
	scheme_add_global("openni-depth-texture", scheme_make_prim_w_arity(openni_depth_texture, "openni-depth-texture", 0, 0), menv);
	scheme_add_global("openni-tcoords", scheme_make_prim_w_arity(openni_tcoords, "openni-tcoords", 0, 0), menv);
	/*
	scheme_add_global("freenect-open", scheme_make_prim_w_arity(freenect_open, "freenect-open", 1, 1), menv);
	scheme_add_global("freenect-grab-device", scheme_make_prim_w_arity(freenect_grab_device, "freenect-grab-device", 1, 1), menv);
	scheme_add_global("freenect-ungrab-device", scheme_make_prim_w_arity(freenect_ungrab_device, "freenect-ungrab-device", 0, 0), menv);
	scheme_add_global("freenect-set-tilt", scheme_make_prim_w_arity(freenect_set_tilt, "freenect-set-tilt", 1, 1), menv);
	scheme_add_global("freenect-get-tilt", scheme_make_prim_w_arity(freenect_get_tilt, "freenect-get-tilt", 0, 0), menv);
	scheme_add_global("freenect-get-rgb-texture", scheme_make_prim_w_arity(freenect_get_rgb_texture, "freenect-get-rgb-texture", 0, 0), menv);
	scheme_add_global("freenect-get-depth-texture", scheme_make_prim_w_arity(freenect_get_depth_texture, "freenect-get-depth-texture", 0, 0), menv);
	scheme_add_global("freenect-get-rgb-calibrated-texture", scheme_make_prim_w_arity(freenect_get_rgb_calibrated_texture, "freenect-get-rgb-calbirated-texture", 0, 0), menv);
	scheme_add_global("freenect-update", scheme_make_prim_w_arity(freenect_update, "freenect-update", 0, 0), menv);
	scheme_add_global("freenect-tcoords", scheme_make_prim_w_arity(freenect_tcoords, "freenect-tcoords", 0, 0), menv);
	scheme_add_global("freenect-worldcoord-at", scheme_make_prim_w_arity(freenect_worldcoord_at, "freenect-worldcoord-at", 2, 2), menv);
	*/

	scheme_finish_primitive_module(menv);
	MZ_GC_UNREG();

	return scheme_void;
}

Scheme_Object *scheme_initialize(Scheme_Env *env)
{
	return scheme_reload(env);
}

Scheme_Object *scheme_module_name()
{
	return scheme_intern_symbol("fluxus-openni");
}

