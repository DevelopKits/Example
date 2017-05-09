#include "imfx.h"
#include "imfx_common.h"
#include "imfx_decode.h"

MFXVideoDECODE * CIMFXDecoder::m_pmfxDEC4DecodeHeader = NULL;
MFXVideoSession * CIMFXDecoder::m_pmfxSession4DecodeHeader = NULL;
MSDKMutex CIMFXDecoder::m_Mutex4DecodeHeader;

CIMFXDecoder::CIMFXDecoder(CIMFXCommon *poIMFXCommon)
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

	memset(&m_mfxResponse, 0, sizeof(m_mfxResponse));
	//m_nIndex = 0;
	//m_syncp = NULL;

	// create decoder
	try
	{
		m_pmfxDEC = new MFXVideoDECODE(*m_pmfxSession);
	}
	catch (std::bad_alloc &)
	{
		throw IMFX_ERR_MEMORY_ALLOC;
	}

	memset(&m_mfxVideoParams, 0, sizeof(m_mfxVideoParams));
	memset(&m_mfxBS, 0, sizeof(m_mfxBS));
	m_bGetStreamHeader = false;
	m_InitIsCalled = false;

	m_pCurrentFreeSurface = NULL;
	m_pCurrentFreeOutputSurface = NULL;
	m_pCurrentOutputSurface = NULL;
	m_outputCodecID = CODEC_ID_E::IMFX_FOURCC_NV12;
}

CIMFXDecoder::~CIMFXDecoder()
{
	Close();
}

IMFX_STS CIMFXDecoder::Init(DECODE_INIT_PARAMS_S *pstDecodeInitParams)
{
	MSDK_CHECK_POINTER(pstDecodeInitParams, IMFX_ERR_NULL_PTR);

	if (true == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_REINITIALIZE);
		return IMFX_ERR_REINITIALIZE;
	}

	/* check CodecID */
	if (IMFX_CODEC_JPEG == pstDecodeInitParams->enInputCodecID)
	{
		if ((IMFX_FOURCC_NV12 != pstDecodeInitParams->enOutputCodecID) && (IMFX_FOURCC_RGB4 != pstDecodeInitParams->enOutputCodecID))
		{
			IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
			return IMFX_ERR_UNSUPPORTED;
		}
	}
	else
	{
		if (IMFX_FOURCC_NV12 != pstDecodeInitParams->enOutputCodecID)
		{
			IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
			return IMFX_ERR_UNSUPPORTED;
		}
	}

	/* get system's api version */
	IMFX_STS retval = IMFX_ERR_NONE;
	mfxVersion version;     // real API version with which library is initialized
	retval = m_poIMFXCommon->QueryVersion(version); // get real API version of the loaded library
	if (IMFX_ERR_NONE != retval)
	{
		IMFX_ERR("err: %d", retval);
		return retval;
	}

	/* check if version fit for jpeg decoding */
	if ((IMFX_CODEC_JPEG == pstDecodeInitParams->enInputCodecID) && !CheckVersion(&version, MSDK_FEATURE_JPEG_DECODE))
	{
		msdk_printf(MSDK_STRING("error: Jpeg is not supported in the %d.%d API version\n"),
					version.Major, version.Minor);
		return IMFX_ERR_UNSUPPORTED;
	}

	/* fill m_mfxVideoParams */

	MSDK_MEMCPY_VAR(m_mfxVideoParams.mfx, pstDecodeInitParams->pstMediaInfo, sizeof(mfxInfoMFX_ext));

	m_mfxVideoParams.mfx.CodecId = pstDecodeInitParams->enInputCodecID;
	m_mfxVideoParams.IOPattern = (mfxU16)(m_enMemType != SYSTEM_MEMORY ? MFX_IOPATTERN_OUT_VIDEO_MEMORY : MFX_IOPATTERN_OUT_SYSTEM_MEMORY);

	/* manual set surface's width and height */
	if (pstDecodeInitParams->ulSurfaceOutWidth > m_mfxVideoParams.mfx.FrameInfo.Width)
	{
		m_mfxVideoParams.mfx.FrameInfo.Width = MSDK_ALIGN16(pstDecodeInitParams->ulSurfaceOutWidth);
	}

	if (pstDecodeInitParams->ulSurfaceOutHeight > m_mfxVideoParams.mfx.FrameInfo.Height)
	{
		m_mfxVideoParams.mfx.FrameInfo.Height = (MFX_PICSTRUCT_PROGRESSIVE == m_mfxVideoParams.vpp.In.PicStruct) ?
			MSDK_ALIGN16(pstDecodeInitParams->ulSurfaceOutHeight) :
			MSDK_ALIGN32(pstDecodeInitParams->ulSurfaceOutHeight);
	}

	/* manual set output CodecID, just for jpeg decode */
	m_outputCodecID = pstDecodeInitParams->enOutputCodecID;
	if (IMFX_FOURCC_RGB4 == m_outputCodecID)
	{
		m_mfxVideoParams.mfx.FrameInfo.FourCC = m_outputCodecID;
		m_mfxVideoParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
	}

	//indicate the async depth
	m_mfxVideoParams.AsyncDepth = pstDecodeInitParams->ulAsyncDepth;
	if ((m_mfxVideoParams.AsyncDepth < 1) || (m_mfxVideoParams.AsyncDepth > 20))
	{
		m_mfxVideoParams.AsyncDepth = DEFAULT_ASYNC_DEPTH;
	}

	mfxStatus sts = MFX_ERR_NONE;

	sts = m_pmfxDEC->Init(&m_mfxVideoParams);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	sts = AllocSurfaces();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	m_InitIsCalled = true;
	return IMFX_ERR_NONE;
}

void CIMFXDecoder::Close()
{
	MSDK_SAFE_DELETE(m_pmfxDEC);

	FreeBuffers();

	m_pCurrentFreeSurface = NULL;

	MSDK_SAFE_FREE(m_pCurrentFreeOutputSurface);

	if (NULL != m_pGeneralAllocator)
	{
		m_pGeneralAllocator->Free(m_pGeneralAllocator->pthis, &m_mfxResponse);
	}
	m_pGeneralAllocator = NULL;
	m_pmfxSession   = NULL;

	memset(&m_mfxResponse, 0, sizeof(m_mfxResponse));

	memset(&m_mfxVideoParams, 0, sizeof(m_mfxVideoParams));
	//m_syncp = NULL;
	memset(&m_mfxBS, 0, sizeof(m_mfxBS));
	m_bGetStreamHeader = false;

	//m_nIndex = 0;
	m_InitIsCalled = false;
}

mfxStatus CIMFXDecoder::AllocSurfaces()
{
	mfxStatus sts = MFX_ERR_NONE;

	mfxU16 nSurfNum = 0; // number of surfaces for decoder

	mfxFrameAllocRequest Request;
	MSDK_ZERO_MEMORY(Request);

	//#ifdef MFX_D3D11_SUPPORT
	//    Request.Type |= WILL_READ; // This line is only required for Windows DirectX11 to ensure that surfaces can be retrieved by the application
	//#endif

	//sts = m_pmfxDEC->Query(&m_mfxVideoParams, &m_mfxVideoParams);
	//MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
	//MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = m_pmfxDEC->QueryIOSurf(&m_mfxVideoParams, &Request);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	if (Request.NumFrameSuggested < m_mfxVideoParams.AsyncDepth)
	{ 
		IMFX_ERR("err: %d", MFX_ERR_MEMORY_ALLOC);
		return MFX_ERR_MEMORY_ALLOC; 
	}

	nSurfNum = MSDK_MAX(Request.NumFrameSuggested, 1);

	// prepare allocation request
	Request.NumFrameSuggested = Request.NumFrameMin = nSurfNum;

	// alloc frames for decoder
	sts = m_pGeneralAllocator->Alloc(m_pGeneralAllocator->pthis, &Request, &m_mfxResponse);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// prepare mfxFrameSurface1 array for decoder
	nSurfNum = m_mfxResponse.NumFrameActual;

	sts = AllocBuffers(nSurfNum);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	for (int i = 0; i < nSurfNum; i++)
	{
		// initating each frame:
		MSDK_MEMCPY_VAR(m_pSurfaces[i].frame.Info, &(Request.Info), sizeof(mfxFrameInfo));
		if (SYSTEM_MEMORY != m_enMemType)
		{
			m_pSurfaces[i].frame.Data.MemId = m_mfxResponse.mids[i];
		}
		else
		{
			sts = m_pGeneralAllocator->Lock(m_pGeneralAllocator->pthis, m_mfxResponse.mids[i], &(m_pSurfaces[i].frame.Data));
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
	}

	return MFX_ERR_NONE;
}

IMFX_STS CIMFXDecoder::RunDecoding(FRAME_DATA_S *pstFrameDataIn, BLOCK_MODE enBlockMode)
{
	MSDK_CHECK_POINTER(pstFrameDataIn, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstFrameDataIn->pcDataAddr, IMFX_ERR_NULL_PTR);

	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return IMFX_ERR_UNINITIALIZED;
	}

	mfxStatus sts = MFX_ERR_NONE;
	mfxFrameSurface1 *pOutSurface = NULL;

	m_mfxBS.Data = (mfxU8 *)pstFrameDataIn->pcDataAddr;
	m_mfxBS.DataLength = pstFrameDataIn->ulDataSize;
	m_mfxBS.DataOffset = pstFrameDataIn->ulDataOffset;
	m_mfxBS.DataFlag = 1;
	m_mfxBS.MaxLength = m_mfxBS.DataLength;

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

		pOutSurface = NULL;
		mfxU32  DataOffset = m_mfxBS.DataOffset;
		do
		{
			sts = m_pmfxDEC->DecodeFrameAsync(&m_mfxBS, &(m_pCurrentFreeSurface->frame), &pOutSurface, &(m_pCurrentFreeOutputSurface->syncp));
			if (MFX_WRN_DEVICE_BUSY == sts)
			{
				MSDK_SLEEP(1);  // Wait if device is busy, then repeat the same call to DecodeFrameAsync
				continue;
			}
			else if (MFX_ERR_MORE_DATA == sts)   //if need more data, break the loop
			{
				if (0 == m_mfxBS.DataLength)
				{
					sts = m_pmfxDEC->DecodeFrameAsync(NULL, &(m_pCurrentFreeSurface->frame), &pOutSurface, &(m_pCurrentFreeOutputSurface->syncp));
					if (MFX_ERR_NONE == sts)
					{
						break;
					}
					else
					{
						pstFrameDataIn->ulDataOffset = 0;
						IMFX_ERR("err: %d", IMFX_ERR_MORE_DATA);
						return IMFX_ERR_MORE_DATA;
					}
				}
				else
				{
					if (DataOffset == m_mfxBS.DataOffset)
					{
						pstFrameDataIn->ulDataOffset = m_mfxBS.DataOffset;
						IMFX_ERR("err: %d", IMFX_ERR_MORE_DATA);
						return IMFX_ERR_MORE_DATA;
					}
					else
					{
						DataOffset = m_mfxBS.DataOffset;
						continue;
					}
				}
			}
			else if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts)
			{
				IMFX_WARN("MFX_ERR_INCOMPATIBLE_VIDEO_PARAM");
				mfxInfoMFX_ext mfx_info;
				DecodeHeader(IMFX_CODEC_JPEG, pstFrameDataIn, &mfx_info);

				if ((mfx_info.FrameInfo.Width > m_mfxVideoParams.mfx.FrameInfo.Width)
					|| (mfx_info.FrameInfo.Height > m_mfxVideoParams.mfx.FrameInfo.Height))
				{
					break;
				}
				else
				{
					m_mfxVideoParams.mfx.FrameInfo.CropH = mfx_info.FrameInfo.CropH;
					m_mfxVideoParams.mfx.FrameInfo.CropW = mfx_info.FrameInfo.CropW;

					sts = m_pmfxDEC->Reset(&m_mfxVideoParams);
					if (MFX_WRN_PARTIAL_ACCELERATION == sts)
					{
						msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
						MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
					}
					MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

					m_pCurrentFreeSurface->frame.Info.CropH = mfx_info.FrameInfo.CropH;
					m_pCurrentFreeSurface->frame.Info.CropW = mfx_info.FrameInfo.CropW;
					m_mfxBS.Data = (mfxU8 *)pstFrameDataIn->pcDataAddr;
					m_mfxBS.DataLength = pstFrameDataIn->ulDataSize;
					m_mfxBS.DataOffset = pstFrameDataIn->ulDataOffset;
					m_mfxBS.DataFlag = 1;
					m_mfxBS.MaxLength = m_mfxBS.DataLength;
					continue;
				}
			}
			else if (MFX_WRN_VIDEO_PARAM_CHANGED == sts)  //if warns that the video param changed, just ingnore the warnning
			{
				continue;
			}
			else if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_SURFACE == sts))
			{
				break;
			}
			else if ((MFX_ERR_DEVICE_FAILED == sts) || (MFX_ERR_DEVICE_LOST == sts))
			{
				IMFX_ERR("err: %d", sts);

				break;
#if 0
				sts = m_pmfxDEC->Close();
				MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
				if (MFX_ERR_NONE != sts)
				{
					IMFX_ERR("err: %d", sts);
					break;
				}

				DeleteSurfaces();

				mfxInfoMFX_ext mfx_info;
				DecodeHeader(IMFX_CODEC_JPEG, pstFrameDataIn, &mfx_info);

				AllocSurfaces();

				sts = m_pmfxDEC->Init(&m_mfxVideoParams);
				if (MFX_WRN_PARTIAL_ACCELERATION == sts)
				{
					msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
					MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
				}
				
				if (MFX_ERR_NONE != sts)
				{
					IMFX_ERR("err: %d", sts);
					break;
				}
				else
				{
					m_mfxBS.Data = (mfxU8 *)pstFrameDataIn->pcDataAddr;
					m_mfxBS.DataLength = pstFrameDataIn->ulDataSize;
					m_mfxBS.DataOffset = pstFrameDataIn->ulDataOffset;
					m_mfxBS.DataFlag = 1;
					m_mfxBS.MaxLength = m_mfxBS.DataLength;
					continue;
				}
#endif
			}
			else
			{
				IMFX_ERR("err: %d", sts);
				break;
			}
		}
		while (1);

		if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_SURFACE == sts))
		{
			// if current free surface is locked we are moving it to the used surfaces array
			m_UsedSurfacesPool.AddSurface(m_pCurrentFreeSurface);
			m_pCurrentFreeSurface = NULL;
		}

		if (MFX_ERR_MORE_SURFACE == sts)
		{
			continue;
		}
		else if (MFX_ERR_NONE == sts)
		{
			if (m_pCurrentFreeOutputSurface->syncp)
			{
				mfxStatus _sts = m_pmfxSession->SyncOperation(m_pCurrentFreeOutputSurface->syncp, MSDK_DEC_WAIT_INTERVAL);
				if (MFX_ERR_NONE != _sts)
				{
					IMFX_ERR("err: %d", IMFX_ERR_MORE_DATA);
					return IMFX_ERR_MORE_DATA;
				}
			}

			msdkFrameSurface *surface = FindUsedSurface(pOutSurface);

			msdk_atomic_inc16(&(surface->render_lock));

			m_pCurrentFreeOutputSurface->surface = surface;
			m_OutputSurfacesPool.AddSurface(m_pCurrentFreeOutputSurface);
			m_pCurrentFreeOutputSurface = NULL;
			break;
		}
		else
		{
			break;
		}

	}
	while (1);

	return (IMFX_STS)sts;
}

IMFX_STS CIMFXDecoder::RunDecodingEx(FRAME_DATA_S *pstFrameDataIn, BLOCK_MODE enBlockMode, bool EndOfFile)
{
	MSDK_CHECK_POINTER(pstFrameDataIn, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstFrameDataIn->pcDataAddr, IMFX_ERR_NULL_PTR);

	if (false == m_InitIsCalled)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return IMFX_ERR_UNINITIALIZED;
	}

	mfxStatus sts = MFX_ERR_NONE;
	mfxFrameSurface1 *pOutSurface = NULL;

	m_mfxBS.Data = (mfxU8 *) pstFrameDataIn->pcDataAddr;
	m_mfxBS.DataLength = pstFrameDataIn->ulDataSize;
	m_mfxBS.DataOffset = 0;
	m_mfxBS.DataFlag = 1;
	m_mfxBS.MaxLength = m_mfxBS.DataLength;

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

		pOutSurface = NULL;
		do
		{
			if (false == EndOfFile)
			{
				sts = m_pmfxDEC->DecodeFrameAsync(&m_mfxBS, &(m_pCurrentFreeSurface->frame), &pOutSurface, &(m_pCurrentFreeOutputSurface->syncp));
			}
			else
			{
				sts = m_pmfxDEC->DecodeFrameAsync(NULL, &(m_pCurrentFreeSurface->frame), &pOutSurface, &(m_pCurrentFreeOutputSurface->syncp));
			}

			if (MFX_WRN_DEVICE_BUSY == sts)
			{
				MSDK_SLEEP(1);  // Wait if device is busy, then repeat the same call to DecodeFrameAsync
				continue;
			}
			else if (MFX_ERR_MORE_DATA == sts)      //if need more data, break the loop
			{
				IMFX_ERR("err: %d", IMFX_ERR_MORE_DATA);
				return IMFX_ERR_MORE_DATA;
			}
			else if (MFX_ERR_INCOMPATIBLE_VIDEO_PARAM == sts)
			{
				// TODO: should reset decoder
				break;
			}
			else if (MFX_WRN_VIDEO_PARAM_CHANGED == sts)     //if warns that the video param changed, just ingnore the warnning
			{
				continue;
			}
			else if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_SURFACE == sts))
			{
				break;
			}
			else
			{
				IMFX_ERR("err: %d", sts);
				break;
			}
		}
		while (1);

		if ((MFX_ERR_NONE == sts) || (MFX_ERR_MORE_SURFACE == sts))
		{
			// if current free surface is locked we are moving it to the used surfaces array
			m_UsedSurfacesPool.AddSurface(m_pCurrentFreeSurface);
			m_pCurrentFreeSurface = NULL;
		}

		if (MFX_ERR_MORE_SURFACE == sts)
		{
			continue;
		}
		else if (MFX_ERR_NONE == sts)
		{
			if (m_pCurrentFreeOutputSurface->syncp)
			{
				mfxStatus _sts = m_pmfxSession->SyncOperation(m_pCurrentFreeOutputSurface->syncp, MSDK_DEC_WAIT_INTERVAL);
				if (MFX_ERR_NONE != _sts)
				{
					IMFX_ERR("err: %d", IMFX_ERR_MORE_DATA);
					return IMFX_ERR_MORE_DATA;
				}
			}

			msdkFrameSurface *surface = FindUsedSurface(pOutSurface);

			msdk_atomic_inc16(&(surface->render_lock));

			m_pCurrentFreeOutputSurface->surface = surface;
			m_OutputSurfacesPool.AddSurface(m_pCurrentFreeOutputSurface);
			m_pCurrentFreeOutputSurface = NULL;
			break;
		}
		else
		{ 
			break; 
		}
	}
	while (1);

	return (IMFX_STS)sts;
}

IMFX_STS CIMFXDecoder::GetOutputSurface(mfxFrameSurface1 **pmfxSurface)
{
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

void CIMFXDecoder::PutBackOutputSurface()
{
	if (NULL != m_pCurrentOutputSurface)
	{
		ReturnSurfaceToBuffers(m_pCurrentOutputSurface);
		m_pCurrentOutputSurface = NULL;
	}
	SyncFrameSurfaces();
}

IMFX_STS CIMFXDecoder::CopyOutputSurface(mfxFrameSurface1 **pmfxSurface)
{
	IMFX_ERR("err: %d", IMFX_ERR_UNSUPPORTED);
	return IMFX_ERR_UNSUPPORTED;
}

mfx_u32 CIMFXDecoder::GetFreeSurfaceCount()
{
	//return m_mfxVideoParams.AsyncDepth-m_OutputSurfacesPool.GetSurfaceCount();
	return m_FreeSurfacesPool.m_SurfacesCount;
}

mfx_u32 CIMFXDecoder::GetOutputSurfaceCount()
{
	return m_OutputSurfacesPool.GetSurfaceCount();
}

IMFX_STS CIMFXDecoder::GetVideoAttr(VIDEO_ATTR_S *pstVideoAttr)
{
	MSDK_CHECK_POINTER(pstVideoAttr, IMFX_ERR_NULL_PTR);

	if (false == m_bGetStreamHeader)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return IMFX_ERR_UNINITIALIZED;
	}

	pstVideoAttr->enCodecID = (CODEC_ID_E)m_mfxVideoParams.mfx.CodecId;
	pstVideoAttr->enPicStruct = (PIC_STRUCT_S)m_mfxVideoParams.mfx.FrameInfo.PicStruct;
	pstVideoAttr->ulBitRate = m_mfxVideoParams.mfx.TargetKbps * 1024;
	pstVideoAttr->ulHeight = m_pSurfaces[0].frame.Info.CropH;
	pstVideoAttr->ulWidth = m_pSurfaces[0].frame.Info.CropW;

	if ((m_mfxVideoParams.mfx.FrameInfo.FrameRateExtD > 0) && (m_mfxVideoParams.mfx.FrameInfo.FrameRateExtN > 0))
	{
		pstVideoAttr->ulFrameRate = m_mfxVideoParams.mfx.FrameInfo.FrameRateExtN / m_mfxVideoParams.mfx.FrameInfo.FrameRateExtD;
	}
	else
	{
		pstVideoAttr->ulFrameRate = 25;
	}

	return IMFX_ERR_NONE;
}

IMFX_STS CIMFXDecoder::GetFrameInfo(mfxFrameInfo *pstFrameInfo)
{
	if (false == m_bGetStreamHeader)
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNINITIALIZED);
		return IMFX_ERR_UNINITIALIZED;
	}

	if (NULL != pstFrameInfo)
	{
		*pstFrameInfo = m_mfxVideoParams.mfx.FrameInfo;
	}
	return IMFX_ERR_NONE;
}

IMFX_STS CIMFXDecoder::DecodeHeader(CODEC_ID_E codec_id, FRAME_DATA_S *pstFrameDataIn, mfxInfoMFX_ext *pstInfoMFX)
{
	AutomaticMutex lock(m_Mutex4DecodeHeader);
	MSDK_CHECK_POINTER(pstInfoMFX, IMFX_ERR_NULL_PTR);

	mfxStatus sts = MFX_ERR_NONE;

	if (NULL == m_pmfxDEC4DecodeHeader)
	{
		atexit(DeleteDecodeHeaderOBJ);

		mfxVersion ver = { { 8, 1 } };

		try
		{
			m_pmfxSession4DecodeHeader = new MFXVideoSession;
		}
		catch (std::bad_alloc &)
		{
			IMFX_ERR("err: %d", IMFX_ERR_MEMORY_ALLOC);
			return IMFX_ERR_MEMORY_ALLOC;
		}

		sts = m_pmfxSession4DecodeHeader->Init(MFX_IMPL_AUTO, &ver);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, IMFX_STS(sts));

		try
		{
			m_pmfxDEC4DecodeHeader = new MFXVideoDECODE(*m_pmfxSession4DecodeHeader);
		}
		catch (std::bad_alloc &)
		{
			IMFX_ERR("err: %d", IMFX_ERR_MEMORY_ALLOC);
			return IMFX_ERR_MEMORY_ALLOC;
		}
	}
	
	//msdk_tick start_time = msdk_time_get_tick();

	mfxVideoParam mfxVideoParams;
	MSDK_ZERO_MEMORY(mfxVideoParams);

	mfxBitstream            mfxBS;
	MSDK_ZERO_MEMORY(mfxBS);

	mfxBS.Data = (mfxU8 *)pstFrameDataIn->pcDataAddr;
	mfxBS.DataLength = pstFrameDataIn->ulDataSize;
	mfxBS.DataOffset = pstFrameDataIn->ulDataOffset;
	mfxBS.DataFlag = 1;
	mfxBS.MaxLength = mfxBS.DataLength;

	mfxVideoParams.mfx.CodecId = codec_id;

	if (IMFX_CODEC_JPEG == codec_id)
	{ 
		MJPEG_AVI_ParsePicStruct(&mfxBS); 
	}

	sts = m_pmfxDEC4DecodeHeader->DecodeHeader(&mfxBS, &mfxVideoParams);
	MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	if ((MFX_ERR_MORE_DATA == sts) && (0 != mfxBS.DataLength))
	{
		sts = m_pmfxDEC4DecodeHeader->DecodeHeader(&mfxBS, &mfxVideoParams);
	}

	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, IMFX_STS(sts));

	if (sizeof(mfxInfoMFX_ext) != sizeof(mfxVideoParams.mfx))
	{
		IMFX_ERR("err: %d", IMFX_ERR_UNKNOWN);
		return IMFX_ERR_UNKNOWN;
	}

	memcpy(pstInfoMFX, &mfxVideoParams.mfx, sizeof(mfxVideoParams.mfx));

	//printf("decode time: %6.3f ms\n", MSDK_GET_TIME(msdk_time_get_tick(), start_time, msdk_time_get_frequency()) * 1000);

	return IMFX_STS(sts);
}

mfxStatus CIMFXDecoder::DeleteSurfaces()
{
	FreeBuffers();

	if (NULL != m_pGeneralAllocator)
	{
		m_pGeneralAllocator->Free(m_pGeneralAllocator->pthis, &m_mfxResponse);
	}

	return MFX_ERR_NONE;
}
