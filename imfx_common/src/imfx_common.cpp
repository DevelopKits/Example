#include <math.h>

#include "imfx.h"
#include "imfx_common.h"
#include "d3d_device.h"
#include "d3d11_device.h"


CIMFXCommon::CIMFXCommon()
{
	m_pGeneralAllocator = NULL;
	m_pmfxAllocatorParams = NULL;
	m_enMemType = D3D9_MEMORY;
#if defined(WIN32) || defined(WIN64)
	m_hWindow = NULL;
#else
	m_hWindow = NULL;
	m_pDisplay = NULL;
#endif
	m_bIsRender = false;
	m_hwdev = NULL;
	m_InitIsCalled = false;
}

CIMFXCommon::~CIMFXCommon()
{
	CloseSession();
}

IMFX_STS CIMFXCommon::InitSession(mfxIMPL impl, mfxVersion ver, MEM_TYPE_E enMemType /*= D3D9_MEMORY*/, mfx_hdl hWindow /*= NULL*/, mfx_hdl hDisplay /*= NULL*/)
{
	mfxStatus sts = MFX_ERR_NONE;

	m_enMemType = enMemType;

	if (NULL != hWindow)
	{
#if defined(WIN32) || defined(WIN64)
		m_hWindow = hWindow;
#else
		m_hWindow = hWindow;
		m_pDisplay = hDisplay;
#endif
		m_bIsRender = true;
		/* if render is enabled, the memory type must reset to D3D9_MEMORY */
		m_enMemType = D3D9_MEMORY;
	}

	sts = Initialize(impl, ver);
	if (MFX_ERR_NONE != sts)
	{
		return IMFX_ERR_UNKNOWN;
	}

	m_InitIsCalled = true;

	return IMFX_ERR_NONE;
}


IMFX_STS CIMFXCommon::CloseSession()
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_NONE;
	}

	m_mfxSession.Close();
	DeleteAllocator();

	m_InitIsCalled = false;
	return IMFX_ERR_NONE;
}

#if 0
mfxStatus CIMFXCommon::Sync( mfxSyncPoint syncp )
{
	mfxStatus sts = MFX_ERR_NONE;
	if (syncp)
	{
		sts = m_mfxSession.SyncOperation(syncp, 60000);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
	else
	{
		return sts;
	}

	return MFX_ERR_NONE;
}
#endif

IMFX_STS CIMFXCommon::LockSurface(mfxFrameSurface1 *pmfxSurface)
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

#if 0
#else
	if (SYSTEM_MEMORY == m_enMemType)
	{
		return IMFX_ERR_NONE;
	}
	else
	{
		MSDK_CHECK_POINTER(pmfxSurface, IMFX_ERR_NULL_PTR);
		if (NULL == pmfxSurface->Data.MemId)
		{
			return IMFX_ERR_NONE;
		}
		else
		{
			return (IMFX_STS)m_pGeneralAllocator->Lock(m_pGeneralAllocator->pthis, pmfxSurface->Data.MemId, &(pmfxSurface->Data));
		}
	}
#endif
}

IMFX_STS CIMFXCommon::UnlockSurface(mfxFrameSurface1 *pmfxSurface)
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}
#if 0
#else

	MSDK_CHECK_POINTER(pmfxSurface, IMFX_ERR_NULL_PTR);
	if (NULL == pmfxSurface->Data.MemId)
	{
		return IMFX_ERR_NONE;
	}
	else
	{
		return (IMFX_STS)m_pGeneralAllocator->Unlock(m_pGeneralAllocator->pthis, pmfxSurface->Data.MemId, &(pmfxSurface->Data));
	}

#endif
}

//mfxStatus CIMFXCommon::Initialize(mfxIMPL impl, mfxVersion ver, bool bCreateSharedHandles)
mfxStatus CIMFXCommon::Initialize(mfxIMPL impl, mfxVersion ver)
{
	mfxStatus sts = MFX_ERR_NONE;

	if (D3D11_MEMORY == m_enMemType)
		impl |= MFX_IMPL_VIA_D3D11;

	// Initialize Intel Media SDK Session
	sts = m_mfxSession.Init(impl, &ver);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

#ifdef D3D_SURFACES_SUPPORT
	if (true == m_bIsRender)
	{
		sts = m_d3dRender.Init((HWND)m_hWindow);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}
#endif

	// If mfxFrameAllocator is provided it means we need to setup DirectX device and memory allocator
	if (NULL == m_pGeneralAllocator)
	{
		sts = CreateAllocator();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}

	return sts;
}

mfxStatus CIMFXCommon::CreateAllocator()
{
	mfxStatus sts = MFX_ERR_NONE;

	m_pGeneralAllocator = new GeneralAllocator();
	if (m_enMemType != SYSTEM_MEMORY)
	{
#if D3D_SURFACES_SUPPORT
		sts = CreateHWDevice();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		// provide device manager to MediaSDK
		mfxHDL hdl = NULL;
		mfxHandleType hdl_t =
#if MFX_D3D11_SUPPORT
			D3D11_MEMORY == m_enMemType ? MFX_HANDLE_D3D11_DEVICE :
#endif // #if MFX_D3D11_SUPPORT
			MFX_HANDLE_D3D9_DEVICE_MANAGER;

		sts = m_hwdev->GetHandle(hdl_t, &hdl);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = m_mfxSession.SetHandle(hdl_t, hdl);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		// create D3D allocator
#if MFX_D3D11_SUPPORT
		if (D3D11_MEMORY == m_enMemType)
		{
			D3D11AllocatorParams *pd3dAllocParams = new D3D11AllocatorParams;
			MSDK_CHECK_POINTER(pd3dAllocParams, MFX_ERR_MEMORY_ALLOC);
			pd3dAllocParams->pDevice = reinterpret_cast<ID3D11Device *>(hdl);

			m_pmfxAllocatorParams = pd3dAllocParams;
		}
		else
#endif // #if MFX_D3D11_SUPPORT
		{
			D3DAllocatorParams *pd3dAllocParams = new D3DAllocatorParams;
			MSDK_CHECK_POINTER(pd3dAllocParams, MFX_ERR_MEMORY_ALLOC);
			pd3dAllocParams->pManager = reinterpret_cast<IDirect3DDeviceManager9 *>(hdl);

			m_pmfxAllocatorParams = pd3dAllocParams;
		}

		/* In case of video memory we must provide MediaSDK with external allocator
		thus we demonstrate "external allocator" usage model.
		Call SetAllocator to pass allocator to mediasdk */
		sts = m_mfxSession.SetFrameAllocator(m_pGeneralAllocator);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		//m_bExternalAlloc = true;
#elif defined LIBVA_SUPPORT
		sts = CreateHWDevice();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		/* It's possible to skip failed result here and switch to SW implementation,
		but we don't process this way */

		// provide device manager to MediaSDK
		VADisplay va_dpy = NULL;
		sts = m_hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL *)&va_dpy);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		sts = m_mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, va_dpy);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		vaapiAllocatorParams *p_vaapiAllocParams = new vaapiAllocatorParams;
		MSDK_CHECK_POINTER(p_vaapiAllocParams, MFX_ERR_MEMORY_ALLOC);

		p_vaapiAllocParams->m_dpy = va_dpy;
		m_pmfxAllocatorParams = p_vaapiAllocParams;

		/* In case of video memory we must provide MediaSDK with external allocator
		thus we demonstrate "external allocator" usage model.
		Call SetAllocator to pass allocator to mediasdk */
		sts = m_mfxSession.SetFrameAllocator(m_pGeneralAllocator);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		//m_bExternalAlloc = true;
#endif
	}
	else
	{ 
		if (!m_pmfxAllocatorParams)
		{
			mfxAllocatorParams* allocatorParams = new mfxAllocatorParams;
			MSDK_CHECK_POINTER(allocatorParams, MFX_ERR_MEMORY_ALLOC);
			m_pmfxAllocatorParams = allocatorParams;

	#ifdef LIBVA_SUPPORT
			//in case of system memory allocator we also have to pass MFX_HANDLE_VA_DISPLAY to HW library
			mfxIMPL impl;
			m_mfxSession.QueryIMPL(&impl);

			if(MFX_IMPL_HARDWARE == MFX_IMPL_BASETYPE(impl))
			{
				sts = CreateHWDevice();
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

				// provide device manager to MediaSDK
				VADisplay va_dpy = NULL;
				sts = m_hwdev->GetHandle(MFX_HANDLE_VA_DISPLAY, (mfxHDL *)&va_dpy);
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
				sts = m_mfxSession.SetHandle(MFX_HANDLE_VA_DISPLAY, va_dpy);
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			}
	#endif
		}
	}
    // initialize general allocator
    sts = m_pGeneralAllocator->Init(m_pmfxAllocatorParams);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	return MFX_ERR_NONE;
}

mfxStatus CIMFXCommon::CreateHWDevice()
{
	mfxStatus sts = MFX_ERR_NONE;
#ifdef D3D_SURFACES_SUPPORT

	HWND window = (m_bIsRender) ? m_d3dRender.GetWindowHandle() : NULL;

#if MFX_D3D11_SUPPORT
	if (D3D11_MEMORY == m_enMemType)
		m_hwdev = new CD3D11Device();
	else
#endif // #if MFX_D3D11_SUPPORT
		m_hwdev = new CD3D9Device();

	if (NULL == m_hwdev)
		return MFX_ERR_MEMORY_ALLOC;

	sts = m_hwdev->Init(
		window,
		m_bIsRender ? 1 : 0,
		MSDKAdapter::GetNumber(m_mfxSession));
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	if (m_bIsRender)
		m_d3dRender.SetHWDevice(m_hwdev);

#elif LIBVA_SUPPORT
	m_hwdev = CreateVAAPIDevice();
	if (NULL == m_hwdev)
	{
		return MFX_ERR_MEMORY_ALLOC;
	}
	sts = m_hwdev->Init((mfxHDL )m_hWindow, (mfxHDL)m_pDisplay, (true == m_bIsRender) ? 1 : 0, MSDKAdapter::GetNumber(m_mfxSession));
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#endif
	return MFX_ERR_NONE;
}

void CIMFXCommon::DeleteAllocator()
{
	// delete allocator
    MSDK_SAFE_DELETE(m_pGeneralAllocator);
	MSDK_SAFE_DELETE(m_pmfxAllocatorParams);
	MSDK_SAFE_DELETE(m_hwdev);
}

IMFX_STS CIMFXCommon::RenderFrame(mfxFrameSurface1 *pSurface)
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	MSDK_CHECK_POINTER(pSurface, IMFX_ERR_NULL_PTR);

	if (true == this->m_bIsRender)
	{
#if defined(WIN32) || defined(WIN64)
		return (IMFX_STS)m_d3dRender.RenderFrame(pSurface, m_pGeneralAllocator);
#else
		return (IMFX_STS)m_hwdev->RenderFrame(pSurface, m_pGeneralAllocator);
#endif
	}
	else
	{
		return IMFX_ERR_INVALID_CALLED;
	}
}

IMFX_STS CIMFXCommon::GetCurrentMediaSDKInfo(mfxVersion &ver, mfxIMPL &impl)
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxSession session = NULL;
	ver.Major = 1;
	ver.Minor = 0;

	do
	{
		sts = MFXInit(MFX_IMPL_AUTO, &ver, &session);
		MSDK_BREAK_ON_ERROR(sts);
		sts = DISPATCHER_EXPOSED_PREFIX(MFXQueryVersion)(session, &ver);
		MSDK_BREAK_ON_ERROR(sts);
		sts = MFXQueryIMPL(session, &impl);
		MSDK_BREAK_ON_ERROR(sts);
	} while (0);

	MFXClose(session);

	return (IMFX_STS)sts;
}

#if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_X11_SUPPORT) || defined(LIBVA_ANDROID_SUPPORT)
mfx_hdl CIMFXCommon::GetDisplayHandle(void)
{
	return m_pDisplay;
}
#endif


IMFX_STS CIMFXCommon::DrawPolygon(POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum)
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

#if ((defined(WIN32) || defined(WIN64)) && (defined(DRAW_LINE)))
	mfxStatus sts = m_hwdev->DrawPolygon(pstPolygonAttr, ulPolygonNum);
	if (MFX_ERR_NONE != sts)
	{
		return IMFX_ERR_UNSUPPORTED;
	}
	else
	{
		return IMFX_ERR_NONE;
	}
#else
	return IMFX_ERR_UNSUPPORTED;
#endif
}

IMFX_STS CIMFXCommon::GetDeviceHandle(mfxHDL *pHdl)
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	mfxStatus sts = m_hwdev->GetDeviceHandle(pHdl);
	if (MFX_ERR_NONE != sts)
	{
		return IMFX_ERR_UNKNOWN;
	}

	return IMFX_ERR_NONE;
}

IMFX_STS CIMFXCommon::QueryVersion(mfxVersion &ver)
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	mfxStatus sts = DISPATCHER_EXPOSED_PREFIX(MFXQueryVersion)(*(&m_mfxSession), &ver);
	if (MFX_ERR_NONE != sts)
	{
		return IMFX_ERR_UNKNOWN;
	}
	else
	{
		return IMFX_ERR_NONE;
	}
}

IMFX_STS CopyFromRawData2Surface(RAW_DATA_S *pstRawData, mfxFrameSurface1* pmfxSurface)
{
	// check if reader is initialized
	//MSDK_CHECK_ERROR(m_bInited, false, MFX_ERR_NOT_INITIALIZED);
	MSDK_CHECK_POINTER(pmfxSurface, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstRawData, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstRawData->pcDataAddr, IMFX_ERR_NULL_PTR);

	mfxU16 w, h, i, pitch;
	mfxU8 *ptr, *ptr2;
	mfxFrameInfo* pInfo = &pmfxSurface->Info;
	mfxFrameData* pData = &pmfxSurface->Data;

	//mfxU32 vid = pmfxSurface->Info.FrameId.ViewId;

	// this reader supports only NV12 mfx surfaces for code transparency,
	// other formats may be added if application requires such functionality
	
	if (IMFX_FOURCC_BGR4 == pstRawData->FourCC)
	{
		return copyFromBGR24ToSurface(pstRawData, pmfxSurface);
	}

	if (IMFX_FOURCC_NV12 != pstRawData->FourCC && IMFX_FOURCC_YV12 != pstRawData->FourCC)
	{
		return IMFX_ERR_UNSUPPORTED;
	}

	if (pInfo->CropH > 0 && pInfo->CropW > 0)
	{
		w = pInfo->CropW;
		h = pInfo->CropH;
	}
	else
	{
		w = pInfo->Width;
		h = pInfo->Height;
	}

	pitch = pData->Pitch;
	ptr = pData->Y + pInfo->CropX + pInfo->CropY * pData->Pitch;

	mfx_u32 ulYV12Offset = 0;

	for (i = 0; i < h; i++)  // load Y
	{
		memcpy(ptr + i * pitch, pstRawData->pcDataAddr + ulYV12Offset, w);
		ulYV12Offset += w;
	}

	// read chroma planes
	switch (pstRawData->FourCC) // color format of data in the input file
	{
	case MFX_FOURCC_YV12: // YUV420 is implied
		switch (pInfo->FourCC)
		{
		case MFX_FOURCC_NV12:

			mfxU8 buf[2048]; // maximum supported chroma width for nv12
			mfxU32 j;
			w /= 2;
			h /= 2;
			ptr = pData->UV + pInfo->CropX + (pInfo->CropY / 2) * pitch;
			if (w > 2048)
			{
				return IMFX_ERR_UNSUPPORTED;
			}
			// load U
			for (i = 0; i < h; i++)
			{
				//nBytesRead = (mfxU32)fread(buf, 1, w, m_fSource);
				memcpy(buf, pstRawData->pcDataAddr + ulYV12Offset, w);
				ulYV12Offset += w;

				for (j = 0; j < w; j++)
				{
					ptr[i * pitch + j * 2] = buf[j];
				}
			}
			// load V
			for (i = 0; i < h; i++)
			{
				//nBytesRead = (mfxU32)fread(buf, 1, w, m_fSource);
				memcpy(buf, pstRawData->pcDataAddr + ulYV12Offset, w);
				ulYV12Offset += w;

				for (j = 0; j < w; j++)
				{
					ptr[i * pitch + j * 2 + 1] = buf[j];
				}
			}

			break;
		case MFX_FOURCC_YV12:
			w /= 2;
			h /= 2;
			pitch /= 2;

			ptr = pData->U + (pInfo->CropX / 2) + (pInfo->CropY / 2) * pitch;
			ptr2 = pData->V + (pInfo->CropX / 2) + (pInfo->CropY / 2) * pitch;

			for (i = 0; i < h; i++)
			{
				//nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSource);
				memcpy(ptr + i * pitch, pstRawData->pcDataAddr + ulYV12Offset, w);
				ulYV12Offset += w;
			}

			for (i = 0; i < h; i++)
			{
				//nBytesRead = (mfxU32)fread(ptr2 + i * pitch, 1, w, m_fSource);
				memcpy(ptr2 + i * pitch, pstRawData->pcDataAddr + ulYV12Offset, w);
				ulYV12Offset += w;
			}

			break;
		default:
			return IMFX_ERR_UNSUPPORTED;
		}
		break;
	case MFX_FOURCC_NV12:
		h /= 2;
		ptr = pData->UV + pInfo->CropX + (pInfo->CropY / 2) * pitch;
		for (i = 0; i < h; i++)
		{
			//nBytesRead = (mfxU32)fread(ptr + i * pitch, 1, w, m_fSource);
			memcpy(ptr, pstRawData->pcDataAddr + ulYV12Offset, w);
			ulYV12Offset += w;
		}

		break;

	default:
		return IMFX_ERR_UNSUPPORTED;
	}

	return IMFX_ERR_NONE;
}

IMFX_STS CopyFromSurface2Surface(mfxFrameSurface1* pmfxSurfaceSrc, mfxFrameSurface1* pmfxSurfaceDst)
{
	MSDK_CHECK_POINTER(pmfxSurfaceSrc, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pmfxSurfaceDst, IMFX_ERR_NULL_PTR);

	mfxU16 w, h, pitch;
	mfxFrameInfo* pInfo = &pmfxSurfaceSrc->Info;
	mfxFrameData* pData = &pmfxSurfaceSrc->Data;

	if (pInfo->CropH > 0 && pInfo->CropW > 0)
	{
		w = pInfo->CropW;
		h = pInfo->CropH;
	}
	else
	{
		w = pInfo->Width;
		h = pInfo->Height;
	}

	pitch = pData->Pitch;

	/* copy Y */
	memcpy(pmfxSurfaceDst->Data.Y, pmfxSurfaceSrc->Data.Y, h * pitch);

	w /= 2;
	h /= 2;
	//pitch /= 2;

	/* copy UV */
	memcpy(pmfxSurfaceDst->Data.UV, pmfxSurfaceSrc->Data.UV, h * pitch);

	return IMFX_ERR_NONE;
}

IMFX_STS CopyFromNV12ToYV12(IN mfxFrameSurface1 *pSurface, INOUT YV12_DATA_S *pstOutYV12Data)
{
	MSDK_CHECK_POINTER(pSurface, IMFX_ERR_NULL_PTR);

	mfxFrameInfo *pInfo = &pSurface->Info;
	mfxFrameData *pData = &pSurface->Data;
	MSDK_CHECK_POINTER(pData, IMFX_ERR_NULL_PTR);

	mfxU32 i, j, h, w;
	//mfxU32 vid = pSurface->Info.FrameId.ViewId;
	pstOutYV12Data->ulDataSize = 0;
	switch (pInfo->FourCC)
	{
	case MFX_FOURCC_NV12:

		/* Y */
		for (i = 0; i < pInfo->CropH; i++)
		{
			memcpy(pstOutYV12Data->pcDataAddr + pstOutYV12Data->ulDataSize,
				pData->Y + (pInfo->CropY * pData->Pitch + pInfo->CropX) + i * pData->Pitch,
				pInfo->CropW);
			pstOutYV12Data->ulDataSize += pInfo->CropW;
		}

		h = pInfo->CropH / 2;
		w = pInfo->CropW;
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j += 2)
			{
				pstOutYV12Data->pcDataAddr[pstOutYV12Data->ulDataSize++] = (mfx_i8)pData->UV[(pInfo->CropY * pData->Pitch / 2 + pInfo->CropX) + i * pData->Pitch + j];
			}
		}
		for (i = 0; i < h; i++)
		{
			for (j = 1; j < w; j += 2)
			{
				pstOutYV12Data->pcDataAddr[pstOutYV12Data->ulDataSize++] = (mfx_i8)pData->UV[(pInfo->CropY * pData->Pitch / 2 + pInfo->CropX) + i * pData->Pitch + j];
			}
		}

		break;

	default:
		return IMFX_ERR_UNSUPPORTED;
	}

	pstOutYV12Data->ulHeight = pInfo->CropH;
	pstOutYV12Data->ulWidth = pInfo->CropW;
	pstOutYV12Data->ulPitch = pstOutYV12Data->ulWidth;

	return IMFX_ERR_NONE;
}

IMFX_STS CopyFromNV12ToYV12AlignedWith16(IN mfxFrameSurface1 *pSurface, INOUT YV12_DATA_S *pstOutYV12Data)
{
	MSDK_CHECK_POINTER(pSurface, IMFX_ERR_NULL_PTR);

	mfxFrameInfo *pInfo = &pSurface->Info;
	mfxFrameData *pData = &pSurface->Data;
	MSDK_CHECK_POINTER(pData, IMFX_ERR_NULL_PTR);

	mfxU32 i, j, h, w;
	//mfxU32 vid = pSurface->Info.FrameId.ViewId;
	pstOutYV12Data->ulDataSize = 0;

	switch (pInfo->FourCC)
	{
	case MFX_FOURCC_NV12:
		/* Y */
		for (i = 0; i < pInfo->Height; i++)
		{
			memcpy(pstOutYV12Data->pcDataAddr + pstOutYV12Data->ulDataSize,
				pData->Y + (pInfo->CropY * pData->Pitch + pInfo->CropX) + i * pData->Pitch,
				pInfo->Width);
			pstOutYV12Data->ulDataSize += pInfo->Width;
		}

		h = pInfo->Height / 2;
		w = pInfo->Width;
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j += 2)
			{
				pstOutYV12Data->pcDataAddr[pstOutYV12Data->ulDataSize++] = (mfx_i8)pData->UV[(pInfo->CropY * pData->Pitch / 2 + pInfo->CropX) + i * pData->Pitch + j];
			}
		}
		for (i = 0; i < h; i++)
		{
			for (j = 1; j < w; j += 2)
			{
				pstOutYV12Data->pcDataAddr[pstOutYV12Data->ulDataSize++] = (mfx_i8)pData->UV[(pInfo->CropY * pData->Pitch / 2 + pInfo->CropX) + i * pData->Pitch + j];
			}
		}

		break;

	default:
		return IMFX_ERR_UNSUPPORTED;
	}

	pstOutYV12Data->ulHeight = pInfo->Height;
	pstOutYV12Data->ulWidth = pInfo->Width;
	pstOutYV12Data->ulPitch = pData->Pitch;

	return IMFX_ERR_NONE;
}

IMFX_STS copyFromRGB32ToRGB24( IN mfxFrameSurface1 *pSurface, INOUT RAW_DATA_S *pstRawData )
{
	MSDK_CHECK_POINTER(pSurface, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstRawData, IMFX_ERR_NULL_PTR);

	int w = MSDK_ALIGN16(pSurface->Info.CropW);
	//int w = pSurface->Info.Width;
	int h = pSurface->Info.CropH;
	int pitch = pSurface->Data.Pitch;

	pstRawData->FourCC = IMFX_FOURCC_BGR4;
	pstRawData->ulHeight = h;
	pstRawData->ulWidth = w;
	pstRawData->ulPitch = pitch;
	pstRawData->ulDataSize = w * h * 3;

	if (pstRawData->ulDataSize > pstRawData->ulMaxLength)
	{
		return IMFX_ERR_INVALID_PARAM;
	}

	mfxFrameInfo &pInfo = pSurface->Info;
	mfxFrameData &pData = pSurface->Data;

	mfxU8* pSrc = MSDK_MIN( MSDK_MIN(pData.R, pData.G), pData.B);
	pSrc = pSrc + pInfo.CropX + pInfo.CropY * pData.Pitch;

	mfxU8* pDst = (mfxU8*)pstRawData->pcDataAddr;

	if (1)	//(1920 == w)
	{
		for (int j=0; j<h; j++)
		{
			__m128i *src = (__m128i *)(pSrc + j * pitch);
			__m128i *dst = (__m128i *)(pDst + j * w * 3);

			__m128i mask = _mm_setr_epi8(0,1,2,4, 5,6,8,9, 10,12,13,14, -1,-1,-1,-1);

			mfx_u32 Pixels = 16*(w/16);

			for (UINT i = 0; i < Pixels; i += 16) {
				__m128i sa = _mm_shuffle_epi8(_mm_load_si128(src), mask);
				__m128i sb = _mm_shuffle_epi8(_mm_load_si128(src + 1), mask);
				__m128i sc = _mm_shuffle_epi8(_mm_load_si128(src + 2), mask);
				__m128i sd = _mm_shuffle_epi8(_mm_load_si128(src + 3), mask);
				_mm_store_si128(dst, _mm_or_si128(sa, _mm_slli_si128(sb, 12)));
				_mm_store_si128(dst + 1, _mm_or_si128(_mm_srli_si128(sb, 4), _mm_slli_si128(sc, 8)));
				_mm_store_si128(dst + 2, _mm_or_si128(_mm_srli_si128(sc, 8), _mm_slli_si128(sd, 4)));
				src += 4;
				dst += 3;
			}
		}
	}
	else
	{
		for(int i = 0; i < h; i++)
		{
			for (int j=0; j < w; j++)
			{
				*(pDst++) = *(pSrc++);	//b
				*(pDst++) = *(pSrc++);	//g
				*(pDst++) = *(pSrc++);	//r
				pSrc++;
			}
			pSrc += (pitch-w * 4);
		}
		//pstRawData->ulDataSize = pDst-(mfxU8*)pstRawData->pcDataAddr;
	}
	return IMFX_ERR_NONE;
}

IMFX_STS copyFromRGB32ToRGB32( IN mfxFrameSurface1 *pSurface, INOUT RAW_DATA_S *pstRawData )
{
	MSDK_CHECK_POINTER(pSurface, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstRawData, IMFX_ERR_NULL_PTR);

	int w = pSurface->Info.CropW;
	int h = pSurface->Info.CropH;
	int pitch = pSurface->Data.Pitch;

	pstRawData->ulHeight = h;
	pstRawData->ulWidth = w;
	pstRawData->ulPitch = pitch;
	pstRawData->ulDataSize = w * h * 4;

	if (pstRawData->ulDataSize > pstRawData->ulMaxLength)
	{
		return IMFX_ERR_INVALID_PARAM;
	}

	mfxFrameInfo &pInfo = pSurface->Info;
	mfxFrameData &pData = pSurface->Data;

	mfxU8* pSrc = MSDK_MIN( MSDK_MIN(pData.R, pData.G), pData.B);
	pSrc = pSrc + pInfo.CropX + pInfo.CropY * pData.Pitch;

	mfxU8* pDst = (mfxU8*)pstRawData->pcDataAddr;

	for (int j=0; j<h; j++)
	{
		memcpy(pDst, pSrc, w*4);
		pDst += w*4;
		pSrc += pitch;
	}

	return IMFX_ERR_NONE;
}

IMFX_STS copyFromBGR24ToSurface(IN RAW_DATA_S *pstRawData, mfxFrameSurface1 *pmfxSurface)
{
	MSDK_CHECK_POINTER(pmfxSurface, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstRawData, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstRawData->pcDataAddr, IMFX_ERR_NULL_PTR);

	int w = pmfxSurface->Info.CropW;
	int h = pmfxSurface->Info.CropH;
	int pitch = pmfxSurface->Data.Pitch;

	mfxFrameInfo &pInfo = pmfxSurface->Info;
	mfxFrameData &pData = pmfxSurface->Data;

	if ((pstRawData->ulWidth != pmfxSurface->Info.CropW) || (pstRawData->ulHeight != h))
	{
		return IMFX_ERR_UNSUPPORTED;
	}

	mfxU8* pDst = MSDK_MIN( MSDK_MIN(pData.R, pData.G), pData.B);
	pDst = pDst + pInfo.CropX + pInfo.CropY * pData.Pitch;
	
	mfxU8* pSrc = (mfxU8*)pstRawData->pcDataAddr;

	if (0)//(0 == (w%16))
	{
		__m128i *src;
		__m128i *dst;

		__m128i mask = _mm_setr_epi8(0,1,2,-1, 3,4,5,-1, 6,7,8,-1, 9,10,11,-1);

		mfx_u32 Pixels = 16*(w/16);

		for (int j=0; j<h; j++)
		{
			dst = (__m128i *)(pDst + j * pitch);
			src = (__m128i *)(pSrc + j * w * 3);

			for (UINT i = 0; i < Pixels; i += 16) {
				__m128i sa = _mm_load_si128(src);
				__m128i sb = _mm_load_si128(src + 1);
				__m128i sc = _mm_load_si128(src + 2);
				__m128i val = _mm_shuffle_epi8(sa, mask);
				_mm_store_si128(dst, val);
				val = _mm_shuffle_epi8(_mm_alignr_epi8(sb, sa, 12), mask);
				_mm_store_si128(dst + 1, val);
				val = _mm_shuffle_epi8(_mm_alignr_epi8(sc, sb, 8), mask);
				_mm_store_si128(dst + 2, val);
				val = _mm_shuffle_epi8(_mm_alignr_epi8(sc, sc, 4), mask);
				_mm_store_si128(dst + 3, val);
				src += 3;
				dst += 4;
			}
		}
	}
	else
	{
		mfx_u8 *src, *dst;
		
		for (int j=0; j<h; j++)	//height
		{
			src = (mfx_u8 *)(pSrc + j * w * 3);
			dst = (mfx_u8 *)(pDst + j * pitch);

			for (int i=0; i<w; i++)
			{
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = 0;
				dst += 4;
				src += 3;
			}
		}
	}
	return IMFX_ERR_NONE;
}