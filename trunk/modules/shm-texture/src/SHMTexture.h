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

#ifndef SHMTEXTURE_H
#define SHMTEXTURE_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "VideoTexture.h"

class SHMTexture: public VideoTexture
{
	public:
			SHMTexture(int key, int w, int h, int f);
			~SHMTexture();

			class Error { };

			void update();

	protected:

			int shmid;
			unsigned char *shm;
};

#endif

