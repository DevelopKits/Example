/*********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2005-2014 Intel Corporation. All Rights Reserved.

**********************************************************************************/

#include "mfx_samples_config.h"

#if defined(WIN32) || defined(WIN64)

#include <windowsx.h>
#include <dwmapi.h>
#include <mmsystem.h>

#include "sample_defs.h"
#include "d3d_render.h"

#pragma warning(disable : 4100)

#if 0
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef WIN64
	CD3DRender* pRender = (CD3DRender*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	CD3DRender* pRender = (CD3DRender*)LongToPtr(GetWindowLongPtr(hWnd, GWL_USERDATA));
#endif
	if (pRender)
	{
		switch(message)
		{
			HANDLE_MSG(hWnd, WM_DESTROY, pRender->OnDestroy);
			HANDLE_MSG(hWnd, WM_KEYUP,   pRender->OnKey);
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
#endif

CD3DRender::CD3DRender()
{
	m_bDwmEnabled = false;
	m_bIsDwmQueueSupported = true;
	m_nMonitorCurrent = 0;

	m_hwdev = NULL;
	MSDK_ZERO_MEMORY(m_sWindowParams);
	m_Hwnd = 0;
	MSDK_ZERO_MEMORY(m_rect);
	m_style = 0;
	m_blIsInited = false;
}
#if 0
BOOL CALLBACK CD3DRender::MonitorEnumProc(HMONITOR /*hMonitor*/,
										  HDC /*hdcMonitor*/,
										  LPRECT lprcMonitor,
										  LPARAM dwData)
{
	CD3DRender * pRender = reinterpret_cast<CD3DRender *>(dwData);
	RECT r = {0};
	if (NULL == lprcMonitor)
		lprcMonitor = &r;

	if (pRender->m_nMonitorCurrent++ == pRender->m_sWindowParams.nAdapter)
	{
		pRender->m_RectWindow = *lprcMonitor;
	}
	return TRUE;
}
#endif

CD3DRender::~CD3DRender()
{
	//if (m_Hwnd)
	//    DestroyWindow(m_Hwnd); /* destoryed by creater */

	//DestroyTimer();
}

mfxStatus CD3DRender::Init(HWND hWnd)
{
	if (true == m_blIsInited)
	{
		IMFX_ERR("D3D render is already initilized!\n");
		return MFX_ERR_UNKNOWN;
	}

	m_Hwnd = hWnd;
	if (NULL == m_Hwnd)
		return MFX_ERR_UNKNOWN;

	m_sWindowParams.lpWindowName = MSDK_STRING("IMFX_display");
	m_sWindowParams.nx = 0;
	m_sWindowParams.ny = 0;
	m_sWindowParams.nWidth = CW_USEDEFAULT;
	m_sWindowParams.nHeight = CW_USEDEFAULT;
	m_sWindowParams.ncell = 0;
	m_sWindowParams.nAdapter = 0;
	m_sWindowParams.nMaxFPS = 10000;

	m_sWindowParams.lpClassName = MSDK_STRING("Render Window Class");
	m_sWindowParams.dwStyle = WS_OVERLAPPEDWINDOW;
	m_sWindowParams.hWndParent = NULL;
	m_sWindowParams.hMenu = NULL;
	m_sWindowParams.hInstance = GetModuleHandle(NULL);
	m_sWindowParams.lpParam = NULL;
	m_sWindowParams.bFullScreen = FALSE;

#ifdef WIN64
	//SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
#else
	//SetWindowLong(hWnd, GWL_USERDATA, PtrToLong(this));
#endif

	m_blIsInited = true;

	return MFX_ERR_NONE;
}

mfxStatus CD3DRender::RenderFrame(mfxFrameSurface1 *pSurface, mfxFrameAllocator *pmfxAlloc)
{
	if (false == m_blIsInited)
	{
		IMFX_ERR("D3D render is uninitialized!\n");
		return MFX_ERR_UNKNOWN;
	}

	RECT rect;
	GetClientRect(m_Hwnd, &rect);
	if (IsRectEmpty(&rect))
		return MFX_ERR_UNKNOWN;

	pSurface->Info.FrameRateExtN = m_sWindowParams.nMaxFPS;
	pSurface->Info.FrameRateExtD = 1;

	//if (m_bIsDwmQueueSupported)
	//{
	//    EnableDwmQueuing();
	//}

	mfxStatus sts = m_hwdev->RenderFrame(pSurface, pmfxAlloc);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	return sts;
}

VOID CD3DRender::UpdateTitle(double fps)
{
	if (m_Hwnd)
	{
		MSG msg;
		MSDK_ZERO_MEMORY(msg);
		while (msg.message != WM_QUIT && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (NULL != m_sWindowParams.lpWindowName) {
			TCHAR str[20];
			_stprintf_s(str, 20, _T("fps=%.2lf"), fps);

			SetWindowText(m_Hwnd, str);
		}
	}
}

VOID CD3DRender::OnDestroy(HWND /*hwnd*/)
{
	PostQuitMessage(0);
}

VOID CD3DRender::OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (TRUE == fDown)
		return;

	if ('1' == vk && false == m_sWindowParams.bFullScreen)
		ChangeWindowSize(true);
	else if (true == m_sWindowParams.bFullScreen)
		ChangeWindowSize(false);
}

void CD3DRender::AdjustWindowRect(RECT *rect)
{
	int cxmax = GetSystemMetrics(SM_CXMAXIMIZED);
	int cymax = GetSystemMetrics(SM_CYMAXIMIZED);
	int cxmin = GetSystemMetrics(SM_CXMINTRACK);
	int cymin = GetSystemMetrics(SM_CYMINTRACK);
	int leftmax = cxmax - cxmin;
	int topmax = cymax - cxmin;
	if (rect->left < 0)
		rect->left = 0;
	if (rect->left > leftmax)
		rect->left = leftmax;
	if (rect->top < 0)
		rect->top = 0;
	if (rect->top > topmax)
		rect->top = topmax;

	if (rect->right < rect->left + cxmin)
		rect->right = rect->left + cxmin;
	if (rect->right - rect->left > cxmax)
		rect->right = rect->left + cxmax;

	if (rect->bottom < rect->top + cymin)
		rect->bottom = rect->top + cymin;
	if (rect->bottom - rect->top > cymax)
		rect->bottom = rect->top + cymax;
}

VOID CD3DRender::ChangeWindowSize(bool bFullScreen)
{
	HMONITOR hMonitor = MonitorFromWindow(m_Hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);

	WINDOWINFO wndInfo;
	wndInfo.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(m_Hwnd, &wndInfo);

	if (!m_sWindowParams.bFullScreen)
	{
		m_rect = wndInfo.rcWindow;
		m_style = wndInfo.dwStyle;
	}

	m_sWindowParams.bFullScreen = bFullScreen;

	if (!bFullScreen)
	{
		AdjustWindowRectEx(&m_rect, 0, 0, 0);
		SetWindowLong(m_Hwnd, GWL_STYLE, m_style);
		SetWindowPos(m_Hwnd, HWND_NOTOPMOST,
			m_rect.left, m_rect.top,
			abs(m_rect.right - m_rect.left), abs(m_rect.bottom - m_rect.top),
			SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowLong(m_Hwnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(m_Hwnd, HWND_NOTOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
			abs(mi.rcMonitor.left - mi.rcMonitor.right), abs(mi.rcMonitor.top - mi.rcMonitor.bottom), SWP_SHOWWINDOW);
	}
}

bool CD3DRender::EnableDwmQueuing()
{
	HRESULT hr;

	// DWM queuing is enabled already.
	if (m_bDwmEnabled)
	{
		return true;
	}

	// Check to see if DWM is currently enabled.
	BOOL bDWM = FALSE;

	hr = DwmIsCompositionEnabled(&bDWM);

	if (FAILED(hr))
	{
		_tprintf(_T("DwmIsCompositionEnabled failed with error 0x%x.\n"), hr);
		return false;
	}

	// DWM queuing is disabled when DWM is disabled.
	if (!bDWM)
	{
		m_bDwmEnabled = false;
		return false;
	}

	// Retrieve DWM refresh count of the last vsync.
	DWM_TIMING_INFO dwmti = { 0 };

	dwmti.cbSize = sizeof(dwmti);

	hr = DwmGetCompositionTimingInfo(NULL, &dwmti);

	if (FAILED(hr))
	{
		_tprintf(_T("DwmGetCompositionTimingInfo failed with error 0x%x.\n"), hr);
		return false;
	}

	// Enable DWM queuing from the next refresh.
	DWM_PRESENT_PARAMETERS dwmpp = { 0 };

	dwmpp.cbSize = sizeof(dwmpp);
	dwmpp.fQueue = TRUE;
	dwmpp.cRefreshStart = dwmti.cRefresh + 1;
	dwmpp.cBuffer = 8; //maximum depth of DWM queue
	dwmpp.fUseSourceRate = TRUE;
	dwmpp.cRefreshesPerFrame = 1;
	dwmpp.eSampling = DWM_SOURCE_FRAME_SAMPLING_POINT;
	dwmpp.rateSource.uiDenominator = 1;
	dwmpp.rateSource.uiNumerator = m_sWindowParams.nMaxFPS;

	hr = DwmSetPresentParameters(m_Hwnd, &dwmpp);

	if (FAILED(hr))
	{
		_tprintf(_T("DwmSetPresentParameters failed with error 0x%x.\n"), hr);
		return false;
	}

	// DWM queuing is enabled.
	m_bDwmEnabled = true;

	return true;
}

void CD3DRender::SetHWDevice(CHWDevice *dev)
{
	m_hwdev = dev;
}

#endif // #if defined(WIN32) || defined(WIN64)