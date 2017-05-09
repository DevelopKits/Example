#include "D3D_RenderEx.h"
#include <math.h>
#pragma comment (lib, "d3d9.lib")  


void Change_Endian_24(unsigned char* data)
{
	char tmp = data[2];
	data[2] = data[0];
	data[0] = tmp;
}

void Change_Endian_32(unsigned char* data)
{
	char tmp0, tmp1;
	tmp0 = data[3];
	tmp1 = data[2];
	data[3] = data[0];
	data[2] = data[1];
	data[0] = tmp0;
	data[1] = tmp1;
}

void Change_Endian_Pic(unsigned char*image, int w, int h, int bpp)
{
	unsigned char* pixeldata = NULL;
	for (int i = 0; i < h;i++)
	{
		for (int j = 0; j < w;j++)
		{
			pixeldata = image + (i*w + j)*bpp / 8;
			if (bpp == 32)
			{
				Change_Endian_32(pixeldata);
			} 
			else if (bpp == 24)
			{
				Change_Endian_24(pixeldata);
			}
		}
	}

}

void RGBToRGBA(unsigned char*RGBImage, const int width, const int height, int &step, unsigned char*RGBAImage)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			RGBAImage[i*width + j * 4 + 0] = RGBAImage[i*step + j * 3 + 0];
			RGBAImage[i*width + j * 4 + 1] = RGBAImage[i*step + j * 3 + 1];
			RGBAImage[i*width + j * 4 + 2] = RGBAImage[i*step + j * 3 + 2];
			RGBAImage[i*width + j * 4 + 3] = 0xff;
		}
	}
}

int D3D_RenderEx::GetMonitorId(HWND hWnd, LPDIRECT3D9 pD3d)
{
	HMONITOR hMainHmonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	UINT nMonitorCount = pD3d->GetAdapterCount();
	UINT nMonitorId = 0;
	for (; nMonitorId < nMonitorCount; nMonitorId++)
	{
		if (pD3d->GetAdapterMonitor(nMonitorId) == hMainHmonitor)
		{
			break;
		}
	}
	if (nMonitorId < nMonitorCount)
	{
		return nMonitorId;
	}
	return nMonitorCount;
}

void D3D_RenderEx::Cleanup()
{
	EnterCriticalSection(&m_critial);
	if (m_pDirect3DSurfaceRender)
		m_pDirect3DSurfaceRender->Release();
	if (m_pDirect3DDevice)
		m_pDirect3DDevice->Release();
	if (m_pDirect3D9)
		m_pDirect3D9->Release();
	LeaveCriticalSection(&m_critial);
}


int D3D_RenderEx::InitD3D(HWND hwnd)
{
	if (hwnd == NULL)
	{
		return -1;
	}
	m_hwnd = hwnd;
	HRESULT lRet;
	InitializeCriticalSection(&m_critial);
	Cleanup();

	m_pDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pDirect3D9 == NULL)
		return -1;

	/*m_pDirect3D9->EnumAdapterModes*/

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	//int x = GetSystemMetrics(SM_CXSCREEN);
	//int y = GetSystemMetrics(SM_CYSCREEN);
	//d3dpp.BackBufferWidth = min(r.right - r.left, x);
	//d3dpp.BackBufferHeight = min(r.bottom - r.top, y);

	GetClientRect(m_hwnd, &m_rtViewport);

	lRet = m_pDirect3D9->CreateDevice(GetMonitorId(hwnd, m_pDirect3D9), D3DDEVTYPE_HAL, hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &m_pDirect3DDevice);
	if (FAILED(lRet))
		return -1;


	return 0;
}

bool D3D_RenderEx::Render(RAW_DATA_S *raw_data)
{
	if (m_pDirect3DSurfaceRender==NULL)
	{
		HRESULT lRet = m_pDirect3DDevice->CreateOffscreenPlainSurface(raw_data->ulWidth, raw_data->ulHeight,
			D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pDirect3DSurfaceRender, NULL);
		if (FAILED(lRet))
			return false;
	}

	
	HRESULT lRet;
	D3DLOCKED_RECT d3d_rect;

	lRet = m_pDirect3DSurfaceRender->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
	if (FAILED(lRet))
		return false;

	byte *pSrc = (byte*)raw_data->pcDataAddr;
	byte * pDest = (BYTE *)d3d_rect.pBits;
	int stride = d3d_rect.Pitch;


	int pixel_w_size = raw_data->ulWidth * 4;
	for (int i = 0; i < raw_data->ulHeight; i++)
	{
		memcpy(pDest, pSrc, pixel_w_size);
		pDest += stride;
		pSrc += pixel_w_size;
	}

	//int pixel_w_size = raw_data->ulWidth * 3;
	//for (int i = 0; i < raw_data->ulHeight; i++)
	//{
	//	for (int j = 0; j < raw_data->ulWidth;j++)
	//	{
	//		memcpy(pDest + (i *raw_data->ulWidth + j) * 3 + 0 , pSrc + (i *raw_data->ulWidth + j) * 3 + 0, 1);
	//		memcpy(pDest + (i *raw_data->ulWidth + j) * 3 + 1 , pSrc + (i *raw_data->ulWidth + j) * 3 + 1, 1);
	//		memcpy(pDest + (i *raw_data->ulWidth + j) * 3 + 2 , pSrc + (i *raw_data->ulWidth + j) * 3 + 2, 1);
	//		//memcpy(pDest + (i *raw_data->ulWidth + j) * 3 + 3 , pSrc + (i *raw_data->ulWidth + j) * 3 + 0, 1);
	//	}
	//
	//	pDest += stride;
	//	pSrc += pixel_w_size;
	//}

	lRet = m_pDirect3DSurfaceRender->UnlockRect();
	if (FAILED(lRet))
		return false;

	m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_pDirect3DDevice->BeginScene();
	IDirect3DSurface9 * pBackBuffer = NULL;

	m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	m_pDirect3DDevice->StretchRect(m_pDirect3DSurfaceRender, NULL, pBackBuffer,NULL, D3DTEXF_LINEAR);
	m_pDirect3DDevice->EndScene();
	m_pDirect3DDevice->Present(NULL, NULL, NULL, NULL);
	pBackBuffer->Release();

	return true;
}

D3D_RenderEx::D3D_RenderEx()
{
	m_pDirect3D9 = NULL;
	m_pDirect3DDevice = NULL;
	m_pDirect3DSurfaceRender = NULL;
}


D3D_RenderEx::~D3D_RenderEx()
{
	delete m_pDirect3D9;
	delete m_pDirect3DDevice;
	delete m_pDirect3DSurfaceRender;
	m_pDirect3D9 = NULL;
	m_pDirect3DDevice = NULL;
	m_pDirect3DSurfaceRender = NULL;
}
