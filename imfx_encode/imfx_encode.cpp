#include "imfx.h"
#include "imfx_common.h"
#include "imfx_encode.h"

CIMFXEncoder::CIMFXEncoder(CIMFXCommon *poIMFXCommon)
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

#if defined(WIN32) || defined(WIN64)
	if (SYSTEM_MEMORY != m_enMemType)
	{
		mfxHDL pHdl = NULL;

		poIMFXCommon->GetDeviceHandle(&pHdl);

		m_pD3DDevice = (IDirect3DDevice9Ex *)pHdl;
	}
	else
	{
		m_pD3DDevice = NULL;
	}
#endif
	m_InitIsCalled = false;

	memset(&m_mfxResponse, 0, sizeof(m_mfxResponse));
	m_syncp = NULL;
	MSDK_ZERO_MEMORY(m_mfxBS);
	m_nIndex = 0;

	m_pmfxSurfaces = NULL;
	m_numSurfaces = 0;
	m_pmfxENC = NULL;

	memset(&m_mfxEncParams, 0, sizeof(m_mfxEncParams));
	m_enEncodeSurfaceUsage = NOT_USE_ENCODE_SURFACE;
}

CIMFXEncoder::~CIMFXEncoder()
{
	Close();
}

IMFX_STS CIMFXEncoder::Init(ENCODE_INIT_PARAMS_S *pstEncodeInitParams, ENCODE_SURFACE_USAGE_E enEncodeSurfaceUsage)
{
	MSDK_CHECK_POINTER(pstEncodeInitParams, IMFX_ERR_NULL_PTR);

	mfxStatus sts = MFX_ERR_NONE;

	if (true == m_InitIsCalled)
	{
		return IMFX_ERR_REINITIALIZE;
	}

	if ((0 == pstEncodeInitParams->ulFrameRate) || (0 == pstEncodeInitParams->ulHeight) || (0 == pstEncodeInitParams->ulWidth))
	{
		return IMFX_ERR_UNSUPPORTED;
	}

	sts = InitMfxEncParams(pstEncodeInitParams);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	m_pmfxENC = new MFXVideoENCODE(*m_pmfxSession);
	MSDK_CHECK_POINTER(m_pmfxENC, IMFX_ERR_MEMORY_ALLOC);

	mfxVersion version;     // real API version with which library is initialized
	IMFX_STS ists = m_poIMFXCommon->QueryVersion(version); // get real API version of the loaded library
	if (IMFX_ERR_NONE != ists)
	{
		return ists;
	}

	if ((IMFX_CODEC_JPEG == pstEncodeInitParams->enDstCodecID) && !CheckVersion(&version, MSDK_FEATURE_JPEG_ENCODE)) {
		msdk_printf(MSDK_STRING("error: Jpeg is not supported in the %d.%d API version\n"),
			version.Major, version.Minor);
		return IMFX_ERR_UNSUPPORTED;
	}

	// Validate video encode parameters (optional)
	// - In this example the validation result is written to same structure
	// - MFX_WRN_INCOMPATIBLE_VIDEO_PARAM is returned if some of the video parameters are not supported,
	//   instead the encoder will select suitable parameters closest matching the requested configuration
	sts = m_pmfxENC->Query(&m_mfxEncParams, &m_mfxEncParams);
	MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	m_enEncodeSurfaceUsage = enEncodeSurfaceUsage;
	if (USE_ENCODE_SURFACE == m_enEncodeSurfaceUsage)
	{
		sts = AllocSurfaces();
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);
	}

	// Initialize the Media SDK encoder
	sts = m_pmfxENC->Init(&m_mfxEncParams);
	MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	// Retrieve video parameters selected by encoder.
	// - BufferSizeInKB parameter is required to set bit stream buffer size
	mfxVideoParam par;
	memset(&par, 0, sizeof(par));
	sts = m_pmfxENC->GetVideoParam(&par);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	mfxU32 nEncodedDataBufferSize = m_mfxEncParams.mfx.FrameInfo.Width * m_mfxEncParams.mfx.FrameInfo.Height * 4;
	m_mfxBS.DataOffset = 0;
	m_mfxBS.DataLength = 0;
	sts = InitMfxBitstream(&m_mfxBS, nEncodedDataBufferSize);
	MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, (IMFX_STS)sts, WipeMfxBitstream(&m_mfxBS));

	m_InitIsCalled = true;

	return IMFX_ERR_NONE;
}

mfxStatus CIMFXEncoder::AllocSurfaces()
{
	mfxStatus sts = MFX_ERR_NONE;

	// Query number of required surfaces for encoder
	mfxFrameAllocRequest EncRequest;
	memset(&EncRequest, 0, sizeof(EncRequest));
	sts = m_pmfxENC->QueryIOSurf(&m_mfxEncParams, &EncRequest);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	//EncRequest.Type |= WILL_WRITE; // This line is only required for Windows DirectX11 to ensure that surfaces can be written to by the application

	// Allocate required surfaces
	// mfxFrameAllocResponse mfxResponse;
	sts = m_pGeneralAllocator->Alloc(m_pGeneralAllocator->pthis, &EncRequest, &m_mfxResponse);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	m_numSurfaces = m_mfxResponse.NumFrameActual;

	//IMFX_DBG("surface number: %d\n", m_numSurfaces);

	// Allocate surface headers (mfxFrameSurface1) for encoder
	m_pmfxSurfaces = new mfxFrameSurface1 [m_numSurfaces];
	MSDK_CHECK_POINTER(m_pmfxSurfaces, MFX_ERR_MEMORY_ALLOC);

	for (int i = 0; i < m_numSurfaces; i++)
	{
		memset(&m_pmfxSurfaces[i], 0, sizeof(mfxFrameSurface1));
		MSDK_MEMCPY_VAR(m_pmfxSurfaces[i].Info, &(m_mfxEncParams.mfx.FrameInfo), sizeof(mfxFrameInfo));

		if (SYSTEM_MEMORY == m_enMemType)
		{
			sts = m_pGeneralAllocator->Lock(m_pGeneralAllocator->pthis, m_mfxResponse.mids[i], &(m_pmfxSurfaces[i].Data));
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
		}
		else
		{
			m_pmfxSurfaces[i].Data.MemId = m_mfxResponse.mids[i];      // MID (memory id) represent one video NV12 surface
		}
	}

	return MFX_ERR_NONE;
}

IMFX_STS CIMFXEncoder::RunEncoding( mfxFrameSurface1 *pmfxSurfaceIn, FRAME_DATA_S *pstFrameDataOut )
{
	mfxStatus sts = MFX_ERR_NONE;
	MSDK_CHECK_POINTER(pmfxSurfaceIn, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pstFrameDataOut, IMFX_ERR_NULL_PTR);
	
	memset(pstFrameDataOut, 0, sizeof(FRAME_DATA_S));

	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	m_mfxBS.DataLength = 0;

	mfxFrameSurface1 *pSurfaceIn = NULL;
#if 0
	if (USE_ENCODE_SURFACE == m_enEncodeSurfaceUsage)
	{
		mfxFrameSurface1* pEncSurfaceIn = GetFreeSurfaceIn();
		if (NULL == pEncSurfaceIn)
		{
			return IMFX_ERR_MORE_SURFACE;
		}
		directxMemId* dxMemIdIn = (directxMemId*)pmfxSurfaceIn->Data.MemId;
		directxMemId* dxMemIdENCIn = (directxMemId*)pEncSurfaceIn->Data.MemId;

		if (dxMemIdIn != dxMemIdENCIn)
		{
			HRESULT hr = m_pD3DDevice->StretchRect(dxMemIdIn->m_surface, NULL, dxMemIdENCIn->m_surface, NULL, D3DTEXF_LINEAR);
			if (FAILED(hr))
			{
				return IMFX_ERR_UNKNOWN;
			}
			pSurfaceIn = pEncSurfaceIn;
		}
		else
		{
			pSurfaceIn = pmfxSurfaceIn;
		}
	}
	else 
	{
		pSurfaceIn = pmfxSurfaceIn;
	}
#else
	pSurfaceIn = pmfxSurfaceIn;
#endif

	do
	{
		sts = m_pmfxENC->EncodeFrameAsync(NULL, pSurfaceIn, &m_mfxBS, &m_syncp);

		if (MFX_ERR_NONE < sts && !m_syncp) // repeat the call if warning and no output
		{
			if (MFX_WRN_DEVICE_BUSY == sts)
				MSDK_SLEEP(1); // wait if device is busy
		}
		else if (MFX_ERR_NONE < sts && m_syncp)
		{
			sts = MFX_ERR_NONE; // ignore warnings if output is available
			break;
		}
		else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
		{
			sts = AllocateSufficientBuffer(&m_mfxBS);
			MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);
		}
		else if (MFX_ERR_MORE_DATA == sts)
		{
			break;  //enter next step
		}
		else
		{
			// get next surface and new task for 2nd bitstream in ViewOutput mode
			MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_BITSTREAM);
			break;
		}
	} while (1);

	if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
	{
		return IMFX_ERR_NOT_ENOUGH_BUFFER;
	}

	if ((MFX_ERR_NONE == sts) && m_syncp)
	{
		;//output is avaliable
	}
	else
	{
		for (;;)
		{
			sts = m_pmfxENC->EncodeFrameAsync(NULL, NULL, &m_mfxBS, &m_syncp);

			if (MFX_ERR_NONE < sts && !m_syncp) // repeat the call if warning and no output
			{
				if (MFX_WRN_DEVICE_BUSY == sts)
					MSDK_SLEEP(1); // wait if device is busy
			}
			else if (MFX_ERR_NONE < sts && m_syncp)
			{
				sts = MFX_ERR_NONE; // ignore warnings if output is available
				break;
			}
			else if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)  //表明调用者传入的缓冲区不够大。
			{
				msdk_printf(MSDK_STRING("pstOutFrameData->ulDataSize is small!\n"));
				break;
				//sts = AllocateSufficientBuffer(&mfxBS);
				//MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
			}
			else if (MFX_ERR_MORE_DATA == sts)  //需要更多的输入数据，表明之前输入的数据已经完成解码，跳出循环。
			{
				sts = MFX_ERR_NONE;
				break;
			}
			else
			{
				// get new task for 2nd bitstream in ViewOutput mode
				MSDK_IGNORE_MFX_STS(sts, MFX_ERR_MORE_BITSTREAM);
				break;
			}
		}

		if (MFX_ERR_NOT_ENOUGH_BUFFER == sts)
		{
			return IMFX_ERR_NOT_ENOUGH_BUFFER;
		}
	}

	/* sync all data */
	if (MFX_ERR_NONE == sts)
	{
		if (NULL == m_syncp)
		{
			return IMFX_ERR_NULL_PTR;
		}

		sts = m_pmfxSession->SyncOperation(m_syncp, 60000);
		if (MFX_ERR_NONE == sts)
		{
			pstFrameDataOut->ulDataSize = m_mfxBS.DataLength;
			pstFrameDataOut->pcDataAddr = (mfx_i8 *)m_mfxBS.Data;
			pstFrameDataOut->enCodecID = (CODEC_ID_E)m_mfxEncParams.mfx.CodecId;
			pstFrameDataOut->ulDataOffset = 0;
		}
	}

	return IMFX_STS(sts);
}

IMFX_STS CIMFXEncoder::Close()
{
	if (NULL != m_pGeneralAllocator)
	{
		MSDK_SAFE_DELETE_ARRAY(m_pmfxSurfaces);

		m_pGeneralAllocator->Free(m_pGeneralAllocator->pthis, &m_mfxResponse);
	}
	m_pGeneralAllocator = NULL;
	m_numSurfaces = 0;

	if (m_pmfxENC)
	{
		m_pmfxENC->Close();
		delete m_pmfxENC;
		m_pmfxENC = NULL;
	}

	memset(&m_mfxEncParams, 0, sizeof(m_mfxEncParams));
	m_enEncodeSurfaceUsage = NOT_USE_ENCODE_SURFACE;
	m_InitIsCalled = false;

	m_pmfxSession = NULL;
	memset(&m_mfxResponse, 0, sizeof(m_mfxResponse));
	m_syncp = NULL;
	WipeMfxBitstream(&m_mfxBS);
	MSDK_ZERO_MEMORY(m_mfxBS);
	m_nIndex = 0;

	return IMFX_ERR_NONE;
}

mfxFrameSurface1* CIMFXEncoder::GetFreeSurfaceIn()
{
	if (NOT_USE_ENCODE_SURFACE == m_enEncodeSurfaceUsage)
	{
		return NULL;
	}

	m_nIndex = GetFreeSurface(m_pmfxSurfaces, m_numSurfaces);
	if (MFX_ERR_NOT_FOUND == m_nIndex)
	{
		return NULL;
	}
	else
	{
		return &m_pmfxSurfaces[m_nIndex];
	}
}

mfxStatus CIMFXEncoder::AllocateSufficientBuffer( mfxBitstream* pBS )
{
	MSDK_CHECK_POINTER(pBS, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(m_pmfxENC, MFX_ERR_NOT_INITIALIZED);

	mfxVideoParam par;
	MSDK_ZERO_MEMORY(par);

	// find out the required buffer size
	mfxStatus sts = m_pmfxENC->GetVideoParam(&par);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// reallocate bigger buffer for output
	sts = ExtendMfxBitstream(pBS, par.mfx.BufferSizeInKB * 1000);
	MSDK_CHECK_RESULT_SAFE(sts, MFX_ERR_NONE, sts, WipeMfxBitstream(pBS));

	return MFX_ERR_NONE;
}

mfxStatus CIMFXEncoder::InitMfxEncParams(ENCODE_INIT_PARAMS_S *pstEncodeInitParams)
{
	m_mfxEncParams.mfx.CodecId = pstEncodeInitParams->enDstCodecID;

	if ((BEST_QUALITY != pstEncodeInitParams->enTargetUsage) && (BEST_SPEED != pstEncodeInitParams->enTargetUsage))
	{
		pstEncodeInitParams->enTargetUsage = BALANCED;
	}

	m_mfxEncParams.mfx.TargetUsage = pstEncodeInitParams->enTargetUsage;

	pstEncodeInitParams->ulBitRate /= 1024; // convert bps to kbps
	if (0 == pstEncodeInitParams->ulBitRate)
	{
		pstEncodeInitParams->ulBitRate = CalculateDefaultBitrate(m_mfxEncParams.mfx.CodecId,
			m_mfxEncParams.mfx.TargetUsage,
			pstEncodeInitParams->ulWidth,
			pstEncodeInitParams->ulHeight,
			pstEncodeInitParams->ulFrameRate);
	}

	m_mfxEncParams.mfx.TargetKbps = (mfxU16)pstEncodeInitParams->ulBitRate;

	m_mfxEncParams.mfx.RateControlMethod = MFX_RATECONTROL_VBR;
	//m_mfxEncParams.mfx.FrameInfo.FrameRateExtN = pstEncodeInitParams->ulFrameRate;
	//m_mfxEncParams.mfx.FrameInfo.FrameRateExtD = pstEncodeInitParams->ulFrameRate;
	ConvertFrameRate(pstEncodeInitParams->ulFrameRate, &m_mfxEncParams.mfx.FrameInfo.FrameRateExtN, &m_mfxEncParams.mfx.FrameInfo.FrameRateExtD);

	// specify memory type
	m_mfxEncParams.IOPattern = (mfxU16)((m_enMemType != SYSTEM_MEMORY) ? MFX_IOPATTERN_IN_VIDEO_MEMORY : MFX_IOPATTERN_IN_SYSTEM_MEMORY);

	if ((IMFX_FOURCC_NV12 == pstEncodeInitParams->enSrcCodecID) || (IMFX_FOURCC_YV12  == pstEncodeInitParams->enSrcCodecID))
	{
		m_mfxEncParams.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
		m_mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
	}
	else if (IMFX_FOURCC_BGR4 == pstEncodeInitParams->enSrcCodecID)
	{
		m_mfxEncParams.mfx.FrameInfo.FourCC = MFX_FOURCC_RGB4;
		m_mfxEncParams.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV444;
	}

	m_mfxEncParams.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
	m_mfxEncParams.mfx.FrameInfo.CropX = 0;
	m_mfxEncParams.mfx.FrameInfo.CropY = 0;
	m_mfxEncParams.mfx.FrameInfo.CropW = (mfxU16)pstEncodeInitParams->ulWidth;     // Half the resolution of decode stream
	m_mfxEncParams.mfx.FrameInfo.CropH = (mfxU16)pstEncodeInitParams->ulHeight;
	// width must be a multiple of 16
	// height must be a multiple of 16 in case of frame picture and a multiple of 32 in case of field picture
	m_mfxEncParams.mfx.FrameInfo.Width = MSDK_ALIGN16(m_mfxEncParams.mfx.FrameInfo.CropW);
	m_mfxEncParams.mfx.FrameInfo.Height =
		(MFX_PICSTRUCT_PROGRESSIVE == m_mfxEncParams.mfx.FrameInfo.PicStruct) ?
		MSDK_ALIGN16(m_mfxEncParams.mfx.FrameInfo.CropH) :
		MSDK_ALIGN32(m_mfxEncParams.mfx.FrameInfo.CropH);

	// Configure Media SDK to keep more operations in flight
	// - AsyncDepth represents the number of tasks that can be submitted, before synchronizing is required
	m_mfxEncParams.AsyncDepth = 1;  //mfxDecParams.AsyncDepth;

	// JPEG encoder settings overlap with other encoders settings in mfxInfoMFX structure
	if (IMFX_CODEC_JPEG == pstEncodeInitParams->enDstCodecID)
	{
		//m_mfxEncParams.mfx.TargetUsage = 0;
		//m_mfxEncParams.mfx.TargetKbps = 0;
		//m_mfxEncParams.mfx.CodecProfile = 0;
		m_mfxEncParams.mfx.Interleaved = 1;
		m_mfxEncParams.mfx.Quality = pstEncodeInitParams->ulJPEGQuality;
		m_mfxEncParams.mfx.RestartInterval = 0;
		MSDK_ZERO_MEMORY(m_mfxEncParams.mfx.reserved5);
	}
	else
	{
		m_mfxEncParams.mfx.GopOptFlag = MFX_GOP_STRICT; //严格按照指定的gopPicSize
		m_mfxEncParams.mfx.GopPicSize = (mfxU16)pstEncodeInitParams->ulGopPicSize;  //group of pictures,一个图像组的图像数目。{I PPP} {I PPP} size = 4;
		m_mfxEncParams.mfx.GopRefDist = 1;  //没有B帧
		m_mfxEncParams.mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE; // baseline
		m_mfxEncParams.mfx.NumRefFrame = 1; //表示一个GOP只有一个I帧
		m_mfxEncParams.mfx.IdrInterval = 0; //每一个I帧都是IDR参考帧
	}

	return MFX_ERR_NONE;
}
