#ifndef __IMFX_DECODE_H__
#define __IMFX_DECODE_H__

#define DEFAULT_ASYNC_DEPTH 4

typedef enum tagWorkMode
{
	SYNC,
	ASYNC
}WORK_MODE;

typedef struct tagDecodeInitParams
{
	CODEC_ID_E	enInputCodecID;     /**< ����ID */
	CODEC_ID_E  enOutputCodecID;	/**< �������ID�� h264,mpeg ֻ֧�� IMFX_FOURCC_NV12, jpeg/mjpeg ����֧�� IMFX_FOURCC_NV12 �� IMFX_FOURCC_RGB4 */
	mfx_u32     ulSurfaceOutWidth;	/**< ���surface��� (�ڲ��Զ�16λ����), ���С��stMediaInfo.width�����Զ�ʹ��stMediaInfo.width */
	mfx_u32		ulSurfaceOutHeight;	/**< ���surface�߶� (�ڲ��Զ�16/32(PicStruct == MFX_PICSTRUCT_PROGRESSIVE)λ����), ���С��stMediaInfo.height�����Զ�ʹ��stMediaInfo.Height */
	mfxInfoMFX_ext *pstMediaInfo;	/**< ý����Ϣͷ��ͨ������ DecodeHeader ��� */
	mfx_u32		ulAsyncDepth;		/**< �첽��� [1, 20] */
	mfx_u32		ulRotation;			/**< Motion JPEG ��ת�Ƕȣ�֧�� [90�� 180�� 270] ��ת */
}DECODE_INIT_PARAMS_S;

class IMFX_API CIMFXDecoder : public CBuffering
{
public:
	CIMFXDecoder(CIMFXCommon *poIMFXCommon);
	~CIMFXDecoder();

	IMFX_STS Init(DECODE_INIT_PARAMS_S *pstDecodeInitParams /*, WORK_MODE enSyncMode */);

	mfx_u32 GetFreeSurfaceCount();

	IMFX_STS RunDecoding(FRAME_DATA_S *pstFrameDataIn, BLOCK_MODE enBlockMode = NON_BLOCK);

	IMFX_STS RunDecodingEx(FRAME_DATA_S *pstFrameDataIn, BLOCK_MODE enBlockMode, bool EndOfFile);

	mfx_u32 GetOutputSurfaceCount();

	IMFX_STS GetOutputSurface(mfxFrameSurface1 **pmfxSurface);

	void PutBackOutputSurface();

	IMFX_STS CopyOutputSurface(mfxFrameSurface1 **pmfxSurface);

	/* deprecated method use DecodeHeader instead */
	IMFX_STS GetVideoAttr(VIDEO_ATTR_S *pstVideoAttr);
	
	IMFX_STS GetFrameInfo(mfxFrameInfo *pstFrameInfo);

	void Close();

	bool IsInitialized()
	{
		return m_InitIsCalled;
	}

public: /* static */
	static IMFX_STS DecodeHeader(CODEC_ID_E codec_id, FRAME_DATA_S *pstFrameDataIn, mfxInfoMFX_ext *pstMediaInfo);
	
private:
	mfxStatus RealInit();
	//mfxStatus InitMfxParams();
	mfxStatus AllocSurfaces();
	mfxStatus DeleteSurfaces();

private:
	CIMFXCommon           *m_poIMFXCommon;
	MFXVideoSession        *m_pmfxSession;
	GeneralAllocator      *m_pGeneralAllocator;
	//mfxFrameSurface1       *m_pmfxSurfaces;
	//msdkFrameSurface*       m_pDecSurfacesOut;
	mfxVideoParam           m_mfxVideoParams;
	msdkFrameSurface*       m_pCurrentFreeSurface; // surface detached from free surfaces array
	msdkOutputSurface*      m_pCurrentFreeOutputSurface; // surface detached from free output surfaces array
	msdkOutputSurface*      m_pCurrentOutputSurface; // surface detached from output surfaces array

	mfxFrameAllocResponse   m_mfxResponse;
	//mfxU16                  m_nIndex;   // index of free surface
	//mfxSyncPoint            m_syncp;
	MEM_TYPE_E				m_enMemType;
	CODEC_ID_E				m_outputCodecID;
	MFXVideoDECODE         *m_pmfxDEC;
	
	mfxBitstream            m_mfxBS;
	bool                    m_bGetStreamHeader;
	bool                    m_InitIsCalled;

	mfx_u32					m_ulInitWidth;	/* inner init width, correspond to <mfxVideoParam.mfx.FrameInfo.width>, won't change at the life cycle */
	mfx_u32					m_ulInitHeight;	/* inner init height, correspond to <mfxVideoParam.mfx.FrameInfo.height>, won't change at the life cycle */

private: /* static */

	static void DeleteDecodeHeaderOBJ(void)
	{
		MSDK_SAFE_DELETE(m_pmfxDEC4DecodeHeader);
		MSDK_SAFE_DELETE(m_pmfxSession4DecodeHeader)
	}

	static MFXVideoDECODE *m_pmfxDEC4DecodeHeader;
	static MFXVideoSession *m_pmfxSession4DecodeHeader;
	static MSDKMutex m_Mutex4DecodeHeader;
};

#endif