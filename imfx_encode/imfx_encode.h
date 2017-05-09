#ifndef __IMFX_ENCODE_H__
#define __IMFX_ENCODE_H__

typedef enum tagEncodeSurfaceUsage
{
	NOT_USE_ENCODE_SURFACE  = 0,    /**< won't use encode surface */
	USE_ENCODE_SURFACE      = 1     /**< will use encoder's surface */
}ENCODE_SURFACE_USAGE_E;

typedef struct tagEncodeInitParams
{
	CODEC_ID_E enSrcCodecID;        /**< only support nv12 color format */
	mfx_u32 ulWidth;
	mfx_u32 ulHeight;

	CODEC_ID_E enDstCodecID;
	mfx_u32 ulFrameRate;
	mfx_u32 ulBitRate;

	union{
		mfx_u32 ulJPEGQuality;            /**< JPEG编码图像，质量参数。取值范围：[1, 100]，100-代表最好的质量 */
		mfx_u32 ulGopPicSize;			/**< I帧间隔 */
	};
	
	TARGET_USAGE_E enTargetUsage;   /**< 视频压缩均衡 */

}ENCODE_INIT_PARAMS_S;

class IMFX_API CIMFXEncoder
{
public:
	CIMFXEncoder(CIMFXCommon *poIMFXCommon);
	~CIMFXEncoder();

	IMFX_STS Init(ENCODE_INIT_PARAMS_S *pstEncodeInitParams, ENCODE_SURFACE_USAGE_E enDataFrom);
	IMFX_STS RunEncoding(mfxFrameSurface1 *pmfxSurfaceIn, FRAME_DATA_S *pstFrameDataOut);
	IMFX_STS Close();
	mfxFrameSurface1* GetFreeSurfaceIn();

private:
	mfxStatus AllocSurfaces();
	mfxStatus AllocateSufficientBuffer(mfxBitstream* pBS);
	mfxStatus InitMfxEncParams(ENCODE_INIT_PARAMS_S *pstEncodeInitParams);

private:
	CIMFXCommon           *m_poIMFXCommon;
	MFXVideoSession        *m_pmfxSession;
	GeneralAllocator      *m_pGeneralAllocator;
	mfxFrameAllocResponse   m_mfxResponse;
	mfxSyncPoint            m_syncp;
	MEM_TYPE_E				m_enMemType;
	mfxBitstream            m_mfxBS;
	mfxU16                  m_nIndex;   // index of free surface
	mfxFrameSurface1       *m_pmfxSurfaces;
	mfxU16                  m_numSurfaces;
	MFXVideoENCODE         *m_pmfxENC;
	mfxVideoParam           m_mfxEncParams;
	ENCODE_SURFACE_USAGE_E  m_enEncodeSurfaceUsage;
#if defined(WIN32) || defined(WIN64)
	IDirect3DDevice9Ex	   *m_pD3DDevice;
#endif

	bool                    m_InitIsCalled;
};

#endif // !__IMFX_ENCODER_H__
