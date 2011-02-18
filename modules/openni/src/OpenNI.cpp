#include <iostream>

#include "OpenNI.h"

using namespace std;

#define CHECK_RC(rc, what)											 \
	if (rc != XN_STATUS_OK)											 \
	{																 \
		cerr << what << " failed " << xnGetStatusString(rc) << endl; \
		throw ExcOpenNI(); \
	}

#define CHECK_ERRORS(rc, errors, what)		\
	if (rc == XN_STATUS_NO_NODE_PRESENT)	\
	{										\
		XnChar strError[1024];				\
		errors.ToString(strError, 1024);	\
		cerr << strError << endl;			\
		throw ExcOpenNI(); \
	}

#define SAMPLE_XML_PATH "data/Sample-Tracking.xml"

OpenNI::OpenNI()
{
	XnStatus rc = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	// Initialize OpenNI
	rc = m_Context.InitFromXmlFile(SAMPLE_XML_PATH, &errors);
	CHECK_ERRORS(rc, errors, "InitFromXmlFile");
	CHECK_RC(rc, "InitFromXmlFile");

	rc = m_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, m_DepthGenerator);
	CHECK_RC(rc, "Find depth generator");

	//g_pDrawer = new XnVPointDrawer(20, g_DepthGenerator);
	//g_pDrawer->SetDepthMap(g_bDrawDepthMap);

	// Initialization done. Start generating
	rc = m_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	xn::DepthMetaData depthMD;
	m_DepthGenerator.GetMetaData(depthMD);
	depth_txt = new VideoTexture(depthMD.XRes(), depthMD.YRes(), GL_LUMINANCE, GL_UNSIGNED_SHORT);
	depth_pixels = new uint16_t[depthMD.XRes() * depthMD.YRes()];
}

void OpenNI::update()
{
	xn::DepthMetaData dm;
	m_DepthGenerator.GetMetaData(dm);

	// TODO: histogram
	//uint16_t* depth = const_cast<uint16_t *>(reinterpret_cast<const uint16_t *>(dm.Data()));

	unsigned int nValue = 0;
	uint16_t nHistValue = 0;
	unsigned int nIndex = 0;
	unsigned int nX = 0;
	unsigned int nY = 0;
	unsigned int nNumberOfPoints = 0;
	XnUInt16 g_nXRes = dm.XRes();
	XnUInt16 g_nYRes = dm.YRes();
	uint16_t *pDestImage = depth_pixels;

	const XnUInt16* pDepth = dm.Data();

	// Calculate the accumulative histogram
	memset(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
	for (nY=0; nY<g_nYRes; nY++)
	{
		for (nX=0; nX<g_nXRes; nX++)
		{
			nValue = *pDepth;

			if (nValue != 0)
			{
				g_pDepthHist[nValue]++;
				nNumberOfPoints++;
			}

			pDepth++;
		}
	}
	for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (65535 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	pDepth = dm.Data();
	{
		XnUInt32 nIndex = 0;
		// Prepare the texture map
		for (nY=0; nY<g_nYRes; nY++)
		{
			for (nX=0; nX < g_nXRes; nX++, nIndex++)
			{
				nValue = *pDepth;

				if (nValue != 0)
				{
					nHistValue = g_pDepthHist[nValue];

					*pDestImage = nHistValue;
				}
				else
				{
					*pDestImage = 0;
				}

				pDepth++;
				pDestImage++;
			}
		}
	}

	depth_txt->upload(depth_pixels);
}
