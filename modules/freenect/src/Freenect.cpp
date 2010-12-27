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

#include <string.h>

#include "OpenGL.h"
#include "Freenect.h"

freenect_context *Freenect::ctx = NULL;

Freenect::Freenect(int id) :
	device(new Device(id))
{
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
	new_rgb_frame(false)
{
	if (freenect_open_device(get_context(), &handle, id) < 0)
		throw ExcFreenectOpenDevice();

	rgb_pixels = new uint8_t[640 * 480 * 3];
	rgb_txt = new VideoTexture(640, 480, GL_RGB);

	depth_pixels = new uint16_t[640 * 480];
	depth_txt = new VideoTexture(640, 480, GL_LUMINANCE, GL_UNSIGNED_SHORT);

	pthread_mutex_init(&mutex, NULL);

	freenect_set_user(handle, this);
	tilt = freenect_get_tilt_degs(freenect_get_tilt_state(handle));
	freenect_set_video_format(handle, FREENECT_VIDEO_RGB);
	freenect_set_video_callback(handle, video_cb);
	freenect_set_depth_format(handle, FREENECT_DEPTH_11BIT);
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

	memcpy(device->rgb_pixels, rgb, 640 * 480 * 3 * sizeof(uint8_t));
	device->new_rgb_frame = true;

	pthread_mutex_unlock(&(device->mutex));
}

void Freenect::depth_cb(freenect_device *dev, void *vdepth, uint32_t timestamp)
{
	Freenect::Device *device = reinterpret_cast<Freenect::Device *>(freenect_get_user(dev));
	pthread_mutex_lock(&(device->mutex));

	uint16_t *depth = reinterpret_cast<uint16_t *>(vdepth);
	for (int i = 0; i < FREENECT_FRAME_PIX; i++)
	{
		uint32_t v = depth[i];
		device->depth_pixels[i] = 65535 - ((v * v) >> 4);
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
	}
	if (new_depth_frame)
	{
		depth_txt->upload(depth_pixels);
		new_depth_frame = false;
	}
	pthread_mutex_unlock(&mutex);
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

