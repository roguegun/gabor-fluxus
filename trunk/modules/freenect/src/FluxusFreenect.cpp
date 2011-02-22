// Copyright (C) 2010 Gabor Papp
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

#include "SchemeHelper.h"
#include "Freenect.h"

using namespace std;
using namespace SchemeHelper;

static map <string, Freenect *> freenect_devices;
static Freenect *grabbed_device = NULL;

// StartFunctionDoc-en
// freenect-get-num-devices
// Returns: devices-number
// Description:
// Example:
// EndFunctionDoc

Scheme_Object *freenect_get_num_devices(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = NULL;
	MZ_GC_DECL_REG(1);
	MZ_GC_VAR_IN_REG(0, ret);
	MZ_GC_REG();

	ret = scheme_make_integer_value(Freenect::get_num_devices());

	MZ_GC_UNREG();
	return ret;
}

// StartFunctionDoc-en
// freenect-open
// Returns: deviceid-symbol
// Description:
// Example:
// EndFunctionDoc

Scheme_Object *freenect_open(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, ret);
	MZ_GC_REG();

	ArgCheck("freenect-open", "i", argc, argv);

	int id = IntFromScheme(argv[0]);
	stringstream ss;
	ss << id;
	string strid = "freenect" + ss.str();
	map<string, Freenect *>::iterator i = freenect_devices.find(strid);
	if (i == freenect_devices.end())
	{
		Freenect *f;
		try
		{
			f = new Freenect(SCHEME_INT_VAL(argv[0]));
		}
		catch (Freenect::ExcFreenectOpenDevice &e)
		{
			cerr << "freenect: cannot open device " << id << endl;
			MZ_GC_UNREG();
			return scheme_void;
		}
		freenect_devices[strid] = f;
	}

	ret = scheme_intern_symbol(strid.c_str());
	MZ_GC_UNREG();
	return ret;
}

Scheme_Object *freenect_grab_device(int argc, Scheme_Object **argv)
{
	DECL_ARGV();
	ArgCheck("freenect-grab-device", "S", argc, argv);

	string strid(SCHEME_SYM_VAL(argv[0]));

	map<string, Freenect *>::iterator i = freenect_devices.find(strid);
	if (i != freenect_devices.end())
	{
		grabbed_device = i->second;
	}
	else
	{
		cerr << "freenect: cannot grab device " << strid << endl;
		grabbed_device = NULL;
	}

	MZ_GC_UNREG();
	return scheme_void;
}

Scheme_Object *freenect_ungrab_device(int argc, Scheme_Object **argv)
{
	grabbed_device = NULL;
	return scheme_void;
}

Scheme_Object *freenect_set_tilt(int argc, Scheme_Object **argv)
{
	DECL_ARGV();
	ArgCheck("freenect-set-tilt", "f", argc, argv);

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-set-tilt can only be used while a freenect device is grabbed." << endl;
	}
	else
	{
		grabbed_device->set_tilt(FloatFromScheme(argv[0]));

	}
	MZ_GC_UNREG();
	return scheme_void;
}

Scheme_Object *freenect_get_tilt(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = scheme_void;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, ret);
	MZ_GC_REG();

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-get-tilt can only be used while a freenect device is grabbed." << endl;
	}
	else
	{
		ret = scheme_make_double(grabbed_device->get_tilt());
	}

	MZ_GC_UNREG();
	return ret;
}

Scheme_Object *freenect_get_rgb_texture(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = scheme_void;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, ret);
	MZ_GC_REG();

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-get-rgb-texture can only be used while a freenect device is grabbed." << endl;
	}
	else
	{
		ret = scheme_make_integer_value(grabbed_device->get_rgb_texture_id());
	}

	MZ_GC_UNREG();
	return ret;
}

Scheme_Object *freenect_get_depth_texture(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = scheme_void;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, ret);
	MZ_GC_REG();

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-get-depth-texture can only be used while a freenect device is grabbed." << endl;
	}
	else
	{
		ret = scheme_make_integer_value(grabbed_device->get_depth_texture_id());
	}

	MZ_GC_UNREG();
	return ret;
}

// StartFunctionDoc-en
// freenect-set-depth-mode mode-symbol
// Returns: void
// Description:
// Sets how the depth values are converted. Mode is one of 'raw, 'scaled, 'hist.
// 'raw returns the original 11-bit values, 'scaled scales them up to the visible range,
// 'hist moves the values into the visually sensible space using a histogram. The default
// mode is 'hist.
// Example:
// (freenect-set-depth-mode 'hist)
// EndFunctionDoc

Scheme_Object *freenect_set_depth_mode(int argc, Scheme_Object **argv)
{
	DECL_ARGV();

	ArgCheck("freenect-set-depth-mode", "S", argc, argv);
	string m = SymbolName(argv[0]);
	int mode = -1;

	string modes[] = {"raw", "scaled", "hist"};
	for (unsigned i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
	{
		if (m == modes[i])
		{
			mode = i;
			break;
		}
	}
	if (mode == -1)
	{
		cerr << "freenect-set-depth-mode: unknown depth mode." << endl;
	}
	else
	{
		Freenect::set_depth_mode(mode);
	}

	MZ_GC_UNREG();
	return scheme_void;
}

Scheme_Object *freenect_get_rgb_calibrated_texture(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = scheme_void;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, ret);
	MZ_GC_REG();

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-get-rgb-calibrated-texture can only be used while a freenect device is grabbed." << endl;
	}
	else
	{
		ret = scheme_make_integer_value(grabbed_device->get_rgb_calibrated_texture_id());
	}

	MZ_GC_UNREG();
	return ret;
}

Scheme_Object *freenect_update(int argc, Scheme_Object **argv)
{
	DECL_ARGV();

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-update can only be used while a freenect device is grabbed." << endl;
	}
	else
	{
		grabbed_device->update();
	}

	MZ_GC_UNREG();
	return scheme_void;
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
// freenect-tcoords
// Returns: list-of-texture-coordinates
// Description:
// Returns the texture coordinates of the video textures. This is necessary,
// because video images are rectangular non-power-of-two textures, while
// fluxus uses GL_TEXTURE_2D power-of-two textures.
// Example:
// EndFunctionDoc

Scheme_Object *freenect_tcoords(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = NULL;
	Scheme_Object **coord_list = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, coord_list);
	MZ_GC_REG();

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-tcoords can only be used while a freenect device is grabbed." << endl;
		ret = scheme_void;
	}
	else
	{
		coord_list = (Scheme_Object **)scheme_malloc(4 *
				sizeof(Scheme_Object *));

		float *coords  = grabbed_device->get_tcoords();

		coord_list[0] = scheme_vector(coords[0], coords[4], coords[2]);
		coord_list[1] = scheme_vector(coords[3], coords[4], coords[5]);
		coord_list[2] = scheme_vector(coords[3], coords[1], coords[5]);
		coord_list[3] = scheme_vector(coords[0], coords[1], coords[2]);

		ret = scheme_build_list(4, coord_list);
	}

	MZ_GC_UNREG();
	return ret;
}

Scheme_Object *freenect_worldcoord_at(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, ret);
	MZ_GC_REG();

	ArgCheck("freenect-worldcoord-at", "ii", argc, argv);

	if (grabbed_device == NULL)
	{
		cerr << "freenect: freenect-worldcoord-at can only be used while a freenect device is grabbed." << endl;
		ret = scheme_void;
	}
	else
	{
		Vector v = grabbed_device->worldcoord_at(IntFromScheme(argv[0]), IntFromScheme(argv[1]));
		ret = scheme_vector(v.x, v.y, v.z);
	}
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

	menv = scheme_primitive_module(scheme_intern_symbol("fluxus-freenect"), env);

	scheme_add_global("freenect-get-num-devices", scheme_make_prim_w_arity(freenect_get_num_devices, "freenect-get-num-devices", 0, 0), menv);
	scheme_add_global("freenect-open", scheme_make_prim_w_arity(freenect_open, "freenect-open", 1, 1), menv);
	scheme_add_global("freenect-grab-device", scheme_make_prim_w_arity(freenect_grab_device, "freenect-grab-device", 1, 1), menv);
	scheme_add_global("freenect-ungrab-device", scheme_make_prim_w_arity(freenect_ungrab_device, "freenect-ungrab-device", 0, 0), menv);
	scheme_add_global("freenect-set-tilt", scheme_make_prim_w_arity(freenect_set_tilt, "freenect-set-tilt", 1, 1), menv);
	scheme_add_global("freenect-get-tilt", scheme_make_prim_w_arity(freenect_get_tilt, "freenect-get-tilt", 0, 0), menv);
	scheme_add_global("freenect-get-rgb-texture", scheme_make_prim_w_arity(freenect_get_rgb_texture, "freenect-get-rgb-texture", 0, 0), menv);
	scheme_add_global("freenect-get-depth-texture", scheme_make_prim_w_arity(freenect_get_depth_texture, "freenect-get-depth-texture", 0, 0), menv);
	scheme_add_global("freenect-get-rgb-calibrated-texture", scheme_make_prim_w_arity(freenect_get_rgb_calibrated_texture, "freenect-get-rgb-calbirated-texture", 0, 0), menv);
	scheme_add_global("freenect-set-depth-mode", scheme_make_prim_w_arity(freenect_set_depth_mode, "freenect-set-depth-mode", 1, 1), menv);
	scheme_add_global("freenect-update", scheme_make_prim_w_arity(freenect_update, "freenect-update", 0, 0), menv);
	scheme_add_global("freenect-tcoords", scheme_make_prim_w_arity(freenect_tcoords, "freenect-tcoords", 0, 0), menv);
	scheme_add_global("freenect-worldcoord-at", scheme_make_prim_w_arity(freenect_worldcoord_at, "freenect-worldcoord-at", 2, 2), menv);

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
	return scheme_intern_symbol("fluxus-freenect");
}

