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

#include <iostream>
#include <math.h>
#include <string.h>

#include "OpenGL.h"
#include "Freenect.h"

using namespace std;

freenect_context *Freenect::ctx = NULL;

bool Freenect::luts = false;
unsigned *Freenect::rgb2depth_lut = NULL;
float *Freenect::distance_lut = NULL;

int Freenect::depth_mode = Freenect::DEPTH_HIST;
unsigned Freenect::depth_hist[2048];

Freenect::Freenect(int id) :
	device(new Device(id))
{
	if (!luts)
	{
		// calculating lookup table for rgb and depth image calibration
		rgb2depth_lut = new unsigned[FREENECT_FRAME_W * FREENECT_FRAME_H];
		// relative transform of the 2 images
		const float m[4][4] =
			{{0.942040, -0.004628, 0.000000, 0.000005},
			 {-0.005672, 0.939875, 0.000000, 0.000003},
			 {0.000000, 0.000000, 0.000000, 0.000000},
			 {23.953022, 31.486654, 0.000000, 1.000000}};
		unsigned *r2d_lut = rgb2depth_lut;

		Vector uv, t;
		for (int y = 0; y < FREENECT_FRAME_H; y++)
		{
			for (int x = 0; x < FREENECT_FRAME_W; x++)
			{
				uv = Vector(x, y);
				//uv.x = x; uv.y = y; uv.z = 0; uv.w = 1;

				t.x = uv.x * m[0][0] + uv.y * m[1][0] + uv.z * m[2][0] + uv.w * m[3][0];
				t.y = uv.x * m[0][1] + uv.y * m[1][1] + uv.z * m[2][1] + uv.w * m[3][1];
				t.z = uv.x * m[0][2] + uv.y * m[1][2] + uv.z * m[2][2] + uv.w * m[3][2];
				t.w = uv.x * m[0][3] + uv.y * m[1][3] + uv.z * m[2][3] + uv.w * m[3][3];

				t.x /= t.w;
				t.y /= t.w;

				if (t.x < 0)
					t.x = 0;
				else if (t.x > 639)
					t.x = 639;
				if (t.y < 0)
					t.y = 0;
				else if (t.y > 479)
					t.y = 479;

				*r2d_lut++ = (((int)(t.y)) * FREENECT_FRAME_W + (int)t.x) * 3;
			}
		}

		// depth to distance
		distance_lut = new float [2048];
		// equation from http://openkinect.org/wiki/Imaging_Information
		const float k1 = 0.1236;
		const float k2 = 2842.5;
		const float k3 = 1.1863;
		const float k4 = 0.0370;
		for (unsigned i = 0; i < 2048; i++)
		{
			distance_lut[i] = k1 * tanf(i / k2 + k3) - k4;
		}

		luts = true;
	}
}

Freenect::~Freenect()
{
	delete device;
}

freenect_context *Freenect::get_context()
{
	if (!ctx)
	{
		if (freenect_init(&ctx, NULL) < 0)
		{
			throw ExcFreenectInit();
		}
		freenect_set_log_level(ctx, FREENECT_LOG_ERROR);
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

Freenect::Device::Device(int id) :
	thread_die(false),
	new_rgb_frame(false),
	infrared(false)
{
	if (freenect_open_device(get_context(), &handle, id) < 0)
		throw ExcFreenectOpenDevice();

	rgb_pixels = new uint8_t[FREENECT_FRAME_W * FREENECT_FRAME_H * 3];
	rgb_txt = new VideoTexture(FREENECT_FRAME_W, FREENECT_FRAME_H, GL_RGB);

	depth_pixels = new uint16_t[FREENECT_FRAME_W * FREENECT_FRAME_H];
	depth_txt = new VideoTexture(FREENECT_FRAME_W, FREENECT_FRAME_H, GL_LUMINANCE, GL_UNSIGNED_SHORT);

	rgb_calibrated_pixels = new uint8_t[FREENECT_FRAME_W * FREENECT_FRAME_H * 3];
	rgb_calibrated_txt = new VideoTexture(FREENECT_FRAME_W, FREENECT_FRAME_H, GL_RGB);

	pthread_mutex_init(&mutex, NULL);

	freenect_set_user(handle, this);
	tilt = freenect_get_tilt_degs(freenect_get_tilt_state(handle));
	freenect_set_video_mode(handle, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
	freenect_set_depth_mode(handle, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	//freenect_set_video_format(handle, FREENECT_VIDEO_RGB);
	freenect_set_video_callback(handle, video_cb);
	freenect_set_video_buffer(handle, rgb_pixels);
	//freenect_set_depth_format(handle, FREENECT_DEPTH_11BIT);
	freenect_set_depth_callback(handle, depth_cb);

	pthread_create(&thread, NULL, &thread_func, this);
}

Freenect::Device::~Device()
{
	thread_die = true;
	pthread_join(thread, NULL);
	pthread_mutex_destroy(&mutex);

	delete rgb_txt;
	delete [] rgb_pixels;
	delete depth_txt;
	delete [] depth_pixels;
	delete rgb_calibrated_txt;
	delete [] rgb_calibrated_pixels;
}

void *Freenect::thread_func(void *vdev)
{
	Device *dev = reinterpret_cast<Device *>(vdev);

	freenect_start_depth(dev->handle);
	freenect_start_video(dev->handle);

	while ((!dev->thread_die) &&
		   (freenect_process_events(get_context()) >= 0))
		;

	freenect_close_device(dev->handle);
	return NULL;
}

void Freenect::video_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
	Freenect::Device *device = reinterpret_cast<Freenect::Device *>(freenect_get_user(dev));
	pthread_mutex_lock(&(device->mutex));

	if (device->infrared)
	{
		uint8_t *rgb8 = reinterpret_cast<uint8_t *>(rgb);
		int j = 0;
		for (int i = 0; i < FREENECT_FRAME_PIX; i++)
		{
			device->rgb_pixels[j] = device->rgb_pixels[j + 1] =
				device->rgb_pixels[j + 2] = rgb8[i];
			j += 3;
		}
	}
	else
		memcpy(device->rgb_pixels, rgb, FREENECT_FRAME_W * FREENECT_FRAME_H * 3 * sizeof(uint8_t));

	device->new_rgb_frame = true;

	pthread_mutex_unlock(&(device->mutex));
}

void Freenect::depth_cb(freenect_device *dev, void *vdepth, uint32_t timestamp)
{
	Freenect::Device *device = reinterpret_cast<Freenect::Device *>(freenect_get_user(dev));
	pthread_mutex_lock(&(device->mutex));

	uint16_t *depth = reinterpret_cast<uint16_t *>(vdepth);

	if (depth_mode == DEPTH_RAW) // raw 11-bit depth data
	{
		memcpy(device->depth_pixels, depth, FREENECT_FRAME_PIX * sizeof(uint16_t));
	}
	else if (depth_mode == DEPTH_SCALED) // scaled up to 16-bit range
	{
		for (int i = 0; i < FREENECT_FRAME_PIX; i++)
		{
			device->depth_pixels[i] = 65535 - (depth[i] << 5);
		}
	}
	else if (depth_mode == DEPTH_HIST) // remapped according to histogram
	{
		memset(depth_hist, 0, 2048 * sizeof(uint16_t));

		unsigned n = 0;
		for (int i = 0; i < FREENECT_FRAME_PIX; i++)
		{
			int v = depth[i];
			if (v)
			{
				depth_hist[v]++;
				n++;
			}
		}

		for (int i = 1; i < 2048; i++)
		{
			depth_hist[i] += depth_hist[i - 1];
		}

		if (n)
		{
			for (int i = 0; i < 2048; i++)
			{
				depth_hist[i] = static_cast<unsigned>(65535 *
						(1.f - static_cast<float>(depth_hist[i]) / static_cast<float>(n)));
			}
			depth_hist[0] = 0;
		}

		for (int i = 0; i < FREENECT_FRAME_PIX; i++)
		{
			device->depth_pixels[i] = depth_hist[depth[i]];
		}
	}

	device->new_depth_frame = true;

	pthread_mutex_unlock(&(device->mutex));
}

void Freenect::Device::update()
{
	pthread_mutex_lock(&mutex);
	if (new_rgb_frame)
	{
		rgb_txt->upload(rgb_pixels);
		new_rgb_frame = false;
		update_rgb_calibrated();
	}
	if (new_depth_frame)
	{
		depth_txt->upload(depth_pixels);
		new_depth_frame = false;
	}
	pthread_mutex_unlock(&mutex);

}

void Freenect::Device::update_rgb_calibrated()
{
	for (int i = 0, j = 0; i < FREENECT_FRAME_PIX; i++, j += 3)
	{
		unsigned off = rgb2depth_lut[i];
		rgb_calibrated_pixels[j] = rgb_pixels[off];
		rgb_calibrated_pixels[j + 1] = rgb_pixels[off + 1];
		rgb_calibrated_pixels[j + 2] = rgb_pixels[off + 2];
	}
	rgb_calibrated_txt->upload(rgb_calibrated_pixels);
}

void Freenect::update()
{
	device->update();
}

void Freenect::set_tilt(float degrees)
{
	if (degrees < -31)
		degrees = -31;
	else
	if (degrees > 31)
		degrees = 31;

	device->tilt = degrees;
	freenect_set_tilt_degs(device->handle, degrees);
}

float Freenect::depth_at(int x, int y)
{
	if ((x >= FREENECT_FRAME_W) || (x < 0) ||
		(y >= FREENECT_FRAME_H) || (y < 0))
	{
		return 0;
	}
	else
	{
		return device->depth_pixels[y * FREENECT_FRAME_W + x] / 65535.0f;
	}
}

Vector Freenect::worldcoord_at(int x, int y)
{
	// http://graphics.stanford.edu/~mdfisher/Kinect.html
	float depth = 0;

	if ((x >= 0) && (x < FREENECT_FRAME_W) &&
		(y >= 0) && (y < FREENECT_FRAME_H))
	{
		depth = distance_lut[device->depth_pixels[y * FREENECT_FRAME_W + x]];
	}

	static const double fx_d = 1.0 / 5.9421434211923247e+02;
	static const double fy_d = 1.0 / 5.9104053696870778e+02;
	static const double cx_d = 3.3930780975300314e+02;
	static const double cy_d = 2.4273913761751615e+02;

	Vector result;
	result.x = float((x - cx_d) * depth * fx_d);
	result.y = float((y - cy_d) * depth * fy_d);
	result.z = float(depth);

	return result;
}

void *Freenect::get_depth_pixels()
{
	return static_cast<void *>(device->depth_pixels);
}

void Freenect::set_video_infrared(bool infra /* = true */)
{
	if (device->infrared != infra)
	{
		cout << "stop video" << endl;
        freenect_stop_video(device->handle);
		cout << "lock mutex" << endl;
		pthread_mutex_lock(&(device->mutex));
		device->infrared = infra;
		cout << "set format" << endl;
		/*
		if (device->infrared)
			freenect_set_video_format(device->handle, FREENECT_VIDEO_IR_8BIT);
		else
			freenect_set_video_format(device->handle, FREENECT_VIDEO_RGB);
		*/
		cout << "unlock mutex" << endl;
		pthread_mutex_unlock(&(device->mutex));
		cout << "start video" << endl;
        freenect_start_video(device->handle);
    }
}

