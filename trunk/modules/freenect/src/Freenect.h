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

class Vector
{
	public:
		Vector()
		{
			x = y = z = 0; w = 1;
		}
		Vector(float _x, float _y, float _z = 0., float _w = 1.0)
		{
			x = _x; y = _y; z = _z; w = _w;
		}

		float x, y, z, w;
};

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
		unsigned get_rgb_calibrated_texture_id() { return device->rgb_calibrated_txt->get_texture_id(); }
		float *get_tcoords() { return device->rgb_txt->get_tcoords(); }

		static void set_depth_mode(int m) { depth_mode = m; };

		float depth_at(int x, int y);

		Vector worldcoord_at(int x, int y);

		void *get_depth_pixels();

		enum {
			DEPTH_RAW = 0,
			DEPTH_SCALED,
			DEPTH_HIST
		};
	private:
		static freenect_context *ctx;
		static freenect_context *get_context();

		static void video_cb(freenect_device *dev, void *rgb, uint32_t timestamp);
		static void depth_cb(freenect_device *dev, void *depth, uint32_t timestamp);

		static int depth_mode;
		static unsigned depth_hist[2048];

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

				uint8_t *rgb_calibrated_pixels;
				VideoTexture *rgb_calibrated_txt;

				void update();

			private:
				void update_rgb_calibrated();
		};

		static void *thread_func(void *vdev);
		static bool luts;
		static unsigned *rgb2depth_lut; // lookup table for rgb transform
		static float *distance_lut; // lookup table for distance from depth

		Device *device;
};
#endif

