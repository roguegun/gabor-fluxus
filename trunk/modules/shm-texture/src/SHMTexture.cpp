#include <iostream>

#include "OpenGL.h"
#include "SHMTexture.h"

using namespace std;

SHMTexture::SHMTexture(int key, int w, int h, int f) : VideoTexture(w, h, f),
	shm(NULL)
{
	// FIXME: texture format RGB, grey

	// locate the segment
	int size = w * h;
	if (f == GL_RGB)
		size *= 3;

	if ((shmid = shmget(key, size, 0666)) < 0)
	{
		perror("shmget");
		throw Error();
	}

	// attach the segment
	if ((shm = (unsigned char *)shmat(shmid, NULL, 0)) == (unsigned char *)-1)
	{
		perror("shmat");
		throw Error();
	}

	update();
}

SHMTexture::~SHMTexture()
{
	// detach shm segment
    if (shm && (shmdt(shm) == -1))
    {
        perror("shm detach");
    }
}

void SHMTexture::update()
{
	upload(shm);
}

