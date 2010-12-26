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

#include "Freenect.h"

using namespace std;

static map <string, Freenect *> freenect_devices;

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

Scheme_Object *freenect_open(int argc, Scheme_Object **argv)
{
	Scheme_Object *ret = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_VAR_IN_REG(1, ret);
	MZ_GC_REG();

	if (!SCHEME_NUMBERP(argv[0]))
	{
		scheme_wrong_type("freenect-open", "number", 0, argc, argv);
		MZ_GC_UNREG();
		return scheme_void;
	}

	int id = SCHEME_INT_VAL(argv[0]);
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

Scheme_Object *scheme_reload(Scheme_Env *env)
{
	Scheme_Env *menv = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, env);
	MZ_GC_VAR_IN_REG(1, menv);
	MZ_GC_REG();

	// add all the modules from this extension
	menv = scheme_primitive_module(scheme_intern_symbol("fluxus-freenect"), env);

	scheme_add_global("freenect-get-num-devices", scheme_make_prim_w_arity(freenect_get_num_devices, "freenect-get-num-devices", 0, 0), menv);
	scheme_add_global("freenect-open", scheme_make_prim_w_arity(freenect_open, "freenect-open", 1, 1), menv);

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

