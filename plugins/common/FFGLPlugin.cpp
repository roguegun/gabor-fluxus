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

#include <iostream>

#include "FFGLPlugin.h"

using namespace std;

/* pointer to the main plugin instance */
FFGLPlugin *FFGLPlugin::main = NULL;
/* plugin information structure */
PlugInfoStruct FFGLPlugin::plugin_info = {
	FF_APIMAJORVERSION, FF_APIMINORVERSION,
	"FFG", "FFGL plugin    ", FF_EFFECT };
/* extended information structure */
PlugExtendedInfoStruct FFGLPlugin::plugin_extended_info = {
	1, 0, (char *)"", (char *)"", 0, NULL };

/* default parameters */
vector<ParameterDefault> FFGLPlugin::parameter_defaults;
unsigned FFGLPlugin::parameter_count = 0;

/* minimum and maximum input frames */
unsigned FFGLPlugin::minimum_input_frames = 1;
unsigned FFGLPlugin::maximum_input_frames = 1;

/**
 * Called when the main plugin instance is created.
 **/
FFGLPlugin::FFGLPlugin() :
	parameters(NULL)
{
	main = this;
}

/**
 * Called when the plugin is instantiated. Default parameters are copied
 * to the plugin instance.
 **/
FFGLPlugin::FFGLPlugin(FFGLViewportStruct *vps) :
	parameters(NULL)
{
	if (parameter_count > 0)
	{
		parameters = new Parameter[parameter_count];
		for (unsigned i = 0; i < parameter_count; i++)
		{
			switch (parameter_defaults[i].type)
			{
				case FF_TYPE_TEXT:
					parameters[i].svalue = parameter_defaults[i].svalue;
					break;
				default:
					parameters[i].fvalue = parameter_defaults[i].fvalue.def;
					break;
			}
		}
	}

	viewport = *vps;
}

FFGLPlugin::~FFGLPlugin()
{
	if (main == this)
	{
		parameter_defaults.clear();
	}

	if (parameters != NULL)
	{
		delete [] parameters;
	}
}

/**
 * Gets information about the plugin - version, unique id, short name and type.
 *
 * \retval PluginInfoStruct* if successful, FF_FAIL otherwise
 **/
PlugInfoStruct *FFGLPlugin::get_info(void)
{
    return &plugin_info;
}

/**
 * Returns the name of the parameter as a 16 1-byte not NULL terminated char
 * array.
 *
 * \param parameter number
 *
 * \return parameter name of FF_FAIL on error
 **/
char *FFGLPlugin::get_parameter_name(unsigned i)
{
	if (i >= parameter_count)
	{
		return (char *)FF_FAIL;
	}
	else
	{
		return parameter_defaults[i].name;
	}
}

/**
 * Returns ParameterValue indicating the default value of parameter specified by
 * index. For float parameters, the return value is a 32-bit float
 * (0 <= value <= 1). Return value for text parameters is a 32-bit pointer to a NULL
 * terminated string.
 *
 * \param parameter number
 *
 * \retval ParameterValue or FF_FAIL on error
 **/
ParameterValue FFGLPlugin::get_parameter_default(unsigned i)
{
    ParameterValue p;

	if (i >= parameter_count)
	{
		p.svalue = (char *)FF_FAIL;
	}
	else
	if (parameter_defaults[i].type == FF_TYPE_TEXT)
	{
		p.svalue = (char *)parameter_defaults[i].svalue;
	}
	else
	{
		p.fvalue = (parameter_defaults[i].fvalue.def - parameter_defaults[i].fvalue.min) /
				   (parameter_defaults[i].fvalue.max - parameter_defaults[i].fvalue.min);
	}
	return p;
}

/**
 * Indicates capability of a feature.
 *
 * \param index capability index
 *
 * \return unsigned integer, mainly FF_SUPPORTED or FF_UNSUPPORTED,
 *		   or number of input frames
 **/
unsigned FFGLPlugin::get_plugin_caps(unsigned index)
{
    switch (index)
	{
		case FF_CAP_16BITVIDEO:
		case FF_CAP_24BITVIDEO:
		case FF_CAP_32BITVIDEO:
		case FF_CAP_PROCESSFRAMECOPY:
		case FF_CAP_COPYORINPLACE:
			return FF_UNSUPPORTED;
			break;

		case FF_CAP_MINIMUMINPUTFRAMES:
			return minimum_input_frames;
			break;

		case FF_CAP_MAXIMUMINPUTFRAMES:
			return maximum_input_frames;
			break;

		case FF_CAP_PROCESSOPENGL:
			return FF_SUPPORTED;
			break;

		case FF_CAP_SETTIME:
			return FF_SUPPORTED;
			break;

		default:
			return FF_UNSUPPORTED;
			break;
	}
}

/**
 * Returns extended information about the plugin.
 * /retval PluginExtendedInfoStruct* or FF_FAIL on error
 */
PlugExtendedInfoStruct *FFGLPlugin::get_extended_info()
{
    return &plugin_extended_info;
}

/**
 * Returns parameter type.
 *
 * /param i parameter number
 *
 * /return ParameterType value which tells the host the datatype of the
 *		   parameter or FF_FAIL on error
 **/
unsigned FFGLPlugin::get_parameter_type(unsigned i)
{
    if (i >= parameter_count)
		return FF_FAIL;
	else
		return parameter_defaults[i].type;
}

/**
 * Returns pointer to ParameterDisplayValue containing a string to display the
 * value of the parameter.
 *
 * /param i parameter number
 *
 * /return pointer or FF_FAIL if failed
 **/
char *FFGLPlugin::get_parameter_display(unsigned i)
{
	static char str[17];
	str[16] = 0;

	if (i >= parameter_count)
		return (char *)FF_FAIL;

	switch (parameter_defaults[i].type)
	{
		case FF_TYPE_BOOLEAN:
			if (parameters[i].fvalue > 0.0)
				sprintf(str, "true");
			else
				sprintf(str, "false");
			break;

		case FF_TYPE_EVENT:
			if (parameters[i].fvalue < 1.0)
				sprintf(str, "off");
			else
				sprintf(str, "on");
			break;

		case FF_TYPE_RED:
		case FF_TYPE_GREEN:
		case FF_TYPE_BLUE:
			snprintf(str, 16, "%d", (int)(255*parameters[i].fvalue));
			break;

		case FF_TYPE_TEXT:
			snprintf(str, 16, "%s", parameters[i].svalue);
			break;

		case FF_TYPE_XPOS:
		case FF_TYPE_YPOS:
		case FF_TYPE_STANDARD:
			snprintf(str, 16, "%.2f", parameters[i].fvalue);
			break;

		default:
			return (char *)FF_FAIL;
			break;
	}

	return str;
}

/**
 * Sets the value of a parameter.
 *
 * /param SetParameterStruct*
 *
 * /return FF_SUCCESS if successful or FF_FAIL if failed
 **/
unsigned FFGLPlugin::set_parameter(SetParameterStruct *sps)
{
	unsigned i = sps->index;
	if (i >= parameter_count)
		return FF_FAIL;

	if (parameter_defaults[i].type == FF_TYPE_TEXT)
	{
		char *s = sps->svalue;
		parameters[i].svalue = s;
	}
	else
	{
		parameters[i].fvalue = parameter_defaults[i].fvalue.min +
			sps->fvalue * (parameter_defaults[i].fvalue.max -
						  parameter_defaults[i].fvalue.min);
	}

	return FF_SUCCESS;
}

/**
 * Returns value of a parameter.
 *
 * /param i parameter number
 *
 * /retval ParameterValue or FF_FAIL on error
 **/
ParameterValue FFGLPlugin::get_parameter(unsigned i)
{
	ParameterValue p;

	if (i >= parameter_count)
		p.ivalue = FF_FAIL;
	else
	if (parameter_defaults[i].type == FF_TYPE_TEXT)
		p.svalue = (char *)parameters[i].svalue;
	else
		p.fvalue = (parameters[i].fvalue - parameter_defaults[i].fvalue.min) /
				   (parameter_defaults[i].fvalue.max - parameter_defaults[i].fvalue.min);
	return p;
}

