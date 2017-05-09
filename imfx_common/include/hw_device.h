/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement.
This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
Copyright(c) 2013 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#pragma once

#include "mfxvideo++.h"
#include "imfx_defs.h"

#define DRAW_LINE

/// Base class for hw device
class CHWDevice
{
public:
    virtual ~CHWDevice(){}
    /** Initializes device for requested processing.
    @param[in] hWindow Window handle to bundle device to.
    @param[in] nViews Number of views to process.
    @param[in] nAdapterNum Number of adapter to use
    */
    virtual mfxStatus Init(
        mfxHDL hWindow,
#if (defined(WIN32) || defined(WIN64))
#else
		mfxHDL hDisplay,
#endif
        mfxU16 nViews,
        mfxU32 nAdapterNum) = 0;
    /// Reset device.
    virtual mfxStatus Reset() = 0;
    /// Get handle can be used for MFX session SetHandle calls
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL *pHdl) = 0;
	virtual mfxStatus GetDeviceHandle(mfxHDL *pHdl) = 0;
    /** Set handle.
    Particular device implementation may require other objects to operate.
    */
    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl) = 0;
    virtual mfxStatus RenderFrame(mfxFrameSurface1 * pSurface, mfxFrameAllocator * pmfxAlloc) = 0;
    virtual void      UpdateTitle(double fps) = 0;
    virtual void      Close() = 0;
#if ((defined(WIN32) || defined(WIN64)) && (defined(DRAW_LINE)))
	virtual mfxStatus DrawPolygon(POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum) = 0;
#endif
};
