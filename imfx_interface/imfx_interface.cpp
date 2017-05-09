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
#include "imfx_interface.h"


mfx_hdl IMFX_init(CODEC_ID_E src_codec_id, CODEC_ID_E dst_codec_id, mfx_hdl window, IMFX_STS *error_code)
{
	*error_code = IMFX_ERR_NONE;

	CIMFX_interface *imfx = new CIMFX_interface;

	IMFX_STS sts;
	sts = imfx->InitCommon(src_codec_id, dst_codec_id, window);
	if (IMFX_ERR_NONE != sts)
	{
		delete imfx;
		*error_code = sts;
		return NULL;
	}



	return (mfx_hdl)imfx;
}

mfx_hdl IMFX_reset( mfx_hdl hdl, CODEC_ID_E src_codec_id, CODEC_ID_E dst_codec_id, mfx_hdl window, IMFX_STS *error_code )
{
	IMFX_destory(hdl);

	hdl = IMFX_init(src_codec_id, dst_codec_id, window, error_code);

	return hdl;
}

IMFX_STS IMFX_destory( mfx_hdl hdl )
{
	CIMFX_interface *imfx = (CIMFX_interface *)hdl;
	MSDK_SAFE_DELETE(imfx);

	return IMFX_ERR_NONE;
}

IMFX_STS IMFX_decode(mfx_hdl hdl, FRAME_DATA_S *frame_data, mfx_u32 dst_width, mfx_u32 dst_height)
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data->pcDataAddr, IMFX_ERR_NULL_PTR);

	if (frame_data->ulDataSize == 0)
	{
		return IMFX_ERR_INVALID_PARAM;
	}

	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	bool re_init_vpp = false;

	IMFX_STS sts = IMFX_ERR_NONE;

	if (NULL == imfx->m_d)
	{
		sts = imfx->ResetDecode(imfx->m_src_codec_id, frame_data);
		if (IMFX_ERR_NONE != sts)
		{
			return sts;
		}
	}
	else
	{
		imfx->m_d->PutBackOutputSurface();
	}

	msdk_tick m_tStartTime;
	msdk_tick m_tEndTime;
	m_tStartTime = msdk_time_get_tick();
	sts = imfx->m_d->RunDecoding(frame_data);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	if (imfx->m_d->GetOutputSurfaceCount() <= 0)
	{
		return IMFX_ERR_MORE_DATA;
	}

	mfxFrameSurface1 *surfaceDecOut;
	imfx->m_d->GetOutputSurface(&surfaceDecOut);
	imfx->surface4render = surfaceDecOut;

	sts = imfx->Render();
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}
	m_tEndTime = msdk_time_get_tick();
	
	return IMFX_ERR_NONE;
}


IMFX_STS IMFX_decode_to_system(mfx_hdl hdl, FRAME_DATA_S *frame_data, mfx_u32 dst_width, mfx_u32 dst_height, RAW_DATA_S *raw_data)
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data->pcDataAddr, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(raw_data, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(raw_data->pcDataAddr, IMFX_ERR_NULL_PTR);

	if ((frame_data->ulDataSize == 0) || (raw_data->ulMaxLength <= 0))
	{
		return IMFX_ERR_INVALID_PARAM;
	}

	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	raw_data->ulDataSize = 0;

	bool re_init_vpp = false;

	IMFX_STS sts = IMFX_ERR_NONE;

	if (NULL == imfx->m_d)
	{
		sts = imfx->ResetDecode(imfx->m_src_codec_id, frame_data);
		if (IMFX_ERR_NONE != sts)
		{
			return sts;
		}
	}
	else
	{
		imfx->m_d->PutBackOutputSurface();
	}

	msdk_tick m_tStartTime;
	msdk_tick m_tEndTime;
	m_tStartTime = msdk_time_get_tick();
	sts = imfx->m_d->RunDecoding(frame_data);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	if (imfx->m_d->GetOutputSurfaceCount() <= 0)
	{
		return IMFX_ERR_MORE_DATA;
	}

	mfxFrameSurface1 *surfaceDecOut;
	imfx->m_d->GetOutputSurface(&surfaceDecOut);
	imfx->surface4render = surfaceDecOut;

	m_tEndTime = msdk_time_get_tick();
	//printf("decode time: %6.3f ms\n", (m_tEndTime - m_tStartTime) * 1000 / (double)msdk_time_get_frequency());
	m_tStartTime = msdk_time_get_tick();

	sts = imfx->ResetVpp(imfx->m_dst_codec_id, true, 1, dst_height, dst_width);
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		return sts;
	}

	sts = imfx->m_v->RunVPP(surfaceDecOut);
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		return sts;
	}

	if (imfx->m_v->GetOutputSurfaceCount() <= 0)
	{
		imfx->m_d->PutBackOutputSurface();
		return IMFX_ERR_MORE_DATA;
	}

	mfxFrameSurface1 *surfaceVppOut;
	imfx->m_v->GetOutputSurface(&surfaceVppOut);
	m_tEndTime = msdk_time_get_tick();
	//printf("vpp time: %6.3f ms\n", (m_tEndTime - m_tStartTime) * 1000 / (double)msdk_time_get_frequency());
	m_tStartTime = msdk_time_get_tick();
	sts = imfx->m_c->LockSurface(surfaceVppOut);
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		imfx->m_v->PutBackOutputSurface();
		return sts;
	}

	if (IMFX_FOURCC_RGB4 == imfx->m_dst_codec_id)
	{
		sts = copyFromRGB32ToRGB32(surfaceVppOut, raw_data);
	}
	else if (IMFX_FOURCC_YV12 == imfx->m_dst_codec_id)
	{
		YV12_DATA_S yv12;
		memset(&yv12, 0, sizeof(YV12_DATA_S));
		yv12.pcDataAddr = raw_data->pcDataAddr;
		yv12.ulMaxLength = raw_data->ulMaxLength;
		sts = CopyFromNV12ToYV12(surfaceVppOut, &yv12);

		raw_data->FourCC = IMFX_FOURCC_YV12;
		raw_data->ulDataSize = yv12.ulDataSize;
		raw_data->ulWidth = yv12.ulWidth;
		raw_data->ulHeight = yv12.ulHeight;
		raw_data->ulPitch = yv12.ulPitch;
	}
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		imfx->m_v->PutBackOutputSurface();
		return sts;
	}

	imfx->m_c->UnlockSurface(surfaceVppOut);
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		imfx->m_v->PutBackOutputSurface();
		return sts;
	}
	imfx->m_v->PutBackOutputSurface();
	m_tEndTime = msdk_time_get_tick();
	//printf("copy time: %6.3f ms\n", (m_tEndTime - m_tStartTime) * 1000 / (double)msdk_time_get_frequency());
	return IMFX_ERR_NONE;
}

IMFX_STS IMFX_decode_and_buildBack_to_system(mfx_hdl hdl, const char *pcOpenClFile, FRAME_DATA_S *frame_data, mfx_u32 dst_width, mfx_u32 dst_height, RAW_DATA_S *raw_data)
{
#ifdef USE_PLUGIN
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data->pcDataAddr, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(raw_data, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(raw_data->pcDataAddr, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pcOpenClFile, IMFX_ERR_NULL_PTR);

	if ((frame_data->ulDataSize == 0) || (raw_data->ulMaxLength <= 0))
	{
		return IMFX_ERR_INVALID_PARAM;

	}
	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	raw_data->ulDataSize = 0;

	bool re_init_vpp = false;

	IMFX_STS sts = IMFX_ERR_NONE;
	if (NULL == imfx->m_d)
	{
		sts = imfx->ResetDecode(imfx->m_src_codec_id, frame_data);
		if (IMFX_ERR_NONE != sts)
		{
			return sts;
		}
	}
	else
	{
		imfx->m_d->PutBackOutputSurface();
	}

	sts = imfx->m_d->RunDecoding(frame_data);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	if (imfx->m_d->GetOutputSurfaceCount() <= 0)
	{
		return IMFX_ERR_MORE_DATA;
	}

	mfxFrameSurface1 *surfaceDecOut;
	imfx->m_d->GetOutputSurface(&surfaceDecOut);
	imfx->surface4render = surfaceDecOut;
	if (NULL == imfx->m_v)
	{
		sts = imfx->ResetVpp(imfx->m_dst_codec_id, false, 1,  surfaceDecOut->Info.Width / 4, surfaceDecOut->Info.Height / 4 );
		if (IMFX_ERR_NONE != sts)
		{
			imfx->m_d->PutBackOutputSurface();
			return sts;
		}
	}

	sts = imfx->m_v->RunVPP(surfaceDecOut);
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		return sts;
	}

	if (imfx->m_v->GetOutputSurfaceCount() <= 0)
	{
		imfx->m_d->PutBackOutputSurface();
		return IMFX_ERR_MORE_DATA;
	}

	mfxFrameSurface1 *surfaceVppOut;
	imfx->m_v->GetOutputSurface(&surfaceVppOut);
	//sts = imfx->m_c->LockSurface(surfaceVppOut);
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		imfx->m_v->PutBackOutputSurface();
		return sts;
	}

	/* 将VPP的surface进行Plug操作 */
	if (NULL == imfx->m_p)
	{
		sts = imfx->ResetPlugin(MSDK_STRING("plugin.bin"));
		if (IMFX_ERR_NONE != sts)
		{
			msdk_printf(MSDK_STRING("init vpp failed: %d\n"), sts);
		}
	}

	mfxFrameSurface1 *PluginSurfaceOut = NULL;
	if (NULL != imfx->m_p)
	{
		sts = imfx->m_p->RunPlugin(surfaceVppOut, &PluginSurfaceOut);
		if (IMFX_ERR_NONE != sts)
		{
			msdk_printf(MSDK_STRING("run plugin failed: %d\n"), sts);
		}
	}
	imfx->m_c->LockSurface(PluginSurfaceOut);

	/* 保存Y向量
	FILE * pfile = NULL;
	fopen_s(&pfile, "d:\\Test.y", "wb");
	if (NULL != pfile)
	{
	for(int iHeight = 0;  iHeight < PluginSurfaceOut->Info.CropH; iHeight++)
	{
	fwrite(PluginSurfaceOut->Data.Y + iHeight * PluginSurfaceOut->Data.Pitch,  PluginSurfaceOut->Info. CropW,  1, pfile);
	}

	fclose(pfile);
	}*/

	mfxFrameInfo *pInfo = &PluginSurfaceOut->Info;
	mfxFrameData *pData = &PluginSurfaceOut->Data;
	MSDK_CHECK_POINTER(pData, IMFX_ERR_NULL_PTR);
	mfxU32 i, j, h, w;

	/* Y */
	int datasize = 0;
	for (i = 0; i < pInfo->CropH; i++)
	{
		memcpy(raw_data->pcDataAddr + datasize, pData->Y + (pInfo->CropY * pData->Pitch + pInfo->CropX) + i * pData->Pitch, pInfo->CropW);
		datasize += pInfo->CropW;
	}
	raw_data->ulDataSize = datasize;
	raw_data->ulHeight = pInfo->CropH;
	raw_data->ulWidth = pInfo->CropW;

	imfx->m_c->UnlockSurface(PluginSurfaceOut);
	if (IMFX_ERR_NONE != sts)
	{
		imfx->m_d->PutBackOutputSurface();
		imfx->m_v->PutBackOutputSurface();
		return sts;
	}
	imfx->m_v->PutBackOutputSurface();
	return IMFX_ERR_NONE;
#else
	return IMFX_ERR_UNSUPPORTED;
#endif
}


IMFX_STS IMFX_display(mfx_hdl hdl)
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);

	CIMFX_interface *imfx = (CIMFX_interface *)hdl;
	IMFX_STS sts = imfx->Render();

	return sts;
}

IMFX_STS IMFX_encode( mfx_hdl hdl, RAW_DATA_S *raw_data, CODEC_ID_E dst_codec_id, FRAME_DATA_S *frame_data )
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(raw_data, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(raw_data->pcDataAddr, IMFX_ERR_NULL_PTR);

	if (raw_data->ulDataSize == 0)
	{
		return IMFX_ERR_INVALID_PARAM;
	}

	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	IMFX_STS sts;

	sts = imfx->ResetEncode(raw_data->FourCC, dst_codec_id, raw_data->ulWidth, raw_data->ulHeight);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	mfxFrameSurface1 *surface = imfx->m_e->GetFreeSurfaceIn();
	if (NULL == surface)
	{
		return IMFX_ERR_MORE_SURFACE;
	}

	sts = imfx->m_c->LockSurface(surface);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}
	sts = CopyFromRawData2Surface(raw_data, surface);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}
	sts = imfx->m_c->UnlockSurface(surface);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	sts = imfx->m_e->RunEncoding(surface, frame_data);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	return IMFX_ERR_NONE;
}

IMFX_STS IMFX_encode2JpgFile(mfx_hdl hdl, char *pcJpgFileName, CODEC_ID_E dst_codec_id)
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(pcJpgFileName, IMFX_ERR_NULL_PTR);

	IMFX_STS sts;
	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	if (NULL == imfx->m_e)
	{
		try
		{
			imfx->m_e = new CIMFXEncoder(imfx->m_c);
		}
		catch (std::bad_alloc &)
		{
			imfx->m_e = NULL;
			return IMFX_ERR_MEMORY_ALLOC;
		}
		catch (IMFX_STS sts)
		{
			return sts;
		}

		ENCODE_INIT_PARAMS_S params;
		MSDK_ZERO_MEMORY(params);
		params.enDstCodecID = IMFX_CODEC_JPEG;
		params.enSrcCodecID = IMFX_FOURCC_NV12;
		params.enTargetUsage = BALANCED;
		params.ulBitRate = 0;
		params.ulFrameRate = 25;
		params.ulHeight = imfx->surface4render->Info.CropH;
		params.ulWidth = imfx->surface4render->Info.CropW;
		params.ulJPEGQuality = 75;

		IMFX_STS sts = imfx->m_e->Init(&params, NOT_USE_ENCODE_SURFACE);
		if (IMFX_ERR_NONE != sts)
		{
			MSDK_SAFE_DELETE(imfx->m_e);
			return sts;
		}
	}

	FRAME_DATA_S stFrameData;
	sts = imfx->m_e->RunEncoding(imfx->surface4render, &stFrameData);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}


	FILE *pJpgFile = NULL;
	fopen_s(&pJpgFile, pcJpgFileName, "wb");
	if (NULL != pJpgFile)
	{
		fwrite(stFrameData.pcDataAddr, 1, stFrameData.ulDataSize, pJpgFile);
		fclose(pJpgFile);
	}
	return IMFX_ERR_NONE;
}

IMFX_STS IMFX_draw_polyon( mfx_hdl hdl, POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum )
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);

	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	IMFX_STS sts = imfx->SetPolyon(pstPolygonAttr, ulPolygonNum );

	return sts;
}

IMFX_STS IMFX_SnapJpegFromVideo(mfx_hdl hdl, FRAME_DATA_S *frame_data_in, mfx_u32 dst_width, mfx_u32 dst_height, FRAME_DATA_S *frame_data_out)
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data_in, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data_in->pcDataAddr, IMFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(frame_data_out, IMFX_ERR_NULL_PTR);

	if (frame_data_in->ulDataSize == 0)
	{
		return IMFX_ERR_INVALID_PARAM;
	}

	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	bool re_init_vpp = false;

	IMFX_STS sts = IMFX_ERR_NONE;

	if (NULL == imfx->m_d)
	{
		sts = imfx->ResetDecode(imfx->m_src_codec_id, frame_data_in);
		if (IMFX_ERR_NONE != sts)
		{
			return sts;
		}
	}

	sts = imfx->m_d->RunDecoding(frame_data_in);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	if (imfx->m_d->GetOutputSurfaceCount() <= 0)
	{
		return IMFX_ERR_MORE_DATA;
	}

	mfxFrameSurface1 *surfaceDecOut;
	imfx->m_d->GetOutputSurface(&surfaceDecOut);

	if (0 != (dst_height * dst_height))
	{
		sts = imfx->ResetVpp(imfx->m_dst_codec_id, false, 1, dst_height, dst_width);
		if (IMFX_ERR_NONE != sts)
		{
			imfx->m_d->PutBackOutputSurface();
			return sts;
		}

		sts = imfx->m_v->RunVPP(surfaceDecOut);
		if (IMFX_ERR_NONE != sts)
		{
			imfx->m_d->PutBackOutputSurface();
			return sts;
		}

		if (imfx->m_v->GetOutputSurfaceCount() <= 0)
		{
			imfx->m_d->PutBackOutputSurface();
			return IMFX_ERR_MORE_DATA;
		}

		imfx->m_v->GetOutputSurface(&surfaceDecOut);
	}


	sts = imfx->ResetEncode(IMFX_FOURCC_NV12, IMFX_CODEC_JPEG, surfaceDecOut->Info.CropW, surfaceDecOut->Info.CropH);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	sts = imfx->m_e->RunEncoding(surfaceDecOut, frame_data_out);
	if (IMFX_ERR_NONE != sts)
	{
		return sts;
	}

	return sts;
}

IMFX_STS IMFX_displayEx(mfx_hdl hdl, RAW_DATA_S *raw_data)
{
	MSDK_CHECK_POINTER(hdl, IMFX_ERR_NULL_PTR);

	CIMFX_interface *imfx = (CIMFX_interface *)hdl;

	bool  ret = imfx->m_rendEx->Render(raw_data);

	return IMFX_ERR_NONE;
}