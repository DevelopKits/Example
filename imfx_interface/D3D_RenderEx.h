#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>

#include "imfx_defs.h"

class D3D_RenderEx
{
public:
	D3D_RenderEx();
	~D3D_RenderEx();
	int InitD3D(HWND hwnd);
	bool Render(RAW_DATA_S *raw_data);

private:

	int GetMonitorId(HWND hWnd, LPDIRECT3D9 pD3d);
	void Cleanup();

private:
	CRITICAL_SECTION  m_critial;
	HWND   m_hwnd;

	IDirect3D9 *m_pDirect3D9;
	IDirect3DDevice9 *m_pDirect3DDevice ;
	IDirect3DSurface9 *m_pDirect3DSurfaceRender;
	RECT m_rtViewport;
};

