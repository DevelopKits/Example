#ifndef __D3D_RENDER_H__
#define __D3D_RENDER_H__

#if defined(WIN32) || defined(WIN64)

#pragma warning(disable : 4201)
#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>
#include <windows.h>
#include "igfx_s3dcontrol.h"
#endif

#include "mfxstructures.h"
#include "mfxvideo.h"

#include "hw_device.h"

#include "imfx_defs.h"

typedef void* WindowHandle;
typedef void* Handle;

#if defined(WIN32) || defined(WIN64)

struct sWindowParams
{
	LPCTSTR lpClassName;
	LPCTSTR lpWindowName;
	DWORD dwStyle;
	int nx;
	int ny;
	int ncell;
	int nAdapter;
	int nMaxFPS;
	int nWidth;
	int nHeight;
	HWND hWndParent;
	HMENU hMenu;
	HINSTANCE hInstance;
	LPVOID lpParam;
	bool bFullScreen; // Stretch window to full screen
};

class IMFX_API CD3DRender
{
public:
	CD3DRender();
	~CD3DRender();

	mfxStatus Init(HWND hWnd);
	mfxStatus RenderFrame(mfxFrameSurface1 *pSurface, mfxFrameAllocator *pmfxAlloc);
	VOID UpdateTitle(double fps);

	HWND GetWindowHandle() { return m_Hwnd; }

	VOID OnDestroy(HWND hwnd);
	VOID OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
	VOID ChangeWindowSize(bool bFullScreen);

	void SetHWDevice(CHWDevice *dev);

protected:
	void AdjustWindowRect(RECT *rect);

	CHWDevice *m_hwdev;

	sWindowParams       m_sWindowParams;
	HWND                m_Hwnd;
	RECT                m_rect;
	DWORD               m_style;

	bool EnableDwmQueuing();
	static BOOL CALLBACK MonitorEnumProc(HMONITOR ,HDC ,LPRECT lprcMonitor,LPARAM dwData);

	bool                 m_bIsDwmQueueSupported;
	bool                 m_bDwmEnabled;
	int                  m_nMonitorCurrent;
	::RECT               m_RectWindow;
	bool				 m_blIsInited;
};
#endif // #if defined(WIN32) || defined(WIN64)

#endif // __DECODE_D3D_RENDER_H__