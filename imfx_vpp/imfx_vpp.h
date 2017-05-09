#ifndef __IMFX_VPP_H__
#define __IMFX_VPP_H__

typedef enum tagVPPSurfaceUsage
{
	NOT_USE_VPP_SURFACE     = 0x00,
	USE_VPP_SURFACE_IN      = 0x01,
	USE_VPP_SURFACE_OUT     = 0x02,
	USE_VPP_SURFACE_IN_OUT  = 0x03
}VPP_SURFACE_USAGE_E;

typedef struct tagVppInitParams
{
	VIDEO_ATTR_S stInVideoAttr;     /**< 输入视频属性 */

	VIDEO_ATTR_S stOutVideoAttr;    /**< 输出视频属性 */

	bool bOutput2SystemMemory;		/**< 输出数据到系统内存，只有当全局内存类型不是 SYSTEM_MEMORY,该标志才有作用 */

	mfx_u32 ulSurfaceInWidth;
	mfx_u32 ulSurfaceInHeight;

	RESIZE_S stResize;			/**< resize mode */

	mfx_u32	ulAsyncDepth;			/**< 异步深度 [1, 20] */

	mfx_u32   ulDenoiseFactor;		/**< 降噪系数 [0,100], 0-不降噪，1-100降噪系数有效值 */
}VPP_INIT_PARAMS_S;

class IMFX_API CIMFXVpp : public CBuffering
{
public:
	CIMFXVpp(CIMFXCommon *poIMFXCommon);
	~CIMFXVpp();

	IMFX_STS Init(VPP_INIT_PARAMS_S *pstVppInitParams, VPP_SURFACE_USAGE_E enVPPSurfaceUsage);

	IMFX_STS RunVPP(mfxFrameSurface1 *pmfxSurfaceIn, BLOCK_MODE enBlockMode = NON_BLOCK);
	IMFX_STS Resize(mfxFrameSurface1 *pmfxSurfaceIn, mfxFrameSurface1 *pmfxSurfaceOut);

	mfx_u32 GetFreeSurfaceCount();
	mfx_u32 GetOutputSurfaceCount();

	IMFX_STS GetOutputSurface(mfxFrameSurface1 **pmfxSurface);
	void PutBackOutputSurface();

	IMFX_STS CopyOutputSurface(mfxFrameSurface1 **pmfxSurface);

	IMFX_STS GetVideoAttr(VIDEO_ATTR_S *pstVideoAttrIn, VIDEO_ATTR_S *pstVideoAttrOut);
	IMFX_STS GetFrameInfo(mfxFrameInfo *pstFrameInfoIn, mfxFrameInfo *pstFrameInfoOut);

	void Close();

private:
	mfxStatus AllocSurfaces();

private:
	CIMFXCommon           *m_poIMFXCommon;
	MFXVideoSession        *m_pmfxSession;
	GeneralAllocator      *m_pGeneralAllocator;
	mfxFrameAllocResponse   m_mfxResponseIn;
	mfxFrameAllocResponse   m_mfxResponseOut;
	VPP_SURFACE_USAGE_E     m_enVPPSurfaceUsage;

	mfxVideoParam           m_mfxVideoParams;

	msdkFrameSurface*       m_pCurrentFreeSurface; // surface detached from free surfaces array
	msdkOutputSurface*      m_pCurrentFreeOutputSurface; // surface detached from free output surfaces array
	msdkOutputSurface*      m_pCurrentOutputSurface; // surface detached from output surfaces array

	//mfxSyncPoint            m_syncp;
	MEM_TYPE_E				m_enMemType;
	bool					m_bOutput2SystemMemory;
	mfxU8*					m_surfaceSystemBuffersOut;

	MFXVideoVPP            *m_pmfxVPP;
	//mfxFrameSurface1       *m_pmfxSurfacesIn;
	//mfxFrameSurface1       *m_pmfxSurfacesOut;
	//mfxU16                  m_numSurfacesIn;
	//mfxU16                  m_numSurfacesOut;
	//mfxU16                  m_nIndexIn;
	//mfxU16                  m_nIndexOut;

	mfxU32                  m_tabExtVppAlg[4];
	mfxExtVPPDoUse          m_VppDoUse;
	mfxExtVPPDoNotUse       m_VppDoNotUse;      // for disabling VPP algorithms
	mfxExtBuffer*           m_VppExtParams[3];

	RESIZE_S				m_stResize;

	bool                    m_InitIsCalled;
};

#endif // !__IMFX_VPP_H__
