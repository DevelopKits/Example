#ifndef __IMFX_INTERFACE_H__
#define __IMFX_INTERFACE_H__

#include "imfx_defs.h"

#define USE_PLUGIN

#ifdef  __cplusplus
extern "C"{
#endif

mfx_hdl IMFX_init(CODEC_ID_E src_codec_id, CODEC_ID_E dst_codec_id, mfx_hdl window, IMFX_STS *error_code);

IMFX_STS IMFX_destory(mfx_hdl hdl);

mfx_hdl IMFX_reset( mfx_hdl hdl, CODEC_ID_E src_codec_id, CODEC_ID_E dst_codec_id, mfx_hdl window, IMFX_STS *error_code );

IMFX_STS IMFX_decode_to_system(mfx_hdl hdl, FRAME_DATA_S *frame_data, mfx_u32 dst_width, mfx_u32 dst_height, RAW_DATA_S *raw_data);

IMFX_STS IMFX_decode_and_buildBack_to_system(mfx_hdl hdl, const char* pcOpenClFile, FRAME_DATA_S *frame_data, mfx_u32 dst_width, mfx_u32 dst_height, RAW_DATA_S *raw_data);

IMFX_STS IMFX_encode(mfx_hdl hdl, RAW_DATA_S *raw_data, CODEC_ID_E dst_codec_id, FRAME_DATA_S *frame_data);

IMFX_STS IMFX_encode2JpgFile(mfx_hdl hdl, char * pcJpgFileName, CODEC_ID_E dst_codec_id);

IMFX_STS IMFX_display(mfx_hdl hdl);

IMFX_STS IMFX_displayEx(mfx_hdl hdl, RAW_DATA_S *raw_data);

IMFX_STS IMFX_draw_polyon(mfx_hdl hdl, POLYGON_ATTR_S *pstPolygonAttr, mfx_u32 ulPolygonNum);

IMFX_STS IMFX_SnapJpegFromVideo(mfx_hdl hdl, FRAME_DATA_S *frame_data_in, mfx_u32 dst_width, mfx_u32 dst_height, FRAME_DATA_S *frame_data_out);

IMFX_STS IMFX_decode(mfx_hdl hdl, FRAME_DATA_S *frame_data, mfx_u32 dst_width, mfx_u32 dst_height);


#ifdef  __cplusplus
}
#endif /* end of __cplusplus */

#endif