/*
 * Copyright (C) 2009 Gabor Papp
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

#ifndef FFGLPLUGIN_H
#define FFGLPLUGIN_H

#include <iostream>
#include <vector>

#include <string.h>

#include "FFGL.h"

#define FF_APIMAJORVERSION 1
#define FF_APIMINORVERSION 500

typedef union
{
	float fvalue;
	const char *svalue;
} Parameter;

class ParameterDefault
{
	public:
		ParameterDefault(const char *name, float def, float min, float max, int type)
		{
			strncpy(this->name, name, 16);
			this->fvalue.def = def;
			this->fvalue.min = min;
			this->fvalue.max = max;
			this->type = type;
		}

		ParameterDefault(const char *name, const char *text, int type)
		{
			strncpy(this->name, name, 16);
			this->svalue = text;
			this->type = type;
		}

		char name[16];
		union
		{
			struct
			{
				float def, min, max;
			} fvalue;
			const char *svalue;
		};
		int type;
};

class FFGLPlugin
{
	public:
		FFGLPlugin();
		FFGLPlugin(FFGLViewportStruct *vps);
		virtual ~FFGLPlugin();

		static FFGLPlugin *get_main()
		{
			return main;
		}

		static PlugInfoStruct *get_info();
		static unsigned get_num_parameters() { return parameter_count; }
		static char *get_parameter_name(unsigned i);
		static ParameterValue get_parameter_default(unsigned i);
		static unsigned get_plugin_caps(unsigned index);
		static PlugExtendedInfoStruct *get_extended_info();
		static unsigned get_parameter_type(unsigned i);

		virtual unsigned initialise() { return FF_SUCCESS; }
		virtual unsigned deinitialise() { return FF_SUCCESS; }

		char *get_parameter_display(unsigned i);
		unsigned set_parameter(SetParameterStruct *sps);
		ParameterValue get_parameter(unsigned i);

		virtual unsigned process_opengl(ProcessOpenGLStruct *pgl) = 0;
		unsigned set_time(double *t) { time = *t; return FF_SUCCESS; }

	protected:
		static void set_name(const char *name)
		{
			strncpy((char *)plugin_info.pluginName, name, 16);
		}

		static void set_id(const char *id)
		{
			strncpy((char *)plugin_info.uniqueID, id, 4);
		}

		static void set_type(unsigned type)
		{
			plugin_info.pluginType = type;
		}

		static void set_minimum_input_frames(unsigned i) { minimum_input_frames = i; }
		static void set_maximum_input_frames(unsigned i) { maximum_input_frames = i; }

		static void set_version(float v)
		{
			plugin_extended_info.PluginMajorVersion = (int)v;
			plugin_extended_info.PluginMinorVersion = (int)(100 * (v - (int)v));
		}

		static void set_description(std::string desc)
		{
			static std::string desc_str = desc;
			plugin_extended_info.Description = (char *)(desc_str.c_str());
		}

		static void set_about(std::string about)
		{
			static std::string about_str = about;
			plugin_extended_info.About = (char *)(about_str.c_str());
		}

		void add_parameter(const char *name, float def, float min, float max, int type)
		{
			if (this == main)
			{
				parameter_defaults.push_back(ParameterDefault(name, def, min, max, type));
				parameter_count = parameter_defaults.size();
			}
		}

		void add_parameter(const char *name, float def, int type)
		{
			if (this == main)
			{
				parameter_defaults.push_back(ParameterDefault(name, def, 0, 1, type));
				parameter_count = parameter_defaults.size();
			}
		}

		void add_parameter(const char *name, const char *text, int type)
		{
			if (this == main)
			{
				parameter_defaults.push_back(ParameterDefault(name, text, type));
				parameter_count = parameter_defaults.size();
			}
		}

		double time;
		FFGLViewportStruct viewport;

		static unsigned minimum_input_frames;
		static unsigned maximum_input_frames;
		static unsigned parameter_count;

		Parameter *parameters;

	private:
		static FFGLPlugin *main;

		static PlugInfoStruct plugin_info;
		static PlugExtendedInfoStruct plugin_extended_info;

		static std::vector<ParameterDefault> parameter_defaults;

};

/**
 * Only exported function of the plugin dynamic library.
 *
 * /param function_code tells the plugin which function is called
 * /param 32-bit value or 32-bit pointer to some structure,
 *		  depending on function code
 * /instance_id 32-bit instance identifier. Only used for instance
 *				specific functions
 *
 * /return depends on function code, see function table for details.
 **/
template <class T>
plugMainUnion plug_main(unsigned function_code, unsigned param, unsigned instance_id)
{
	plugMainUnion retval;
	ParameterValue p;

	T *plugobj = (T *)instance_id;

	switch (function_code)
	{
		case FF_GETINFO:
			retval.PISvalue = T::get_info();
			break;
		case FF_INITIALISE:
			retval.ivalue = T::get_main()->initialise();
			break;
		case FF_DEINITIALISE:
			retval.ivalue = T::get_main()->deinitialise();
			break;
		case FF_GETNUMPARAMETERS:
			retval.ivalue = T::get_num_parameters();
			break;
		case FF_GETPARAMETERNAME:
			retval.svalue = T::get_parameter_name(param);
			break;
		case FF_GETPARAMETERDEFAULT:
			p = T::get_parameter_default(param);
			retval.ivalue = p.ivalue;
			break;
		case FF_GETPLUGINCAPS:
			retval.ivalue = T::get_plugin_caps(param);
			break;
		case FF_GETEXTENDEDINFO:
			retval.ivalue = (unsigned)(T::get_extended_info());
			break;
		case FF_GETPARAMETERTYPE:
			retval.ivalue = T::get_parameter_type(param);
			break;
		case FF_GETPARAMETERDISPLAY:
			retval.svalue = plugobj->get_parameter_display(param);
			break;
		case FF_SETPARAMETER:
			retval.ivalue = plugobj->set_parameter((SetParameterStruct *)param);
			break;
		case FF_GETPARAMETER:
			p = plugobj->get_parameter(param);
			retval.ivalue = p.ivalue;
			break;
		case FF_INSTANTIATEGL:
		{
			T *pi = new T((FFGLViewportStruct *)param);
			retval.ivalue = (unsigned)pi;
			break;
		}
		case FF_DEINSTANTIATEGL:
			delete plugobj;
			retval.ivalue = FF_SUCCESS;
			break;
		case FF_GETINPUTSTATUS:
			retval.ivalue = FF_INPUT_INUSE;
			break;
		case FF_PROCESSOPENGL:
			retval.ivalue = plugobj->process_opengl((ProcessOpenGLStruct *)param);
			break;
		case FF_SETTIME:
			retval.ivalue = plugobj->set_time((double *)param);
			break;
		default:
			retval.ivalue = FF_FAIL;
			break;
	}
	return retval;
}

#endif

