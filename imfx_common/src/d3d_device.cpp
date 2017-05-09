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

#include "mfx_samples_config.h"

#if defined(WIN32) || defined(WIN64)

//prefast singnature used in combaseapi.h
#ifndef _PREFAST_
    #pragma warning(disable:4068)
#endif

#include "d3d_device.h"
#include "d3d_allocator.h"
#include "sample_defs.h"
#include "igfx_s3dcontrol.h"

#include "atlbase.h"

CD3D9Device::CD3D9Device()
{
    m_pD3D9 = NULL;
    m_pD3DD9 = NULL;
    m_pDeviceManager9 = NULL;
    MSDK_ZERO_MEMORY(m_D3DPP);
    m_resetToken = 0;

    m_nViews = 0;
    m_pS3DControl = NULL;

    MSDK_ZERO_MEMORY(m_backBufferDesc);
    m_pDXVAVPS = NULL;
    m_pDXVAVP_Left = NULL;
    m_pDXVAVP_Right = NULL;

    MSDK_ZERO_MEMORY(m_targetRect);

    MSDK_ZERO_MEMORY(m_VideoDesc);
    MSDK_ZERO_MEMORY(m_BltParams);
    MSDK_ZERO_MEMORY(m_Sample);

    // Initialize DXVA structures

    DXVA2_AYUVSample16 color = {
        0x8000,          // Cr
        0x8000,          // Cb
        0x1000,          // Y
        0xffff           // Alpha
    };

    DXVA2_ExtendedFormat format =   {           // DestFormat
        DXVA2_SampleProgressiveFrame,           // SampleFormat
        DXVA2_VideoChromaSubsampling_MPEG2,     // VideoChromaSubsampling
        DXVA_NominalRange_0_255,                // NominalRange
        DXVA2_VideoTransferMatrix_BT709,        // VideoTransferMatrix
        DXVA2_VideoLighting_bright,             // VideoLighting
        DXVA2_VideoPrimaries_BT709,             // VideoPrimaries
        DXVA2_VideoTransFunc_709                // VideoTransferFunction
    };

    // init m_VideoDesc structure
    MSDK_MEMCPY_VAR(m_VideoDesc.SampleFormat, &format, sizeof(DXVA2_ExtendedFormat));
    m_VideoDesc.SampleWidth                         = 0;
    m_VideoDesc.SampleHeight                        = 0;
    m_VideoDesc.InputSampleFreq.Numerator           = 60;
    m_VideoDesc.InputSampleFreq.Denominator         = 1;
    m_VideoDesc.OutputFrameFreq.Numerator           = 60;
    m_VideoDesc.OutputFrameFreq.Denominator         = 1;

    // init m_BltParams structure
    MSDK_MEMCPY_VAR(m_BltParams.DestFormat, &format, sizeof(DXVA2_ExtendedFormat));
    MSDK_MEMCPY_VAR(m_BltParams.BackgroundColor, &color, sizeof(DXVA2_AYUVSample16));

    // init m_Sample structure
    m_Sample.Start = 0;
    m_Sample.End = 1;
    m_Sample.SampleFormat = format;
    m_Sample.PlanarAlpha.Fraction = 0;
    m_Sample.PlanarAlpha.Value = 1;

    m_bIsA2rgb10 = FALSE;
#ifdef DRAW_LINE
	m_pLineArr = NULL;
	MSDK_ZERO_MEMORY(m_pLine);
	m_pstPolygonAttr = NULL;
	m_ulPolygonNum = 0;
#endif
}

bool CD3D9Device::CheckOverlaySupport()
{
    D3DCAPS9                d3d9caps;
    D3DOVERLAYCAPS          d3doverlaycaps = {0};
    IDirect3D9ExOverlayExtension *d3d9overlay = NULL;
    bool overlaySupported = false;

    memset(&d3d9caps, 0, sizeof(d3d9caps));
    HRESULT hr = m_pD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3d9caps);
    if (FAILED(hr) || !(d3d9caps.Caps & D3DCAPS_OVERLAY))
    {
        overlaySupported = false;
    }
    else
    {
        hr = m_pD3D9->QueryInterface(IID_PPV_ARGS(&d3d9overlay));
        if (FAILED(hr) || (d3d9overlay == NULL))
        {
            overlaySupported = false;
        }
        else
        {
            hr = d3d9overlay->CheckDeviceOverlayType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                m_D3DPP.BackBufferWidth,
                m_D3DPP.BackBufferHeight,
                m_D3DPP.BackBufferFormat, NULL,
                D3DDISPLAYROTATION_IDENTITY, &d3doverlaycaps);
            MSDK_SAFE_RELEASE(d3d9overlay);

            if (FAILED(hr))
            {
                overlaySupported = false;
            }
            else
            {
                overlaySupported = true;
            }
        }
    }

    return overlaySupported;
}

mfxStatus CD3D9Device::FillD3DPP(mfxHDL hWindow, mfxU16 nViews, D3DPRESENT_PARAMETERS &D3DPP)
{
    mfxStatus sts = MFX_ERR_NONE;

    D3DPP.Windowed = true;
    D3DPP.hDeviceWindow = (HWND)hWindow;

    D3DPP.Flags                      = D3DPRESENTFLAG_VIDEO;
    D3DPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	D3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // note that this setting leads to an implicit timeBeginPeriod call
    D3DPP.BackBufferCount            = 1;
    D3DPP.BackBufferFormat           = (m_bIsA2rgb10) ? D3DFMT_A2R10G10B10 : D3DFMT_X8R8G8B8;

    //if (hWindow)
    //{
    //    RECT r;
    //    GetClientRect((HWND)hWindow, &r);
    //    int x = GetSystemMetrics(SM_CXSCREEN);
    //    int y = GetSystemMetrics(SM_CYSCREEN);
    //    D3DPP.BackBufferWidth  = min(r.right - r.left, x);
    //    D3DPP.BackBufferHeight = min(r.bottom - r.top, y);
    //}
    //else
    {
        D3DPP.BackBufferWidth  = 2500; //GetSystemMetrics(SM_CYSCREEN);
        D3DPP.BackBufferHeight = 1600; //GetSystemMetrics(SM_CYSCREEN);
    }
    //
    // Mark the back buffer lockable if software DXVA2 could be used.
    // This is because software DXVA2 device requires a lockable render target
    // for the optimal performance.
    //
    {
        D3DPP.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    }

    bool isOverlaySupported = CheckOverlaySupport();
    if (2 == nViews && !isOverlaySupported)
        return MFX_ERR_UNSUPPORTED;

    bool needOverlay = (2 == nViews) ? true : false;

    D3DPP.SwapEffect = needOverlay ? D3DSWAPEFFECT_OVERLAY : D3DSWAPEFFECT_DISCARD;

    return sts;
}

int CD3D9Device::GetMonitorId(HWND hWnd)
{
	HMONITOR hMainHmonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	UINT nMonitorCount = m_pD3D9->GetAdapterCount();
	UINT nMonitorId = 0;
	for (; nMonitorId < nMonitorCount; nMonitorId++)
	{
		if (m_pD3D9->GetAdapterMonitor(nMonitorId) == hMainHmonitor)
		{
			break;
		}
	}

	return (nMonitorId < nMonitorCount) ? nMonitorId : nMonitorCount;
}


mfxStatus CD3D9Device::Init(
    mfxHDL hWindow,
    mfxU16 nViews,
    mfxU32 nAdapterNum)
{
    mfxStatus sts = MFX_ERR_NONE;

    if (2 < nViews)
        return MFX_ERR_UNSUPPORTED;

    m_nViews = nViews;

    HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D9);
    if (!m_pD3D9 || FAILED(hr))
        return MFX_ERR_DEVICE_FAILED;

    ZeroMemory(&m_D3DPP, sizeof(m_D3DPP));
    sts = FillD3DPP(hWindow, nViews, m_D3DPP);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	GetMonitorId((HWND)hWindow);
    hr = m_pD3D9->CreateDeviceEx(
		/*nAdapterNum*//*GetMonitorId((HWND)hWindow)*/ 0,
        D3DDEVTYPE_HAL,
        (HWND)hWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
        &m_D3DPP,
        NULL,
        &m_pD3DD9);
    if (FAILED(hr))
        return MFX_ERR_NULL_PTR;

    if(hWindow)
    {
        hr = m_pD3DD9->ResetEx(&m_D3DPP, NULL);
        if (FAILED(hr))
            return MFX_ERR_UNDEFINED_BEHAVIOR;
        hr = m_pD3DD9->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
        if (FAILED(hr))
            return MFX_ERR_UNDEFINED_BEHAVIOR;
    }
    UINT resetToken = 0;

    hr = DXVA2CreateDirect3DDeviceManager9(&resetToken, &m_pDeviceManager9);
    if (FAILED(hr))
        return MFX_ERR_NULL_PTR;

    hr = m_pDeviceManager9->ResetDevice(m_pD3DD9, resetToken);
    if (FAILED(hr))
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    m_resetToken = resetToken;

#ifdef DRAW_LINE
	/* line */
	// 创建Direct3D线对象
	if (FAILED(D3DXCreateLine(m_pD3DD9, &m_pLine)))
	{
		return MFX_ERR_UNDEFINED_BEHAVIOR;
	}

	//m_pLineArr = new D3DXVECTOR2[MAX_VERTEXES_OF_POLYGON+1];
#endif
    return sts;
}

mfxStatus CD3D9Device::Reset()
{
    HRESULT hr = NO_ERROR;
    MSDK_CHECK_POINTER(m_pD3DD9, MFX_ERR_NULL_PTR);

    if (m_D3DPP.Windowed)
    {
        RECT r;
        GetClientRect((HWND)m_D3DPP.hDeviceWindow, &r);
        int x = GetSystemMetrics(SM_CXSCREEN);
        int y = GetSystemMetrics(SM_CYSCREEN);
        m_D3DPP.BackBufferWidth  = min(r.right - r.left, x);
        m_D3DPP.BackBufferHeight = min(r.bottom - r.top, y);
    }
    else
    {
        m_D3DPP.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
        m_D3DPP.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
    }

    // Reset will change the parameters, so use a copy instead.
    D3DPRESENT_PARAMETERS d3dpp = m_D3DPP;
    hr = m_pD3DD9->ResetEx(&d3dpp, NULL);

    if (FAILED(hr))
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    hr = m_pDeviceManager9->ResetDevice(m_pD3DD9, m_resetToken);
    if (FAILED(hr))
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    return MFX_ERR_NONE;
}

void CD3D9Device::Close()
{
    MSDK_SAFE_RELEASE(m_pDXVAVP_Left);
    MSDK_SAFE_RELEASE(m_pDXVAVP_Right);
    MSDK_SAFE_RELEASE(m_pDXVAVPS);

    MSDK_SAFE_RELEASE(m_pDeviceManager9);
    MSDK_SAFE_RELEASE(m_pD3DD9);
    MSDK_SAFE_RELEASE(m_pD3D9);
    m_pS3DControl = NULL;
}

CD3D9Device::~CD3D9Device()
{
    Close();

#ifdef DRAW_LINE
	if (m_pLine != NULL)  
	{  
		m_pLine->Release();
		m_pLine = NULL;
	}
	MSDK_SAFE_DELETE_ARRAY(m_pLineArr);

	if (NULL != m_pstPolygonAttr)
	{
		for (mfx_u32 i=0; i<m_ulPolygonNum; i++)
		{
			delete []m_pstPolygonAttr[i].pstFPoint;
		}
		delete []m_pstPolygonAttr;
	}
#endif
}

mfxStatus CD3D9Device::GetHandle(mfxHandleType type, mfxHDL *pHdl)
{
    if (MFX_HANDLE_DIRECT3D_DEVICE_MANAGER9 == type && pHdl != NULL)
    {
        *pHdl = m_pDeviceManager9;

        return MFX_ERR_NONE;
    }
    else if (MFX_HANDLE_GFXS3DCONTROL == type && pHdl != NULL)
    {
        *pHdl = m_pS3DControl;

        return MFX_ERR_NONE;
    }
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CD3D9Device::SetHandle(mfxHandleType type, mfxHDL hdl)
{
    if (MFX_HANDLE_GFXS3DCONTROL == type && hdl != NULL)
    {
        m_pS3DControl = (IGFXS3DControl*)hdl;
        return MFX_ERR_NONE;
    }
    else if (MFX_HANDLE_DEVICEWINDOW == type && hdl != NULL) //for render window handle
    {
        m_D3DPP.hDeviceWindow = (HWND)hdl;
        return MFX_ERR_NONE;
    }
    return MFX_ERR_UNSUPPORTED;
}

#ifdef DRAW_LINE
VOID CD3D9Device::DrawCircle(mfx_f32 X, mfx_f32 Y, int radius, int numSides)
{
#define PI 3.14159265358979f

	D3DXVECTOR2 Line[128];

	for (int i=0; i<=numSides; i++)
	{
		Line[i].x = X + radius * std::cos(PI * i *2 / numSides);
		Line[i].y = Y + radius * std::sin(PI * i *2 / numSides);
	}
	Line[numSides + 1].x = Line[0].x;
	Line[numSides + 1].y = Line[0].y;

	//m_pLine->Draw(Line, numSides+1, m_ulLineColor);
}
#endif

mfxStatus CD3D9Device::RenderFrame(mfxFrameSurface1 * pSurface, mfxFrameAllocator * pmfxAlloc)
{
    HRESULT hr = S_OK;

    if (!(1 == m_nViews || (2 == m_nViews && NULL != m_pS3DControl)))
        return MFX_ERR_UNDEFINED_BEHAVIOR;

    MSDK_CHECK_POINTER(pSurface, MFX_ERR_NULL_PTR);
    MSDK_CHECK_POINTER(m_pDeviceManager9, MFX_ERR_NOT_INITIALIZED);
    MSDK_CHECK_POINTER(pmfxAlloc, MFX_ERR_NULL_PTR);

    // don't try to render second view if output rect changed since first view
    if (2 == m_nViews && (0 != pSurface->Info.FrameId.ViewId))
        return MFX_ERR_NONE;

    hr = m_pD3DD9->TestCooperativeLevel();

    switch (hr)
    {
        case D3D_OK :
            break;

        case D3DERR_DEVICELOST :
        {
            return MFX_ERR_DEVICE_LOST;
        }

        case D3DERR_DEVICENOTRESET :
            {
            return MFX_ERR_UNKNOWN;
        }

        default :
        {
            return MFX_ERR_UNKNOWN;
        }
    }
/*
	RECT source = { pSurface->Info.CropX, pSurface->Info.CropY,
		pSurface->Info.CropX + pSurface->Info.CropW,
		pSurface->Info.CropY + pSurface->Info.CropH };

	RECT targetRect = {0}, dest = {0};
	GetClientRect(m_D3DPP.hDeviceWindow, &targetRect);
	dest = targetRect;
*/
	m_pD3DD9->BeginScene();
    CComPtr<IDirect3DSurface9> pBackBuffer;
    hr = m_pD3DD9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

    mfxHDLPair* dxMemId = (mfxHDLPair*)pSurface->Data.MemId;

    hr = m_pD3DD9->StretchRect((IDirect3DSurface9*)dxMemId->first, NULL, pBackBuffer, NULL, D3DTEXF_LINEAR);
    if (FAILED(hr))
    {
        return MFX_ERR_UNKNOWN;
    }
	m_pD3DD9->EndScene();
#if 0
#ifdef DRAW_LINE
	if (m_ulPolygonNum > 0)
	{
		if( SUCCEEDED(m_pD3DD9->BeginScene()))
		{
			for (mfx_u32 i=0; i<m_ulPolygonNum; i++)
			{
				m_pLine->SetWidth(m_pstPolygonAttr[i].fLineWidth);
				m_pLine->SetAntialias(TRUE);

				MSDK_SAFE_DELETE_ARRAY(m_pLineArr);

				m_pLineArr = new D3DXVECTOR2[m_pstPolygonAttr[i].ulVertexNum+1];

				for (mfx_u32 j=0; j<m_pstPolygonAttr[i].ulVertexNum; j++)
				{
					m_pLineArr[j].x = m_pstPolygonAttr[i].pstFPoint[j].x;
					m_pLineArr[j].y = m_pstPolygonAttr[i].pstFPoint[j].y;
				}

				m_pLineArr[m_pstPolygonAttr[i].ulVertexNum].x = m_pstPolygonAttr[i].pstFPoint[0].x;
				m_pLineArr[m_pstPolygonAttr[i].ulVertexNum].y = m_pstPolygonAttr[i].pstFPoint[0].y;

				m_pLine->Draw(m_pLineArr, m_pstPolygonAttr[i].ulVertexNum+1, m_pstPolygonAttr[i].ulLineColor);
			}

			//for (int i=0; i<m_ulVertexNum; i++)
			//{
			//	DrawCircle(m_pLineArr[i].x, m_pLineArr[i].y, 10, 10);
			//}

			m_pD3DD9->EndScene();
		}
		m_ulPolygonNum = 0;
	}
#endif
#endif
	//msdk_tick m_tStartTime = msdk_time_get_tick();
    if (SUCCEEDED(hr)&& (1 == m_nViews || pSurface->Info.FrameId.ViewId == 1))
    {
        hr = m_pD3DD9->Present(NULL, NULL, NULL, NULL);
    }
	//msdk_tick m_tEndTime = msdk_time_get_tick();

	//printf("Present: %6.3f ms\n", (m_tEndTime - m_tStartTime) * 1000 / (double)msdk_time_get_frequency());

    return SUCCEEDED(hr) ? MFX_ERR_NONE : MFX_ERR_DEVICE_FAILED;
}

mfxStatus CD3D9Device::CreateVideoProcessors()
{
    if (!(1 == m_nViews || (2 == m_nViews && NULL != m_pS3DControl)))
        return MFX_ERR_UNDEFINED_BEHAVIOR;

   MSDK_SAFE_RELEASE(m_pDXVAVP_Left);
   MSDK_SAFE_RELEASE(m_pDXVAVP_Right);

   HRESULT hr ;

   if (2 == m_nViews && NULL != m_pS3DControl)
   {
       hr = m_pS3DControl->SetDevice(m_pDeviceManager9);
       if (FAILED(hr))
       {
           return MFX_ERR_DEVICE_FAILED;
       }
   }

   ZeroMemory(&m_backBufferDesc, sizeof(m_backBufferDesc));
   IDirect3DSurface9 *backBufferTmp = NULL;
   hr = m_pD3DD9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBufferTmp);
   if (NULL != backBufferTmp)
       backBufferTmp->GetDesc(&m_backBufferDesc);
   MSDK_SAFE_RELEASE(backBufferTmp);

   if (SUCCEEDED(hr))
   {
       // Create DXVA2 Video Processor Service.
       hr = DXVA2CreateVideoService(m_pD3DD9,
           IID_IDirectXVideoProcessorService,
           (void**)&m_pDXVAVPS);
   }

   if (2 == m_nViews)
   {
        // Activate L channel
        if (SUCCEEDED(hr))
        {
           hr = m_pS3DControl->SelectLeftView();
        }

        if (SUCCEEDED(hr))
        {
           // Create VPP device for the L channel
           hr = m_pDXVAVPS->CreateVideoProcessor(DXVA2_VideoProcProgressiveDevice,
               &m_VideoDesc,
               m_D3DPP.BackBufferFormat,
               1,
               &m_pDXVAVP_Left);
        }

        // Activate R channel
        if (SUCCEEDED(hr))
        {
           hr = m_pS3DControl->SelectRightView();
        }

   }
   if (SUCCEEDED(hr))
   {
       hr = m_pDXVAVPS->CreateVideoProcessor(DXVA2_VideoProcProgressiveDevice,
           &m_VideoDesc,
           m_D3DPP.BackBufferFormat,
           1,
           &m_pDXVAVP_Right);
   }

   return SUCCEEDED(hr) ? MFX_ERR_NONE : MFX_ERR_DEVICE_FAILED;
}

mfxStatus CD3D9Device::GetDeviceHandle(mfxHDL *pHdl)
{
	*pHdl = m_pD3DD9;
	return MFX_ERR_NONE;
}

#ifdef DRAW_LINE
mfxStatus CD3D9Device::DrawPolygon(POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum)
{
	if ((0 == ulPolygonNum) && (ulPolygonNum >= 100))
	{
		return MFX_ERR_UNSUPPORTED;
	}
	MSDK_CHECK_POINTER(pstPolygonAttr, MFX_ERR_NULL_PTR);

	if (NULL != m_pstPolygonAttr)
	{
		for (mfx_u32 i=0; i<m_ulPolygonNum; i++)
		{
			delete []m_pstPolygonAttr[i].pstFPoint;
		}
		delete []m_pstPolygonAttr;
	}

	m_pstPolygonAttr = new POLYGON_ATTR_S[ulPolygonNum];

	for (mfx_u32 i=0; i<ulPolygonNum; i++)
	{
		m_pstPolygonAttr[i].pstFPoint = new FPOINT_S[pstPolygonAttr[i].ulVertexNum];
		for (mfx_u32 j=0; j<pstPolygonAttr[i].ulVertexNum; j++)
		{
			m_pstPolygonAttr[i].pstFPoint[j].x = pstPolygonAttr[i].pstFPoint[j].x;
			m_pstPolygonAttr[i].pstFPoint[j].y = pstPolygonAttr[i].pstFPoint[j].y;
		}
		m_pstPolygonAttr[i].fLineWidth = pstPolygonAttr[i].fLineWidth;
		m_pstPolygonAttr[i].ulLineColor = pstPolygonAttr[i].ulLineColor;
		m_pstPolygonAttr[i].ulVertexNum = pstPolygonAttr[i].ulVertexNum;
	}

	m_ulPolygonNum = ulPolygonNum;
#if 0
	m_pLineArr = new (D3DXVECTOR2 *)[ulPolygonNum];

	for (int i=0; i<ulPolygonNum; i++)
	{
		m_pLineArr[i] = new D3DXVECTOR2[pstPolygonAttr[i]->ulVertexNum];
		for (int j = 0; j < pstPolygonAttr[i]->ulVertexNum; j++)
		{
			m_pLineArr[i]->x = pstPolygonAttr[i]->pstFPoint[i].x;
			m_pLineArr[i]->y = pstPolygonAttr[i]->pstFPoint[i].y;
		}
	}
	
	if (ulVertexNum == 0)	/* clear line */
	{
		m_ulVertexNum = 0;
	}
	else	/* draw polygon, include point and line*/
	{
		/* last point is equal to point[0] */
		m_pLineArr[ulVertexNum].x = pstPoint[0].x;
		m_pLineArr[ulVertexNum].y = pstPoint[0].y;
		m_ulVertexNum = ulVertexNum + 1;
	}
#endif
	return MFX_ERR_NONE;
}
#endif

#endif // #if defined(WIN32) || defined(WIN64)