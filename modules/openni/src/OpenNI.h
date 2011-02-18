#ifndef OPENNI_H
#define OPENNI_H

#include <iostream>

#include <XnCppWrapper.h>
#include <XnVPointControl.h>

#include "VideoTexture.h"

class Exc : public std::exception {};

class OpenNI
{
	public:
		OpenNI();
		void update();

		unsigned get_depth_texture_id() { return depth_txt->get_texture_id(); }
		float *get_tcoords() { return depth_txt->get_tcoords(); }

		class ExcOpenNI : public Exc {};

	protected:
		xn::Context m_Context;
		xn::DepthGenerator m_DepthGenerator;

#define MAX_DEPTH 10000
		float g_pDepthHist[MAX_DEPTH];

		VideoTexture *depth_txt;
		uint16_t *depth_pixels;

};

#endif

