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
	thread_die(false)
{
	if (freenect_open_device(get_context(), &handle, id) < 0)
		throw ExcFreenectOpenDevice();

	pthread_mutex_init(&mutex, NULL);

	freenect_set_user(handle, this);
	tilt = freenect_get_tilt_degs(freenect_get_tilt_state(handle));
	freenect_set_video_format(handle, FREENECT_VIDEO_RGB);
	freenect_set_video_callback(handle, video_cb);
	freenect_set_depth_format(handle, FREENECT_DEPTH_11BIT);
	freenect_set_depth_callback(handle, depth_cb);

	pthread_create(&thread, NULL, &thread_func, this);
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

Freenect::Device::~Device()
{
	thread_die = true;
	pthread_join(thread, NULL);
	pthread_mutex_destroy(&mutex);
}

void Freenect::video_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
	Freenect::Device *device = reinterpret_cast<Freenect::Device *>(freenect_get_user(dev));
	pthread_mutex_lock(&(device->mutex));

	pthread_mutex_unlock(&(device->mutex));
}

void Freenect::depth_cb(freenect_device *dev, void *depth, uint32_t timestamp)
{
	Freenect::Device *device = reinterpret_cast<Freenect::Device *>(freenect_get_user(dev));
	pthread_mutex_lock(&(device->mutex));

	pthread_mutex_unlock(&(device->mutex));
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

