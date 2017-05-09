#ifndef __INTERFACE_INNER_H__
#define __INTERFACE_INNER_H__
#include "D3D_RenderEx.h"

class CIMFX_interface
{
public:
	CIMFX_interface();

	~CIMFX_interface();

	IMFX_STS InitCommon(CODEC_ID_E src_codec_id, CODEC_ID_E dst_codec_id, mfx_hdl hWnd);

	IMFX_STS ResetDecode(CODEC_ID_E dstVideoType, FRAME_DATA_S *pstFrameData);

	IMFX_STS ResetVpp(CODEC_ID_E dstVideoType, bool Copy2SystemMemory = false, int ASyncDepth = 4, int dstW = 0, int dstH = 0);

#ifdef USE_PLUGIN
	IMFX_STS ResetPlugin(msdk_char *pcPlusPath, int srcW = 0, int srcH = 0);
#endif

	IMFX_STS ResetEncode(CODEC_ID_E srcVideoType, CODEC_ID_E dstVideoType, int width, int height);

	IMFX_STS Render();


	IMFX_STS SetPolyon(POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum);

public:
	CIMFXCommon     *m_c;
	CIMFXDecoder    *m_d;
	CIMFXVpp        *m_v;

	CIMFXEncoder    *m_e;
#ifdef USE_PLUGIN
	CIMFXPlugin        *m_p;
#endif
	CODEC_ID_E m_src_codec_id;
	CODEC_ID_E m_dst_codec_id;
	mfx_u32 m_dec_cropH;
	mfx_u32 m_dec_cropW;
	PIC_STRUCT_S m_pic_struct;
	mfx_u32 m_width;
	mfx_u32 m_height;

	mfx_u32 m_enc_width;
	mfx_u32 m_enc_height;

	mfxFrameSurface1 *surface4render;

	bool m_init_flag;
	D3D_RenderEx * m_rendEx;
};

#endif
