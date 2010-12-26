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

#ifndef FREENECT_H
#define FREENECT_H

#include <iostream>

#include <libfreenect/libfreenect.h>

class Exc : public std::exception {};

class Freenect
{
	public:
		Freenect(int id);
		~Freenect();

		void set_tilt(float degrees);
		float get_tilt() { return device->tilt; }

		static int get_num_devices();

		class ExcFreenectInit : public Exc {};
		class ExcFreenectOpenDevice : public Exc {};

	private:
		static freenect_context *ctx;
		static freenect_context *get_context();

		class Device
		{
			public:
				Device(int id);

				freenect_device *dev;
				float tilt;
		};

		Device *device;
};
#endif

