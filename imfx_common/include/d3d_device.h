/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement.
This sample was distributed or derived from the Intel's Media Samples package.
The original version of this sample may be obtained from https://software.intel.com/en-us/intel-media-server-studio
or https://software.intel.com/en-us/media-client-solutions-support.
Copyright(c) 2011-2015 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#pragma once

#if defined( _WIN32 ) || defined ( _WIN64 )

#include "hw_device.h"

#pragma warning(disable : 4201)
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>
#include <windows.h>

#ifdef DRAW_LINE
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#define MAX_VERTEXES_OF_POLYGON 20	//max vertex of polygon, inlude line

#endif

#define VIDEO_MAIN_FORMAT D3DFMT_YUY2

class IGFXS3DControl;

/** Direct3D 9 device implementation.
@note Can be initilized for only 1 or two 2 views. Handle to
MFX_HANDLE_GFXS3DCONTROL must be set prior if initializing for 2 views.

@note Device always set D3DPRESENT_PARAMETERS::Windowed to TRUE.
*/
class CD3D9Device : public CHWDevice
{
public:
    CD3D9Device();
    virtual ~CD3D9Device();

    virtual mfxStatus Init(
        mfxHDL hWindow,
        mfxU16 nViews,
        mfxU32 nAdapterNum);
    virtual mfxStatus Reset();
    virtual mfxStatus GetHandle(mfxHandleType type, mfxHDL *pHdl);
	virtual mfxStatus GetDeviceHandle(mfxHDL *pHdl);
    virtual mfxStatus SetHandle(mfxHandleType type, mfxHDL hdl);
    virtual mfxStatus RenderFrame(mfxFrameSurface1 * pSurface, mfxFrameAllocator * pmfxAlloc);
    virtual void      UpdateTitle(double /*fps*/) { }
    virtual void      Close() ;
            void      DefineFormat(bool isA2rgb10) { m_bIsA2rgb10 = (isA2rgb10) ? TRUE : FALSE; }

#ifdef DRAW_LINE
	virtual mfxStatus DrawPolygon(POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum);
#endif
protected:
    mfxStatus CreateVideoProcessors();
    bool CheckOverlaySupport();
	int GetMonitorId(HWND hWnd);
    virtual mfxStatus FillD3DPP(mfxHDL hWindow, mfxU16 nViews, D3DPRESENT_PARAMETERS &D3DPP);
#ifdef DRAW_LINE
	VOID DrawCircle(mfx_f32 X, mfx_f32 Y, int radius, int numSides);
#endif
private:
    IDirect3D9Ex*               m_pD3D9;
    IDirect3DDevice9Ex*         m_pD3DD9;
    IDirect3DDeviceManager9*    m_pDeviceManager9;
    D3DPRESENT_PARAMETERS       m_D3DPP;
    UINT                        m_resetToken;

    mfxU16                      m_nViews;
    IGFXS3DControl*             m_pS3DControl;


    D3DSURFACE_DESC                 m_backBufferDesc;

    // service required to create video processors
    IDirectXVideoProcessorService*  m_pDXVAVPS;
    //left channel processor
    IDirectXVideoProcessor*         m_pDXVAVP_Left;
    // right channel processor
    IDirectXVideoProcessor*         m_pDXVAVP_Right;

    // target rectangle
    RECT                            m_targetRect;

    // various structures for DXVA2 calls
    DXVA2_VideoDesc                 m_VideoDesc;
    DXVA2_VideoProcessBltParams     m_BltParams;
    DXVA2_VideoSample               m_Sample;

    BOOL                            m_bIsA2rgb10;

#ifdef DRAW_LINE
	LPD3DXLINE						m_pLine; //Direct3D线对象
	D3DXVECTOR2						*m_pLineArr; //线段顶点

	POLYGON_ATTR_S                 *m_pstPolygonAttr;
	mfx_u32							m_ulPolygonNum;

#endif
};

#endif // #if defined( _WIN32 ) || defined ( _WIN64 )
