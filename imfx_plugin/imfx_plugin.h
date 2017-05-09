#ifndef __IMFX_PLUGIN_H__
#define __IMFX_PLUGIN_H__

#include "vm/so_defs.h"
#include "plugin_utils.h"
//#include "rotate_plugin_api.h"
//#include "plugin_loader.h"
#include "opencl_plugin.h"
#include "OCLStruct.h"

/*初始化结构体的4个点,按顺时针方向给出4个点，不按顺时针方向也可以，内部已排序*//*每个通道的4个点不一定相同*/
typedef struct tagPointInfo
{
	FPOINT_S imagePoints[4];
}POINT_INFO_S;

/*放四个点的信息和窗口的宽高信息*/
struct RotateParam
{
	long Angle;
	long wWith;
	long wHeight;
	POINT_INFO_S sPointY_INFOR;
	POINT_INFO_S sPointUV_INFOR;
};

typedef struct tagPluginInitParams
{
	VIDEO_ATTR_S stInVideoAttr;     /**< 输入视频属性 */

	VIDEO_ATTR_S stOutVideoAttr;    /**< 输出视频属性 */

	mfx_u32	ulAsyncDepth;			/**< 异步深度 [1, 20], 目前暂不支持异步操作 */

	msdk_char strPluginPath[MSDK_MAX_FILENAME_LEN]; // plugin dll path and name
	sPluginParams pluginParams;
	mfxU8 nRotationAngle;

	mfxFrameInfo    FrameInfo;

}PLUGIN_INIT_PARAMS_S;

class IMFX_API CIMFXPlugin	// : public CBuffering
{
public:
	CIMFXPlugin(CIMFXCommon *poIMFXCommon);
	~CIMFXPlugin();

	IMFX_STS Init(PLUGIN_INIT_PARAMS_S *pstPluginInitParams);
	IMFX_STS RunPlugin(mfxFrameSurface1 *pmfxSurfaceIn, mfxFrameSurface1 **pmfxSurfaceOut, mfxU32 in_num = 1, mfxU32 out_num = 1);
	IMFX_STS RunPluginEx(mfxFrameSurface1 *pmfxSurfaceIn, mfxFrameSurface1 **pmfxSurfaceOut, int dstW, int dstH);

	void Close();
	IMFX_STS SetPointsParam(RotateParam *rotateParams);
	/*亮度调节*/
	IMFX_STS SetParam(float &fParams);
	mfxFrameSurface1 * GetFreeSurfaceIn();

private:
	mfxStatus AllocSurfaces();
	mfxStatus InitPluginParam(PLUGIN_INIT_PARAMS_S *pInParams);

private:
	CIMFXCommon           *m_poIMFXCommon;
	MFXVideoSession        *m_pmfxSession;
	GeneralAllocator      *m_pGeneralAllocator;
	mfxFrameAllocResponse   m_PluginResponseIn;
	mfxFrameAllocRequest	m_RequestIn;
	mfxFrameAllocResponse   m_PluginResponseOut;
	mfxFrameAllocRequest	m_RequestOut;

	//VPP_SURFACE_USAGE_E     m_enVPPSurfaceUsage;

	//msdkFrameSurface*       m_pCurrentFreeSurface; // surface detached from free surfaces array
	//msdkOutputSurface*      m_pCurrentFreeOutputSurface; // surface detached from free output surfaces array
	//msdkOutputSurface*      m_pCurrentOutputSurface; // surface detached from output surfaces array

	MEM_TYPE_E				m_enMemType;

	//mfxFrameSurface1       *m_pmfxSurfacesIn;
	//mfxFrameSurface1       *m_pmfxSurfacesOut;

	//msdk_so_handle          m_PluginModule;
	//MFXGenericPlugin*       m_pusrPlugin;
	std::auto_ptr<MFXPlugin> m_pPlugin;

	mfxVideoParam           m_PluginVideoParams;

	RotateParam            m_RotateParams;

	// new method

	OCLStruct           m_OCLStruct; //OCL data
	OCLPlugin*          m_pOCLPlugin;

	IDirect3DDevice9Ex	*m_pD3DDevice;
	mfx_u32				m_ulAsyncDepth;			/**< 异步深度 [1, 20] */

	//for ocl input
	mfx_u32				m_numSurfaceIn;
	mfxMemId			*m_pSurfaceMidsIn;
	mfxFrameSurface1_OCL *m_pmfxOCLSurfacesIn; // frames array for OCL plugin input

	//for ocl output
	mfx_u32				m_numSurfaceOut;
	mfxMemId			*m_pSurfaceMidsOut;
	mfxFrameSurface1_OCL *m_pmfxOCLSurfacesOut; // frames array for OCL plugin input

	bool				m_InitIsCalled;
};

#endif // !__IMFX_PLUGIN_H__
