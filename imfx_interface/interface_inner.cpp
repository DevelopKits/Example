#include "imfx.h"
#include "imfx_common.h"
#include "imfx_decode.h"
#include "imfx_vpp.h"
#include "imfx_encode.h"
#include "imfx_interface.h"
#ifdef USE_PLUGIN
	#include "imfx_plugin.h"
#endif
#include <string>
#include "interface_inner.h"


CIMFX_interface::CIMFX_interface()
{
	m_init_flag = false;

	m_c = NULL;
	m_d = NULL;
	m_v = NULL;
	m_e = NULL;
#ifdef USE_PLUGIN
	m_p = NULL;
#endif
	m_dec_cropH = 0;
	m_dec_cropW = 0;
	m_pic_struct = PIC_STRUCT_S::IMFX_PICSTRUCT_PROGRESSIVE;
	m_width = 0;
	m_height = 0;
	m_enc_width = 0;
	m_enc_height = 0;
	surface4render = NULL;

	//p = NULL;
	//MSDK_ZERO_MEMORY(m_mfxBS);
}

CIMFX_interface::~CIMFX_interface()
{
	MSDK_SAFE_DELETE(m_d);
	MSDK_SAFE_DELETE(m_e);
	MSDK_SAFE_DELETE(m_v);

#ifdef USE_PLUGIN
	MSDK_SAFE_DELETE(m_p);
#endif
	MSDK_SAFE_DELETE(m_c);// !!! must delete after decode, encode, vpp and plugin module
	//WipeMfxBitstream(&m_mfxBS);
	//m_FileWriter.Close();
}

IMFX_STS CIMFX_interface::InitCommon(CODEC_ID_E src_codec_id, CODEC_ID_E dst_codec_id, mfx_hdl hWnd)
{
	m_src_codec_id = src_codec_id;
	m_dst_codec_id = dst_codec_id;

	if (true == m_init_flag)
	{
		return IMFX_ERR_NONE;
	}

	IMFX_STS sts;
	MSDK_SAFE_DELETE(m_c);
	m_c = new CIMFXCommon;

	mfxIMPL impl = MFX_IMPL_HARDWARE_ANY;
	mfxVersion ver = {{8, 1}};
#if 0
	sts = m_c->GetCurrentMediaSDKInfo(ver, impl);


	if ((IMFX_ERR_NONE == sts) && ((impl & MFX_IMPL_HARDWARE) || (impl & MFX_IMPL_HARDWARE_ANY)))
	{
		impl = MFX_IMPL_HARDWARE_ANY;
	}
	else
	{
		printf("WARNING use software impl!\n");
		impl = MFX_IMPL_SOFTWARE;
		ver.Major = 1;
		ver.Minor = 8;
	}

#endif
	sts = m_c->InitSession(impl, ver, D3D9_MEMORY, hWnd);
	if (IMFX_ERR_NONE != sts)
	{
		printf("WARNING: use software impl!\n");
		impl = MFX_IMPL_SOFTWARE;
		sts = m_c->InitSession(impl, ver, SYSTEM_MEMORY, hWnd);
		if (IMFX_ERR_NONE != sts)
		{
			printf("ERROR: use software impl failed with error: %d\n", sts);
			return sts;
		}
	}
	else
	{
		printf("use hardware impl");
	}


	m_init_flag = true;

	m_rendEx = new D3D_RenderEx;
	m_rendEx->InitD3D((HWND)hWnd);
	return IMFX_ERR_NONE;

	//sts = InitMfxBitstream(&m_mfxBS, 2 * 1024 * 1024);
	//MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
}

IMFX_STS CIMFX_interface::ResetDecode(CODEC_ID_E dstVideoType, FRAME_DATA_S *pstFrameData)
{
	if (false == m_init_flag)
	{
		return IMFX_ERR_UNINITIALIZED_COMMON;
	}

	IMFX_STS sts;
	MSDK_SAFE_DELETE(m_d);

	try
	{
		m_d = new CIMFXDecoder(m_c);
	}
	catch (std::bad_alloc &)
	{
		m_d = NULL;
		return IMFX_ERR_MEMORY_ALLOC;
	}
	catch (IMFX_STS sts)
	{
		return sts;
	}

	mfxInfoMFX_ext mfx_info;
	sts = CIMFXDecoder::DecodeHeader(m_src_codec_id, pstFrameData, &mfx_info);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	DECODE_INIT_PARAMS_S DecodeParams;
	MSDK_ZERO_MEMORY(DecodeParams);

	DecodeParams.enInputCodecID = dstVideoType;
	DecodeParams.enOutputCodecID = IMFX_FOURCC_NV12;
	DecodeParams.ulAsyncDepth = 1;
	DecodeParams.ulRotation = 0;
	DecodeParams.ulSurfaceOutHeight = 0;
	DecodeParams.ulSurfaceOutWidth = 0;
	DecodeParams.pstMediaInfo = &mfx_info;

	sts = m_d->Init(&DecodeParams);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	m_dec_cropH = mfx_info.FrameInfo.CropH;
	m_dec_cropW = mfx_info.FrameInfo.CropW;
	m_pic_struct = (PIC_STRUCT_S)mfx_info.FrameInfo.PicStruct;

	m_init_flag = true;
	return IMFX_ERR_NONE;
}

IMFX_STS CIMFX_interface::ResetVpp(CODEC_ID_E dstVideoType, bool Copy2SystemMemory /*= false*/, int ASyncDepth /*= 4*/, int dstW /*= 0*/, int dstH /*= 0*/)
{
	if (false == m_init_flag)
	{
		return IMFX_ERR_UNINITIALIZED_COMMON;
	}

	bool reset_vpp_flag = false;
	if (NULL != m_v)    //next time
	{
		if ((0 != dstW) && (0 != dstH)) // need to resize
		{
			if ((m_width != dstW) || (m_height != dstH))    //size changed, need to reset vpp
			{
				m_width = dstW;
				m_height = dstH;
				reset_vpp_flag = true;
			}
		}
		else
		{
			if ((m_width != m_dec_cropW) || (m_height != m_dec_cropH)) //last time the state is resize
			{
				m_width = m_dec_cropW;
				m_height = m_dec_cropH;
				reset_vpp_flag = true;
			}
		}
	}
	else //first time
	{
		if ((0 != dstW) && (0 != dstH)) //need to resize
		{
			m_width = dstW;
			m_height = dstH;
		}
		else
		{
			m_width = m_dec_cropW;
			m_height = m_dec_cropH;
		}

		reset_vpp_flag = true;
	}

	if (false == reset_vpp_flag)
	{
		return IMFX_ERR_NONE;
	}

	MSDK_SAFE_DELETE(m_v);

	try
	{
		m_v = new CIMFXVpp(m_c);
	}
	catch (std::bad_alloc &)
	{
		m_v = NULL;
		return IMFX_ERR_MEMORY_ALLOC;
	}
	catch (IMFX_STS sts)
	{
		return sts;
	}

	VPP_INIT_PARAMS_S   stVPPInitParams;
	MSDK_ZERO_MEMORY(stVPPInitParams);
	stVPPInitParams.stInVideoAttr.enCodecID = IMFX_FOURCC_NV12;
	stVPPInitParams.stInVideoAttr.ulBitRate = 0;
	stVPPInitParams.stInVideoAttr.ulFrameRate = 25;
	stVPPInitParams.stInVideoAttr.ulHeight = m_dec_cropH;
	stVPPInitParams.stInVideoAttr.ulWidth = m_dec_cropW;
	stVPPInitParams.stInVideoAttr.enPicStruct = m_pic_struct;
#if defined(_WIN32) || defined(_WIN64)
	if ((IMFX_FOURCC_YV12 == dstVideoType) || (IMFX_FOURCC_NV12 == dstVideoType))
	{
		stVPPInitParams.stOutVideoAttr.enCodecID = IMFX_FOURCC_NV12;
	}
	else if (IMFX_FOURCC_RGB4 == dstVideoType)
	{
		stVPPInitParams.stOutVideoAttr.enCodecID = IMFX_FOURCC_RGB4;
	}
#else
	stVPPInitParams.stOutVideoAttr.enCodecID = IMFX_FOURCC_RGB4;    /* linux now just support this format */
#endif
	stVPPInitParams.stOutVideoAttr.ulBitRate = 0;
	stVPPInitParams.stOutVideoAttr.ulFrameRate = 25;
	stVPPInitParams.stOutVideoAttr.ulHeight = m_height;
	stVPPInitParams.stOutVideoAttr.ulWidth = m_width;
	stVPPInitParams.stOutVideoAttr.enPicStruct = m_pic_struct;

	stVPPInitParams.ulSurfaceInWidth = 0;
	stVPPInitParams.ulSurfaceInHeight = 0;
	stVPPInitParams.stResize.mode = BY_WIDTH_HEIGHT;
	stVPPInitParams.stResize.u.s.width = m_width;
	stVPPInitParams.stResize.u.s.height = m_height;

	stVPPInitParams.ulDenoiseFactor = 0;
	stVPPInitParams.ulAsyncDepth = ASyncDepth;
	stVPPInitParams.bOutput2SystemMemory = Copy2SystemMemory;
	IMFX_STS sts = m_v->Init(&stVPPInitParams, USE_VPP_SURFACE_OUT);

	if (IMFX_ERR_NONE != sts)
	{
		MSDK_SAFE_DELETE(m_v);
		return sts;
	}

	return IMFX_ERR_NONE;
}

#ifdef USE_PLUGIN
IMFX_STS CIMFX_interface::ResetPlugin(msdk_char *pcPlusPath, int srcW /*= 0*/, int srcH /*= 0*/)
{
	VIDEO_ATTR_S video_attr;
	if (NULL != m_v)
	{
		if (IMFX_ERR_NONE != m_v->GetVideoAttr(NULL, &video_attr))
		{
			return IMFX_ERR_UNINITIALIZED;
		}
	}
	else if (NULL != m_d)
	{
		if (IMFX_ERR_NONE != m_d->GetVideoAttr(&video_attr))
		{
			return IMFX_ERR_UNINITIALIZED;
		}
	}
	else
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	MSDK_SAFE_DELETE(m_p);

	try
	{
		m_p = new CIMFXPlugin(m_c);
	}
	catch (std::bad_alloc &)
	{
		m_v = NULL;
		return IMFX_ERR_MEMORY_ALLOC;
	}
	catch (IMFX_STS sts)
	{
		return sts;
	}

	PLUGIN_INIT_PARAMS_S stPluginInitParams;
	MSDK_ZERO_MEMORY(stPluginInitParams);
	stPluginInitParams.stInVideoAttr.enCodecID = IMFX_FOURCC_NV12;
	stPluginInitParams.stInVideoAttr.ulBitRate = 0;
	stPluginInitParams.stInVideoAttr.ulFrameRate = video_attr.ulFrameRate;
	//stPluginInitParams.stInVideoAttr.ulHeight = video_attr.ulHeight;
	//stPluginInitParams.stInVideoAttr.ulWidth = video_attr.ulWidth;
	stPluginInitParams.stInVideoAttr.ulHeight = (0 == srcH) ? video_attr.ulHeight : srcH;
	stPluginInitParams.stInVideoAttr.ulWidth = (0 == srcW) ? video_attr.ulWidth : srcW;
	stPluginInitParams.stInVideoAttr.enPicStruct = video_attr.enPicStruct;
#if defined(_WIN32) || defined(_WIN64)
	stPluginInitParams.stOutVideoAttr.enCodecID = IMFX_FOURCC_NV12;
#else
	stPluginInitParams.stOutVideoAttr.enCodecID = IA_FOURCC_RGB4;
#endif
	stPluginInitParams.stOutVideoAttr.ulBitRate = 0;
	stPluginInitParams.stOutVideoAttr.ulFrameRate = video_attr.ulFrameRate;
	//stPluginInitParams.stOutVideoAttr.ulHeight = stPluginInitParams.stInVideoAttr.ulHeight;
	//stPluginInitParams.stOutVideoAttr.ulWidth = stPluginInitParams.stInVideoAttr.ulWidth;
	stPluginInitParams.stOutVideoAttr.ulHeight = (0 == srcH) ? video_attr.ulHeight : srcH;
	stPluginInitParams.stOutVideoAttr.ulWidth = (0 == srcW) ? video_attr.ulWidth : srcW;
	stPluginInitParams.stOutVideoAttr.enPicStruct = stPluginInitParams.stInVideoAttr.enPicStruct;///IA_GPU_PICSTRUCT_PROGRESSIVE;//(IA_GPU_PIC_STRUCT_S)(poPlayerCtrl->m_pmfxSurfaceOut->Info.PicStruct);
	stPluginInitParams.ulAsyncDepth = 4;

	//将插件名称传递到plus内部
	msdk_strcopy(stPluginInitParams.strPluginPath, pcPlusPath);

	if (NULL != m_v)
	{
		m_v->GetFrameInfo(NULL, &stPluginInitParams.FrameInfo);
	}
	else
	{
		m_d->GetFrameInfo(&stPluginInitParams.FrameInfo);
	}

	//stPluginInitParams.FrameInfo.Width = 512;
	//stPluginInitParams.FrameInfo.Height = 512;
	//stPluginInitParams.FrameInfo.CropW = 512;
	//stPluginInitParams.FrameInfo.CropH = 512;

	IMFX_STS lRetVal;
	lRetVal = m_p->Init(&stPluginInitParams);

	if (lRetVal != IMFX_ERR_NONE)
	{
		IMFX_ERR("plugin init failed\n");
		return lRetVal;
	}

	return IMFX_ERR_NONE;
}
#endif

IMFX_STS CIMFX_interface::ResetEncode(CODEC_ID_E srcVideoType, CODEC_ID_E dstVideoType, int width, int height)
{
	if (false == m_init_flag)
	{
		return IMFX_ERR_UNINITIALIZED_COMMON;
	}

	bool re_init_encode = false;

	if ((0 == width) || (0 == height))
	{
		return IMFX_ERR_INVALID_PARAM;
	}

	if (NULL != m_e)
	{
		if ((height != m_enc_height) || (width != m_enc_width))
		{
			m_enc_width = width;
			m_enc_height = height;
			re_init_encode = true;
		}
	}
	else
	{
		m_enc_width = width;
		m_enc_height = height;
		re_init_encode = true;
	}

	if (false == re_init_encode)
	{
		return IMFX_ERR_NONE;
	}

	MSDK_SAFE_DELETE(m_e);

	try
	{
		m_e = new CIMFXEncoder(m_c);
	}
	catch (std::bad_alloc &)
	{
		m_e = NULL;
		return IMFX_ERR_MEMORY_ALLOC;
	}
	catch (IMFX_STS sts)
	{
		return sts;
	}

	ENCODE_INIT_PARAMS_S params;
	MSDK_ZERO_MEMORY(params);
	params.enDstCodecID = dstVideoType;
	params.enSrcCodecID = srcVideoType;
	params.enTargetUsage = BALANCED;
	params.ulBitRate = 0;
	params.ulFrameRate = 25;
	params.ulHeight = m_enc_height;
	params.ulWidth = m_enc_width;
	params.ulJPEGQuality = (IMFX_CODEC_JPEG == dstVideoType) ? 75 : 0;

	IMFX_STS sts = m_e->Init(&params, USE_ENCODE_SURFACE);
	if (IMFX_ERR_NONE != sts)
	{
		MSDK_SAFE_DELETE(m_e);
		return sts;
	}

	return IMFX_ERR_NONE;
}

IMFX_STS CIMFX_interface::Render()
{
	if (false == m_init_flag)
	{
		return IMFX_ERR_UNINITIALIZED_COMMON;
	}

	if (NULL == surface4render)
	{
		return IMFX_ERR_NULL_PTR;
	}

	IMFX_STS sts = m_c->RenderFrame(surface4render);

	return sts;
}

IMFX_STS CIMFX_interface::SetPolyon(POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum)
{
	if (false == m_init_flag)
	{
		return IMFX_ERR_UNINITIALIZED_COMMON;
	}

	IMFX_STS sts = m_c->DrawPolygon(pstPolygonAttr, ulPolygonNum);

	return sts;
}
