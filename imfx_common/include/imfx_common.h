#ifndef __IMFX_COMMON_H__
#define __IMFX_COMMON_H__

typedef enum tagMemType {
	SYSTEM_MEMORY = 0x00,
	D3D9_MEMORY = 0x01,
	D3D11_MEMORY = 0x02,
}MEM_TYPE_E;

class IMFX_API CIMFXCommon
{
public:
	CIMFXCommon();
	~CIMFXCommon();

	/* windows platform hDisplay always be NULL */
	IMFX_STS InitSession(mfxIMPL impl, mfxVersion ver, MEM_TYPE_E enMemType = D3D9_MEMORY, mfx_hdl hWindow = NULL, mfx_hdl hDisplay = NULL);

	IMFX_STS CloseSession();

	IMFX_STS RenderFrame(mfxFrameSurface1 *pSurface);

	IMFX_STS GetCurrentMediaSDKInfo(mfxVersion &ver, mfxIMPL &impl);
	
	IMFX_STS QueryVersion(mfxVersion &ver);

#if defined(LIBVA_DRM_SUPPORT) || defined(LIBVA_X11_SUPPORT) || defined(LIBVA_ANDROID_SUPPORT)
	mfx_hdl GetDisplayHandle(void);
#endif
	//mfxStatus Sync(mfxSyncPoint syncp);

	IMFX_STS LockSurface(mfxFrameSurface1 *pmfxSurface);
	
	IMFX_STS UnlockSurface(mfxFrameSurface1 *pmfxSurface);

	//IMFX_STS CopyFromRawData2Surface(RAW_DATA_S *pstRawData, mfxFrameSurface1* pmfxSurface);
	//IMFX_STS CopyFromSurface2Surface(mfxFrameSurface1* pmfxSurfaceSrc, mfxFrameSurface1* pmfxSurfaceDst);

	IMFX_STS DrawPolygon(POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum);

	IMFX_STS GetDeviceHandle(mfxHDL *pHdl);

	inline bool IsInitialized()
	{
		return m_InitIsCalled;
	}

	inline MFXVideoSession *GetSession()
	{
		return &m_mfxSession;
	}

	inline GeneralAllocator *GetAllocator()
	{
		return m_pGeneralAllocator;
	}

	inline MEM_TYPE_E GetMemType()
	{
		return m_enMemType;
	}

protected:
	//mfxStatus Initialize(mfxIMPL impl, mfxVersion ver, bool bCreateSharedHandles = false);
	mfxStatus Initialize(mfxIMPL impl, mfxVersion ver);
	mfxStatus CreateAllocator();
	mfxStatus CreateHWDevice();
	void DeleteAllocator();

	//MFXCoreInterface	m_mfxCore;

	MFXVideoSession     m_mfxSession;
	GeneralAllocator*   m_pGeneralAllocator;
	MEM_TYPE_E			m_enMemType;
	mfxAllocatorParams *m_pmfxAllocatorParams;
	CHWDevice		   *m_hwdev;
#if defined(WIN32) || defined(WIN64)
	CD3DRender        m_d3dRender;
	mfx_hdl			    m_hWindow;
#else
	mfx_hdl			    m_hWindow;
	mfx_hdl              m_pDisplay;
#endif
	bool				m_bIsRender;

	bool				m_InitIsCalled;
};

IMFX_API IMFX_STS CopyFromRawData2Surface(RAW_DATA_S *pstRawData, mfxFrameSurface1* pmfxSurface);

IMFX_API IMFX_STS CopyFromSurface2Surface(mfxFrameSurface1* pmfxSurfaceSrc, mfxFrameSurface1* pmfxSurfaceDst);

/* width and height was the real value of image's width and height */
IMFX_API IMFX_STS CopyFromNV12ToYV12(IN mfxFrameSurface1 *pSurface, INOUT YV12_DATA_S *pstOutYV12Data);

/* width and height was align with 16 */
IMFX_API IMFX_STS CopyFromNV12ToYV12AlignedWith16(IN mfxFrameSurface1 *pSurface, INOUT YV12_DATA_S *pstOutYV12Data);

IMFX_API IMFX_STS copyFromRGB32ToRGB24(IN mfxFrameSurface1 *pSurface, INOUT RAW_DATA_S *pstRawData);

IMFX_API IMFX_STS copyFromRGB32ToRGB32(IN mfxFrameSurface1 *pSurface, INOUT RAW_DATA_S *pstRawData);

IMFX_STS copyFromBGR24ToSurface(IN RAW_DATA_S *pstRawData, mfxFrameSurface1 *pmfxSurface);

#endif
