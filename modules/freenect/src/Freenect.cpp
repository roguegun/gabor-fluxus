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

#include "Freenect.h"

freenect_context *Freenect::ctx = NULL;

freenect_context *Freenect::get_context()
{
	if (!ctx)
	{
		if (freenect_init(&ctx, NULL) < 0)
		{
			throw ExcFreenectInit();
		}
		freenect_set_log_level(ctx, FREENECT_LOG_DEBUG);
	}

	return ctx;
}

int Freenect::get_num_devices()
{
	try
	{
		return freenect_num_devices(get_context());
	}
	catch (ExcFreenectInit &e)
	{
		return 0;
	}
}

