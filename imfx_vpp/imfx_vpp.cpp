#include "imfx.h"
#include "imfx_common.h"
#include "imfx_vpp.h"

CIMFXVpp::CIMFXVpp(CIMFXCommon *poIMFXCommon)
{
	if (NULL == poIMFXCommon)
	{
		throw IMFX_ERR_NULL_PTR;
	}

	if (false == poIMFXCommon->IsInitialized())
	{
		throw IMFX_ERR_UNINITIALIZED_COMMON;
	}

	m_poIMFXCommon = poIMFXCommon;
	m_pmfxSession = poIMFXCommon->GetSession();
	m_pGeneralAllocator = poIMFXCommon->GetAllocator();
	m_enMemType = poIMFXCommon->GetMemType();

	memset(&m_mfxResponseIn, 0, sizeof(m_mfxResponseIn));
	memset(&m_mfxResponseOut, 0, sizeof(m_mfxResponseOut));
	m_enVPPSurfaceUsage = NOT_USE_VPP_SURFACE;
	//m_syncp = NULL;

	memset(&m_mfxVideoParams, 0, sizeof(m_mfxVideoParams));
	m_pmfxVPP = NULL;
	//m_pmfxSurfacesIn = NULL;
	//m_pmfxSurfacesOut = NULL;
	//m_numSurfacesIn = 0;
	//m_numSurfacesOut = 0;
	//m_nIndexIn = 0;
	//m_nIndexOut = 0;

	m_InitIsCalled = false;

	m_pCurrentFreeSurface = NULL;
	m_pCurrentFreeOutputSurface = NULL;
	m_pCurrentOutputSurface = NULL;
	m_surfaceSystemBuffersOut = NULL;
	m_bOutput2SystemMemory = false;

	MSDK_ZERO_MEMORY(m_stResize);
}

IMFX_STS CIMFXVpp::Init(VPP_INIT_PARAMS_S *pstVppInitParams, VPP_SURFACE_USAGE_E enVPPSurfaceUsage)
{
	MSDK_CHECK_POINTER(pstVppInitParams, IMFX_ERR_NULL_PTR);

	if (true == m_InitIsCalled)
	{
		IMFX_WARN("warning: %d", IMFX_ERR_REINITIALIZE);
		return IMFX_ERR_REINITIALIZE;
	}

	mfxStatus sts = MFX_ERR_NONE;
	m_enVPPSurfaceUsage = enVPPSurfaceUsage;

	m_bOutput2SystemMemory = pstVppInitParams->bOutput2SystemMemory;

	if (SYSTEM_MEMORY == m_enMemType)
	{
		m_bOutput2SystemMemory = false;
	}

	// Input data
	m_mfxVideoParams.vpp.In.FourCC = pstVppInitParams->stInVideoAttr.enCodecID;
	if (MFX_FOURCC_RGB4 == m_mfxVideoParams.vpp.In.FourCC)
	{
		m_mfxVideoParams.vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
	}
	else if (MFX_FOURCC_NV12 == m_mfxVideoParams.vpp.In.FourCC)
	{
		m_mfxVideoParams.vpp.In.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
	}
	else
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
		return IMFX_ERR_UNSUPPORTED;
	}
	m_mfxVideoParams.vpp.In.CropX = 0;
	m_mfxVideoParams.vpp.In.CropY = 0;
	m_mfxVideoParams.vpp.In.CropW = (mfxU16)pstVppInitParams->stInVideoAttr.ulWidth;
	m_mfxVideoParams.vpp.In.CropH = (mfxU16)pstVppInitParams->stInVideoAttr.ulHeight;
	m_mfxVideoParams.vpp.In.PicStruct = pstVppInitParams->stInVideoAttr.enPicStruct;//MFX_PICSTRUCT_PROGRESSIVE;
	m_mfxVideoParams.vpp.In.FrameRateExtN = (pstVppInitParams->stInVideoAttr.ulFrameRate > 0) ? pstVppInitParams->stInVideoAttr.ulFrameRate : 25;
	m_mfxVideoParams.vpp.In.FrameRateExtD = 1;
	// width must be a multiple of 16
	// height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
	m_mfxVideoParams.vpp.In.Width = MSDK_ALIGN16(m_mfxVideoParams.vpp.In.CropW);
	m_mfxVideoParams.vpp.In.Height =
		(MFX_PICSTRUCT_PROGRESSIVE == m_mfxVideoParams.vpp.In.PicStruct) ?
		MSDK_ALIGN16(m_mfxVideoParams.vpp.In.CropH) :
		MSDK_ALIGN32(m_mfxVideoParams.vpp.In.CropH);

	if (MSDK_ALIGN16(pstVppInitParams->ulSurfaceInWidth) > m_mfxVideoParams.vpp.In.Width)
	{
		m_mfxVideoParams.vpp.In.Width = MSDK_ALIGN16(pstVppInitParams->ulSurfaceInWidth);
	}

	mfx_u32 surfaceInHeight = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVideoParams.vpp.In.PicStruct) ?
							  MSDK_ALIGN16(pstVppInitParams->ulSurfaceInHeight) :
							  MSDK_ALIGN32(pstVppInitParams->ulSurfaceInHeight);

	if (surfaceInHeight > m_mfxVideoParams.vpp.In.Height)
	{
		m_mfxVideoParams.vpp.In.Height = surfaceInHeight;
	}

	// Output data
	m_mfxVideoParams.vpp.Out.FourCC = pstVppInitParams->stOutVideoAttr.enCodecID;
	if (MFX_FOURCC_RGB4 == m_mfxVideoParams.vpp.Out.FourCC)
	{
		m_mfxVideoParams.vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
	}
	else if (MFX_FOURCC_NV12 == m_mfxVideoParams.vpp.Out.FourCC)
	{
		m_mfxVideoParams.vpp.Out.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
	}
	else
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
		return IMFX_ERR_UNSUPPORTED;
	}

	m_stResize = pstVppInitParams->stResize;

	mfx_u32 width = 0, height = 0;

	if ((BY_WIDTH_HEIGHT == m_stResize.mode) && (m_stResize.u.s.height > 0) && (m_stResize.u.s.width > 0))
	{
		height = m_stResize.u.s.height;
		width = m_stResize.u.s.width;
	}
	else if ((BY_RATIO == m_stResize.mode) && (m_stResize.u.ratio > 0.0f))
	{
		height = (mfx_u32)(m_mfxVideoParams.vpp.In.Height * m_stResize.u.ratio);
		width = (mfx_u32)(m_mfxVideoParams.vpp.In.Width * m_stResize.u.ratio);
	}
	else /* none resize */
	{
		height = m_mfxVideoParams.vpp.In.Height;
		width = m_mfxVideoParams.vpp.In.Width;
	}

	m_mfxVideoParams.vpp.Out.CropX = 0;
	m_mfxVideoParams.vpp.Out.CropY = 0;
	m_mfxVideoParams.vpp.Out.CropW = (mfxU16)width;   // 1/16th the resolution of decode stream
#if defined(WIN32) || defined(WIN64)
	//m_mfxVideoParams.vpp.Out.CropW &= ~0x01;	/* odd number has problem in render */
#endif
	m_mfxVideoParams.vpp.Out.CropH = (mfxU16)height;
#if defined(WIN32) || defined(WIN64)
	//m_mfxVideoParams.vpp.Out.CropH &= ~0x01; /* odd number has problem in render */
#endif
	m_mfxVideoParams.vpp.Out.PicStruct = pstVppInitParams->stOutVideoAttr.enPicStruct;		//MFX_PICSTRUCT_PROGRESSIVE;
	m_mfxVideoParams.vpp.Out.FrameRateExtN = (pstVppInitParams->stOutVideoAttr.ulFrameRate > 0) ? pstVppInitParams->stOutVideoAttr.ulFrameRate : 25;
	m_mfxVideoParams.vpp.Out.FrameRateExtD = 1;
	// width must be a multiple of 16
	// height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
	m_mfxVideoParams.vpp.Out.Width = MSDK_ALIGN16(m_mfxVideoParams.vpp.Out.CropW);
	m_mfxVideoParams.vpp.Out.Height =
		(MFX_PICSTRUCT_PROGRESSIVE == m_mfxVideoParams.vpp.Out.PicStruct) ?
		MSDK_ALIGN16(m_mfxVideoParams.vpp.Out.CropH) :
		MSDK_ALIGN32(m_mfxVideoParams.vpp.Out.CropH);

	m_mfxVideoParams.IOPattern = (mfxU16)((m_enMemType != SYSTEM_MEMORY) ? MFX_IOPATTERN_IN_VIDEO_MEMORY : MFX_IOPATTERN_IN_SYSTEM_MEMORY);

	if (true == m_bOutput2SystemMemory)
	{
		m_mfxVideoParams.IOPattern |= MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
	}
	else
	{
		m_mfxVideoParams.IOPattern |= (mfxU16)((m_enMemType != SYSTEM_MEMORY) ? MFX_IOPATTERN_OUT_VIDEO_MEMORY : MFX_IOPATTERN_OUT_SYSTEM_MEMORY);
	}

	// Configure Media SDK to keep more operations in flight
	// - AsyncDepth represents the number of tasks that can be submitted, before synchronizing is required
	m_mfxVideoParams.AsyncDepth = pstVppInitParams->ulAsyncDepth;
	m_mfxVideoParams.AsyncDepth = ((m_mfxVideoParams.AsyncDepth <= 0) || (m_mfxVideoParams.AsyncDepth > 20)) ? 1 : m_mfxVideoParams.AsyncDepth;

	if ((pstVppInitParams->ulDenoiseFactor >= 1) && (pstVppInitParams->ulDenoiseFactor <= 100))
	{
		// Initialize extended buffer for frame processing
		// - Denoise           VPP denoise filter
		// - mfxExtVPPDoUse:   Define the processing algorithm to be used
		// - mfxExtVPPDenoise: Denoise configuration
		// - mfxExtBuffer:     Add extended buffers to VPP parameter configuration

		memset(&m_VppDoUse, 0, sizeof(m_VppDoUse));
		//mfxU32 tabDoUseAlg[2];
		m_VppDoUse.Header.BufferId = MFX_EXTBUFF_VPP_DOUSE;
		m_VppDoUse.Header.BufferSz = sizeof(mfxExtVPPDoUse);
		m_VppDoUse.NumAlg = 1;
		m_VppDoUse.AlgList = m_tabExtVppAlg;
		m_tabExtVppAlg[0] = MFX_EXTBUFF_VPP_DENOISE;

		mfxExtVPPDenoise denoiseConfig;
		memset(&denoiseConfig, 0, sizeof(denoiseConfig));
		denoiseConfig.Header.BufferId = MFX_EXTBUFF_VPP_DENOISE;
		denoiseConfig.Header.BufferSz = sizeof(mfxExtVPPDenoise);
		denoiseConfig.DenoiseFactor = pstVppInitParams->ulDenoiseFactor;       // can be 1-100

		//mfxExtVPPImageStab imageStabConfig;
		//memset(&imageStabConfig, 0, sizeof(imageStabConfig));
		//imageStabConfig.Header.BufferId = MFX_EXTBUFF_VPP_IMAGE_STABILIZATION;
		//imageStabConfig.Header.BufferSz = sizeof(mfxExtVPPImageStab);
		//imageStabConfig.Mode = MFX_IMAGESTAB_MODE_BOXING;	//MFX_IMAGESTAB_MODE_UPSCALE;

		//#define VPP_DETAIL
#ifdef VPP_DETAIL
		m_VppDoUse.NumAlg = 2;
		m_tabExtVppAlg[1] = MFX_EXTBUFF_VPP_DETAIL;

		mfxExtVPPDetail detailConfig;
		memset(&detailConfig, 0, sizeof(detailConfig));
		detailConfig.Header.BufferId = MFX_EXTBUFF_VPP_DETAIL;
		detailConfig.Header.BufferSz = sizeof(mfxExtVPPDetail);
		detailConfig.DetailFactor = 50;

		m_VppExtParams[0] = (mfxExtBuffer *) &m_VppDoUse;
		m_VppExtParams[1] = (mfxExtBuffer *) &denoiseConfig;
		m_VppExtParams[2] = (mfxExtBuffer *) &detailConfig;
		m_mfxVideoParams.NumExtParam = 3;
		m_mfxVideoParams.ExtParam = (mfxExtBuffer **) &m_VppExtParams[0];
#else
		m_VppExtParams[0] = (mfxExtBuffer *) &m_VppDoUse;
		m_VppExtParams[1] = (mfxExtBuffer *) &denoiseConfig;
		m_mfxVideoParams.NumExtParam = 2;
		m_mfxVideoParams.ExtParam = (mfxExtBuffer **) &m_VppExtParams[0];
#endif
	}
	else
	{
		m_VppDoNotUse.NumAlg = 4;
		m_VppDoNotUse.Header.BufferId = MFX_EXTBUFF_VPP_DONOTUSE;
		m_VppDoNotUse.Header.BufferSz = sizeof(m_VppDoNotUse);

		m_tabExtVppAlg[0] = MFX_EXTBUFF_VPP_DENOISE;         // turn off denoising (on by default)
		m_tabExtVppAlg[1] = MFX_EXTBUFF_VPP_SCENE_ANALYSIS;  // turn off scene analysis (on by default)
		m_tabExtVppAlg[2] = MFX_EXTBUFF_VPP_DETAIL;          // turn off detail enhancement (on by default)
		m_tabExtVppAlg[3] = MFX_EXTBUFF_VPP_PROCAMP;         // turn off processing amplified (on by default)

		m_VppDoNotUse.AlgList = m_tabExtVppAlg;

		m_VppExtParams[0] = (mfxExtBuffer *)&m_VppDoNotUse;

		m_mfxVideoParams.NumExtParam = 1;
		m_mfxVideoParams.ExtParam = (mfxExtBuffer **) &m_VppExtParams[0];
	}

	m_pmfxVPP = new MFXVideoVPP(*m_pmfxSession);
	MSDK_CHECK_POINTER(m_pmfxVPP, IMFX_ERR_MEMORY_ALLOC);

	/* alloc surface */
	sts = AllocSurfaces();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	sts = m_pmfxVPP->Init(&m_mfxVideoParams);
	MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	m_InitIsCalled = true;
	return IMFX_ERR_NONE;
}

mfxStatus CIMFXVpp::AllocSurfaces()
{
	mfxStatus sts = MFX_ERR_NONE;
	mfxU16 nSurfNum = 0; // number of surfaces

	if (NOT_USE_VPP_SURFACE == m_enVPPSurfaceUsage)
	{
		return MFX_ERR_NONE; 
	}

	mfxFrameAllocRequest Request[2];     // [0] - in, [1] - out
	memset(&Request, 0, sizeof(mfxFrameAllocRequest) * 2);

	sts = m_pmfxVPP->Query(&m_mfxVideoParams, &m_mfxVideoParams);
	MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = m_pmfxVPP->QueryIOSurf(&m_mfxVideoParams, Request);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	//VPPRequest[0].Type |= WILL_WRITE; // This line is only required for Windows DirectX11 to ensure that surfaces can be written to by the application
	//VPPRequest[1].Type |= WILL_READ; // This line is only required for Windows DirectX11 to ensure that surfaces can be retrieved by the application

	if (USE_VPP_SURFACE_IN & m_enVPPSurfaceUsage)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
		return MFX_ERR_UNSUPPORTED;
#if 0
		//TODO: maybe i can alloc one surface for vpp,need to verify this mind
		sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &VPPRequest[0], &m_mfxResponseIn);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		m_numSurfacesIn = m_mfxResponseIn.NumFrameActual;
		//IMFX_DBG("surface in number: %d\n", m_numSurfacesIn);

		m_pmfxSurfacesIn = new mfxFrameSurface1 [m_numSurfacesIn];
		MSDK_CHECK_POINTER(m_pmfxSurfacesIn, MFX_ERR_MEMORY_ALLOC);

		for (int i = 0; i < m_numSurfacesIn; i++)
		{
			memset(&m_pmfxSurfacesIn[i], 0, sizeof(mfxFrameSurface1));
			MSDK_MEMCPY_VAR(m_pmfxSurfacesIn[i].Info, &(m_mfxVideoParams.vpp.In), sizeof(mfxFrameInfo));
			if (SYSTEM_MEMORY == m_enMemType)
			{
				sts = m_pMFXAllocator->Lock(m_pMFXAllocator->pthis, m_mfxResponseIn.mids[i], &(m_pmfxSurfacesIn[i].Data));
				MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			}
			else
			{
				m_pmfxSurfacesIn[i].Data.MemId = m_mfxResponseIn.mids[i];  // MID (memory id) represent one D3D NV12 surface
			}
		}
#endif
	}

	if (USE_VPP_SURFACE_OUT & m_enVPPSurfaceUsage)
	{
		// Allocate required surfaces

		if (Request[1].NumFrameSuggested < m_mfxVideoParams.AsyncDepth)
		{
			IMFX_ERR("err: %d", MFX_ERR_MEMORY_ALLOC);
			return MFX_ERR_MEMORY_ALLOC;
		}

		nSurfNum = MSDK_MAX(Request[1].NumFrameSuggested, 1);

		Request[1].NumFrameSuggested = Request[1].NumFrameMin = nSurfNum;

		if (false == m_bOutput2SystemMemory)
		{
			sts = m_pGeneralAllocator->Alloc(m_pGeneralAllocator->pthis, &Request[1], &m_mfxResponseOut);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

			nSurfNum = m_mfxResponseOut.NumFrameActual;
		}

		//IMFX_DBG("surface out number: %d\n", m_numSurfacesOut);
		// Allocate surface headers (mfxFrameSurface1) for VPP

		sts = AllocBuffers(nSurfNum);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		for (int i = 0; i < nSurfNum; i++)
		{
			// initating each frame:
			MSDK_MEMCPY_VAR(m_pSurfaces[i].frame.Info, &(Request[1].Info), sizeof(mfxFrameInfo));
			if (true == m_bOutput2SystemMemory)
			{
				switch (m_mfxVideoParams.vpp.Out.FourCC)
				{
				case MFX_FOURCC_NV12:
					{
						mfxU16 width = (mfxU16) MSDK_ALIGN32(m_mfxVideoParams.vpp.Out.Width);
						mfxU16 height = (mfxU16) MSDK_ALIGN32(m_mfxVideoParams.vpp.Out.Height);
						mfxU8 bitsPerPixel = 12;        // NV12 format is a 12 bits per pixel format
						mfxU32 surfaceSize = width * height * bitsPerPixel / 8;

						if (NULL == m_surfaceSystemBuffersOut)
						{
							try
							{
								m_surfaceSystemBuffersOut = (mfxU8 *) new mfxU8[(size_t)(surfaceSize * nSurfNum)];
							}
							catch (std::bad_alloc &)
							{
								IMFX_ERR("err: %d", MFX_ERR_MEMORY_ALLOC);
								return MFX_ERR_MEMORY_ALLOC;
							}
						}

						MSDK_CHECK_POINTER(m_surfaceSystemBuffersOut, MFX_ERR_MEMORY_ALLOC);

						m_pSurfaces[i].frame.Data.Y = &m_surfaceSystemBuffersOut[(size_t)(surfaceSize * i)];
						m_pSurfaces[i].frame.Data.U = m_pSurfaces[i].frame.Data.Y + (size_t)(width * height);
						m_pSurfaces[i].frame.Data.V = m_pSurfaces[i].frame.Data.U + 1;
						m_pSurfaces[i].frame.Data.Pitch = width;
					}
					break;
				case MFX_FOURCC_RGB4:
					{
						mfxU16 width = (mfxU16) MSDK_ALIGN32(m_mfxVideoParams.vpp.Out.Width);
						mfxU16 height = (mfxU16) MSDK_ALIGN32(m_mfxVideoParams.vpp.Out.Height);
						mfxU8 bitsPerPixel = 32;        // RGB4 format is a 32 bits per pixel format
						mfxU32 surfaceSize = width * height * bitsPerPixel / 8;
						if (NULL == m_surfaceSystemBuffersOut)
						{
							try
							{
								m_surfaceSystemBuffersOut = (mfxU8 *) new mfxU8[(size_t)(surfaceSize * nSurfNum)];
							}
							catch (std::bad_alloc &)
							{
								IMFX_ERR("err: %d", MFX_ERR_MEMORY_ALLOC);
								return MFX_ERR_MEMORY_ALLOC;
							}
						}

						MSDK_CHECK_POINTER(m_surfaceSystemBuffersOut, MFX_ERR_MEMORY_ALLOC);

						m_pSurfaces[i].frame.Data.B = m_surfaceSystemBuffersOut;
						m_pSurfaces[i].frame.Data.G = m_surfaceSystemBuffersOut + 1;
						m_pSurfaces[i].frame.Data.R = m_surfaceSystemBuffersOut + 2;
						m_pSurfaces[i].frame.Data.A = m_surfaceSystemBuffersOut + 3;
						m_pSurfaces[i].frame.Data.Pitch = width * 4;
					}
					break;
				default:
					{
						IMFX_ERR("err: %d", MFX_ERR_UNSUPPORTED);
						return MFX_ERR_UNSUPPORTED;
					}
					break;
				}
			}
			else
			{
				if (SYSTEM_MEMORY != m_enMemType)
				{
					m_pSurfaces[i].frame.Data.MemId = m_mfxResponseOut.mids[i];
				}
				else
				{
					sts = m_pGeneralAllocator->Lock(m_pGeneralAllocator->pthis, m_mfxResponseOut.mids[i], &(m_pSurfaces[i].frame.Data));
					MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
				}
			}
		}
	}

	return MFX_ERR_NONE;
}

IMFX_STS CIMFXVpp::RunVPP( mfxFrameSurface1 *pmfxSurfaceIn, BLOCK_MODE enBlockMode)
{
	MSDK_CHECK_POINTER(pmfxSurfaceIn, IMFX_ERR_NULL_PTR);

	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return IMFX_ERR_UNINITIALIZED;
	}

	mfxStatus           sts = MFX_ERR_NONE;
	//mfxFrameSurface1 *pOutSurface = NULL;

	//bool isMultipleOut = false; //指示是否是帧率提升的标志，例如 30fps->60fps

	do
	{
		SyncFrameSurfaces();

		/* get a surface from free surface pool */
		if (NULL == m_pCurrentFreeSurface)
		{
			m_pCurrentFreeSurface = m_FreeSurfacesPool.GetSurface();

			if (NULL == m_pCurrentFreeSurface)
			{
				if (NON_BLOCK == enBlockMode)
				{
					IMFX_ERR("err: %d", IMFX_ERR_NO_ENOUGH_SURFACE);
					return IMFX_ERR_NO_ENOUGH_SURFACE;
				}
				else
				{
					MSDK_SLEEP(5);
					continue;
				}
			}
		}

		/* get a output surface from free output surface pool */
		if (NULL == m_pCurrentFreeOutputSurface)
		{
			m_pCurrentFreeOutputSurface = GetFreeOutputSurface();

			if (NULL == m_pCurrentFreeOutputSurface)
			{
				IMFX_ERR("err: %d", IMFX_ERR_NOT_FOUND);
				return  IMFX_ERR_NOT_FOUND;
			}
		}

		do
		{
			if ((m_pCurrentFreeSurface->frame.Info.CropW == 0) ||
				(m_pCurrentFreeSurface->frame.Info.CropH == 0))
			{
				m_pCurrentFreeSurface->frame.Info.CropW = pmfxSurfaceIn->Info.CropW;
				m_pCurrentFreeSurface->frame.Info.CropH = pmfxSurfaceIn->Info.CropH;
				m_pCurrentFreeSurface->frame.Info.CropX = pmfxSurfaceIn->Info.CropX;
				m_pCurrentFreeSurface->frame.Info.CropY = pmfxSurfaceIn->Info.CropY;
			}
			if (pmfxSurfaceIn->Info.PicStruct != m_pCurrentFreeSurface->frame.Info.PicStruct)
			{
				m_pCurrentFreeSurface->frame.Info.PicStruct = pmfxSurfaceIn->Info.PicStruct;
			}
			if ((pmfxSurfaceIn->Info.PicStruct == 0) && (m_pCurrentFreeSurface->frame.Info.PicStruct == 0))
			{
				m_pCurrentFreeSurface->frame.Info.PicStruct = pmfxSurfaceIn->Info.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
			}

			if ((pmfxSurfaceIn->Info.CropH != m_mfxVideoParams.vpp.In.CropH) || (pmfxSurfaceIn->Info.CropW != m_mfxVideoParams.vpp.In.CropW))
			{
				m_mfxVideoParams.vpp.In.CropH = pmfxSurfaceIn->Info.CropH;
				m_mfxVideoParams.vpp.In.CropW = pmfxSurfaceIn->Info.CropW;
			}

			{
				mfx_u32 width = 0, height = 0;

				if ((BY_WIDTH_HEIGHT == m_stResize.mode) && (m_stResize.u.s.height > 0) && (m_stResize.u.s.width > 0))
				{
					height = m_stResize.u.s.height;
					width = m_stResize.u.s.width;
				}
				else if ((BY_RATIO == m_stResize.mode) && (m_stResize.u.ratio > 0.0f))
				{
					height = (mfx_u32)(m_mfxVideoParams.vpp.In.CropH * m_stResize.u.ratio);
					width = (mfx_u32)(m_mfxVideoParams.vpp.In.CropW * m_stResize.u.ratio);
				}
				else /* none resize */
				{
					height = m_mfxVideoParams.vpp.In.CropH;
					width = m_mfxVideoParams.vpp.In.CropW;
				}

				if ((width > m_mfxVideoParams.vpp.Out.Width) || (height > m_mfxVideoParams.vpp.Out.Height))
				{
					sts = MFX_ERR_INCOMPATIBLE_VIDEO_PARAM;
					break;
				}

				m_pCurrentFreeSurface->frame.Info.CropW = width;
				m_pCurrentFreeSurface->frame.Info.CropH = height;
			}

			sts = m_pmfxVPP->RunFrameVPPAsync( pmfxSurfaceIn, &(m_pCurrentFreeSurface->frame), NULL, &(m_pCurrentFreeOutputSurface->syncp));

			if (MFX_WRN_DEVICE_BUSY == sts)
			{
				MSDK_SLEEP(1); // just wait and then repeat the same call to RunFrameVPPAsync
			}
		}
		while (MFX_WRN_DEVICE_BUSY == sts);

		if (MFX_ERR_MORE_DATA == sts)
		{
			// will never happen actually
			continue;
		}
		else if (MFX_ERR_MORE_SURFACE == sts)
		{
			//MFX_ERR_MORE_SURFACE means output is ready but need more surface
			//because VPP produce multiple out. example: Frame Rate Conversion 30->60
			//isMultipleOut = true;
			continue;
		}
		else if (MFX_ERR_NONE != sts)
		{
			IMFX_ERR("err: %d", sts);
			break;
		}

		if (MFX_ERR_NONE == sts)
		{
			if (m_pCurrentFreeOutputSurface->syncp)
			{
				int try_count = 2;
				mfxStatus _sts = MFX_ERR_NONE;
				do 
				{
					_sts = m_pmfxSession->SyncOperation(m_pCurrentFreeOutputSurface->syncp, MSDK_VPP_WAIT_INTERVAL);
					if (MFX_ERR_NONE == _sts)
					{
						break;
					}
					else if (MFX_WRN_IN_EXECUTION == _sts)
					{
						--try_count;
						continue;
					}
					else
					{
						IMFX_ERR("err: %d", _sts);
						return (IMFX_STS)_sts;
					}
				} while (try_count > 0);
				
				if (MFX_ERR_NONE != _sts)
				{
					IMFX_ERR("err: %d", _sts);
					return (IMFX_STS)_sts;
				}
			}

			m_UsedSurfacesPool.AddSurface(m_pCurrentFreeSurface);

			msdk_atomic_inc16(&(m_pCurrentFreeSurface->render_lock));

			m_pCurrentFreeOutputSurface->surface = m_pCurrentFreeSurface;
			m_OutputSurfacesPool.AddSurface(m_pCurrentFreeOutputSurface);

			m_pCurrentFreeOutputSurface = NULL;
			m_pCurrentFreeSurface = NULL;
			break;
		}
	}
	while (1);

	return (IMFX_STS)sts;
}

void CIMFXVpp::Close()
{
	if (m_pmfxVPP)
	{
		m_pmfxVPP->Close();
		delete m_pmfxVPP;
		m_pmfxVPP = NULL;
	}

	FreeBuffers();

	m_pCurrentFreeSurface = NULL;

	MSDK_SAFE_FREE(m_pCurrentFreeOutputSurface);

	//m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxResponseIn);
	memset(&m_mfxResponseIn, 0, sizeof(m_mfxResponseIn));

	if (NULL != m_pGeneralAllocator)
	{
		m_pGeneralAllocator->Free(m_pGeneralAllocator->pthis, &m_mfxResponseOut);
	}
	m_pGeneralAllocator = NULL;

	memset(&m_mfxResponseOut, 0, sizeof(m_mfxResponseOut));

	m_pmfxSession   = NULL;

	m_enVPPSurfaceUsage = NOT_USE_VPP_SURFACE;

	memset(&m_mfxVideoParams, 0, sizeof(m_mfxVideoParams));

	MSDK_SAFE_DELETE_ARRAY(m_surfaceSystemBuffersOut);

	m_InitIsCalled = false;
}

CIMFXVpp::~CIMFXVpp()
{
	Close();
}

mfx_u32 CIMFXVpp::GetFreeSurfaceCount()
{
	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return 0;
	}

	//return m_mfxVideoParams.AsyncDepth-m_OutputSurfacesPool.GetSurfaceCount();
	return m_FreeSurfacesPool.m_SurfacesCount;
}

mfx_u32 CIMFXVpp::GetOutputSurfaceCount()
{
	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return 0;
	}

	return m_OutputSurfacesPool.GetSurfaceCount();
}

IMFX_STS CIMFXVpp::GetOutputSurface(mfxFrameSurface1 **pmfxSurface)
{
	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	if (NULL == m_pCurrentOutputSurface)
	{
		m_pCurrentOutputSurface = m_OutputSurfacesPool.GetSurface();
		if (NULL == m_pCurrentOutputSurface)
		{
			IMFX_ERR("err: %d", IMFX_ERR_MORE_DATA);
			return IMFX_ERR_MORE_DATA;
		}
	}

	*pmfxSurface = &(m_pCurrentOutputSurface->surface->frame);

	return IMFX_ERR_NONE;
}

void CIMFXVpp::PutBackOutputSurface()
{
	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return ;
	}

	if (NULL != m_pCurrentOutputSurface)
	{
		ReturnSurfaceToBuffers(m_pCurrentOutputSurface);
		m_pCurrentOutputSurface = NULL;
	}
	SyncFrameSurfaces();
}

IMFX_STS CIMFXVpp::CopyOutputSurface(mfxFrameSurface1 **pmfxSurface)
{
	IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
	return IMFX_ERR_UNSUPPORTED;
}

IMFX_STS CIMFXVpp::GetVideoAttr(VIDEO_ATTR_S *pstVideoAttrIn, VIDEO_ATTR_S *pstVideoAttrOut)
{
	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
		return IMFX_ERR_UNINITIALIZED;
	}

	if ((NULL == pstVideoAttrIn) && (NULL == pstVideoAttrOut))
	{
		IMFX_ERR("err: %d", IMFX_ERR_NULL_PTR);
		return IMFX_ERR_NULL_PTR;
	}

	if (NULL != pstVideoAttrIn)
	{
		pstVideoAttrIn->enCodecID = (CODEC_ID_E)m_mfxVideoParams.vpp.In.FourCC;
		pstVideoAttrIn->enPicStruct = (PIC_STRUCT_S)m_mfxVideoParams.vpp.In.PicStruct;
		pstVideoAttrIn->ulBitRate = 0;
		pstVideoAttrIn->ulFrameRate = m_mfxVideoParams.vpp.In.FrameRateExtN / m_mfxVideoParams.vpp.In.FrameRateExtD;
		pstVideoAttrIn->ulHeight = m_mfxVideoParams.vpp.In.CropH;
		pstVideoAttrIn->ulWidth = m_mfxVideoParams.vpp.In.CropW;
	}

	if (NULL != pstVideoAttrOut)
	{
		pstVideoAttrOut->enCodecID = (CODEC_ID_E)m_mfxVideoParams.vpp.Out.FourCC;
		pstVideoAttrOut->enPicStruct = (PIC_STRUCT_S)m_mfxVideoParams.vpp.Out.PicStruct;
		pstVideoAttrOut->ulBitRate = m_mfxVideoParams.mfx.TargetKbps * 1024;
		pstVideoAttrOut->ulFrameRate = m_mfxVideoParams.vpp.Out.FrameRateExtN / m_mfxVideoParams.vpp.Out.FrameRateExtD;
		pstVideoAttrOut->ulHeight = m_mfxVideoParams.vpp.Out.CropH;
		pstVideoAttrOut->ulWidth = m_mfxVideoParams.vpp.Out.CropW;
	}

	return IMFX_ERR_NONE;
}

IMFX_STS CIMFXVpp::Resize(mfxFrameSurface1 *pmfxSurfaceIn, mfxFrameSurface1 *pmfxSurfaceOut)
{
	MSDK_CHECK_POINTER(pmfxSurfaceIn, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pmfxSurfaceOut, IMFX_ERR_NULL_PTR);

	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return IMFX_ERR_UNINITIALIZED;
	}

	mfxStatus           sts = MFX_ERR_NONE;
	mfxSyncPoint syncp = NULL;
	do
	{
		do
		{
			sts = m_pmfxVPP->RunFrameVPPAsync( pmfxSurfaceIn, pmfxSurfaceOut, NULL, &syncp);

			if (MFX_WRN_DEVICE_BUSY == sts)
			{
				MSDK_SLEEP(1); // just wait and then repeat the same call to RunFrameVPPAsync
			}
		}
		while (MFX_WRN_DEVICE_BUSY == sts);

		if (MFX_ERR_MORE_DATA == sts)   // will never happen actually
		{
			continue;
		}
		else if (MFX_ERR_MORE_SURFACE == sts)
		{
			//MFX_ERR_MORE_SURFACE means output is ready but need more surface
			//because VPP produce multiple out. example: Frame Rate Conversion 30->60
			//isMultipleOut = true;
			continue;
		}
		else if (MFX_ERR_NONE != sts)
		{
			break;
		}

		if (MFX_ERR_NONE == sts)
		{
			mfxStatus _sts = m_pmfxSession->SyncOperation(syncp, MSDK_DEC_WAIT_INTERVAL);
			if (MFX_ERR_NONE != _sts)
			{
				IMFX_ERR("err: %d", IMFX_ERR_MORE_DATA);
				return IMFX_ERR_MORE_DATA;
			}
		}
	}
	while (1);

	return (IMFX_STS)sts;
}

IMFX_STS CIMFXVpp::GetFrameInfo( mfxFrameInfo *pstFrameInfoIn, mfxFrameInfo *pstFrameInfoOut )
{
	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return IMFX_ERR_UNINITIALIZED;
	}

	if (NULL != pstFrameInfoIn)
	{
		*pstFrameInfoIn = m_mfxVideoParams.vpp.In;
	}

	if (NULL != pstFrameInfoOut)
	{
		*pstFrameInfoOut = m_mfxVideoParams.vpp.Out;
	}

	return IMFX_ERR_NONE;
}
