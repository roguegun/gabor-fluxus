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
#include <pthread.h>

#include <libfreenect/libfreenect.h>

#include "VideoTexture.h"

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

		void update();
		unsigned get_rgb_texture_id() { return device->rgb_txt->get_texture_id(); }
		unsigned get_depth_texture_id() { return device->depth_txt->get_texture_id(); }
		float *get_tcoords() { return device->rgb_txt->get_tcoords(); }

	private:
		static freenect_context *ctx;
		static freenect_context *get_context();

		static void video_cb(freenect_device *dev, void *rgb, uint32_t timestamp);
		static void depth_cb(freenect_device *dev, void *depth, uint32_t timestamp);

		class Device
		{
			public:
				Device(int id);
				~Device();

				freenect_device *handle;
				float tilt;

				pthread_mutex_t mutex;
				pthread_t thread;
				bool thread_die;

				uint8_t *rgb_pixels;
				VideoTexture *rgb_txt;
				bool new_rgb_frame;

				uint16_t *depth_pixels;
				VideoTexture *depth_txt;
				bool new_depth_frame;

				void update();
		};

		static void *thread_func(void *vdev);

		Device *device;
};
#endif

