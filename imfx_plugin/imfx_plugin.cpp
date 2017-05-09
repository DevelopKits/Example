#include "imfx.h"
#include "imfx_common.h"
#include "imfx_plugin.h"
//#include "mfx_plugin_module.h"
//#include "d3d_utils.h"

static MSDKMutex m_Mutex4OCL;

CIMFXPlugin::CIMFXPlugin(CIMFXCommon *poIMFXCommon)
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

	mfxHDL pHdl = NULL;

	poIMFXCommon->GetDeviceHandle(&pHdl);

	m_pD3DDevice = (IDirect3DDevice9Ex *)pHdl;

	m_numSurfaceIn = 0;
	m_pSurfaceMidsIn = NULL;
	m_pmfxOCLSurfacesIn = NULL;

	m_numSurfaceOut = 0;
	m_pSurfaceMidsOut = NULL;
	m_pmfxOCLSurfacesOut = NULL;

	memset(&m_PluginResponseIn, 0, sizeof(m_PluginResponseIn));
	//m_enVPPSurfaceUsage = NOT_USE_VPP_SURFACE;

	memset(&m_PluginVideoParams, 0, sizeof(m_PluginVideoParams));
	memset(&m_RotateParams, 0, sizeof(m_RotateParams));

	//m_pPluginSurfaces = NULL;

	m_InitIsCalled = false;

	m_ulAsyncDepth = 0;
	m_pOCLPlugin = NULL;
}

IMFX_STS CIMFXPlugin::SetPointsParam(RotateParam *rotateParams)
{
	mfxStatus sts = MFX_ERR_NONE;
	sts = m_pOCLPlugin->SetAuxParams(rotateParams, sizeof(m_RotateParams));
	//sts = m_pOCLPlugin->SetPointsAuxParams(rotateParams, sizeof(m_RotateParams));
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);
	return (IMFX_STS)sts;
}

IMFX_STS CIMFXPlugin::SetParam(float &fParams)
{
	m_pOCLPlugin->SetParam(fParams);

	return IMFX_ERR_NONE;
}

IMFX_STS CIMFXPlugin::Init(PLUGIN_INIT_PARAMS_S *pstPluginInitParams)
{
	MSDK_CHECK_POINTER(pstPluginInitParams, IMFX_ERR_NULL_PTR);

	if (true == m_InitIsCalled)
	{
		return IMFX_ERR_REINITIALIZE;
	}

	mfxStatus sts = MFX_ERR_NONE;

	m_ulAsyncDepth = pstPluginInitParams->ulAsyncDepth;

	m_ulAsyncDepth = ((m_ulAsyncDepth > 0) && (m_ulAsyncDepth <= 20)) ? m_ulAsyncDepth : 1;

#if 0
	/* load dll */
	m_PluginModule = msdk_so_load(pstPluginInitParams->strPluginDLLPath);
	MSDK_CHECK_POINTER(m_PluginModule, MFX_ERR_NOT_FOUND);

	PluginModuleTemplate::fncCreateGenericPlugin pCreateFunc = (PluginModuleTemplate::fncCreateGenericPlugin)msdk_so_get_addr(m_PluginModule, "mfxCreateGenericPlugin");
	MSDK_CHECK_POINTER(pCreateFunc, MFX_ERR_NOT_FOUND);

	m_pusrPlugin = (*pCreateFunc)();
	MSDK_CHECK_POINTER(m_pusrPlugin, MFX_ERR_NOT_FOUND);

	mfxVersion version;
	sts = m_poIMFXCommon->QueryVersion(version); // get real API version of the loaded library
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	if (CheckVersion(&version, MSDK_FEATURE_PLUGIN_API)) {
		mfxIMPL impl =  MFX_IMPL_HARDWARE;

		sPluginParams pluginParams;
		mfxU32 CodecId = pstPluginInitParams->stInVideoAttr.enCodecID;
		pluginParams.pluginGuid = msdkGetPluginUID(impl, MSDK_VENCODE, CodecId);
		if (AreGuidsEqual(pstPluginInitParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL) && impl == MFX_IMPL_HARDWARE)
			pstPluginInitParams->pluginParams.pluginGuid = msdkGetPluginUID(MFX_IMPL_SOFTWARE, MSDK_VENCODE, CodecId);
		if (!AreGuidsEqual(pstPluginInitParams->pluginParams.pluginGuid, MSDK_PLUGINGUID_NULL))
		{
			m_pPlugin.reset(LoadPlugin(MFX_PLUGINTYPE_VIDEO_ENCODE, *m_pmfxSession, pstPluginInitParams->pluginParams.pluginGuid, 1));
			if (m_pPlugin.get() == NULL) sts = MFX_ERR_UNSUPPORTED;
		}
	}
#endif

	sts = InitPluginParam(pstPluginInitParams);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	m_Mutex4OCL.Lock();
	m_OCLStruct.OCLInit(m_pD3DDevice);

	//create OCL plugin
	m_pOCLPlugin = new OCLPlugin(m_pD3DDevice, false);
	sts = m_pOCLPlugin->InitOCL(pstPluginInitParams->strPluginPath,   &m_OCLStruct);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	m_Mutex4OCL.Unlock();
	/* alloc surface */
	sts = AllocSurfaces();
	//sts = AllocSurfaces_new();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	mfxPlugin pluginAdapter = make_mfx_plugin_adapter((MFXGenericPlugin*)m_pOCLPlugin);

	sts = MFXVideoUSER_Register(*m_pmfxSession, 0, &pluginAdapter);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

	// need to call Init after registration because mfxCore interface is needed

	//sts = m_pOCLPlugin->Init(&OCLPluginVideoParams, &m_RotateRequest, &m_PluginResponse);
	sts = m_pOCLPlugin->SetAuxParams(&m_pGeneralAllocator, 0);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);
	//sts = m_pOCLPlugin->Init(&m_PluginVideoParams, m_pmfxOCLSurfacesIn, m_numSurfaceIn);
	//sts = m_pOCLPlugin->Init(&m_PluginVideoParams, outputSurfaceMids, surfaceNum);
	sts = m_pOCLPlugin->Init(&m_PluginVideoParams);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, (IMFX_STS)sts);

#if 0
	// register plugin callbacks in Media SDK
	mfxPlugin plg = make_mfx_plugin_adapter(m_pusrPlugin);
	sts = MFXVideoUSER_Register(*m_pmfxSession, 0, &plg);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// need to call Init after registration because mfxCore interface is needed
	sts = m_pusrPlugin->Init(&m_pluginVideoParams);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = m_pusrPlugin->SetAuxParams(&m_RotateParams, sizeof(m_RotateParams));
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
#endif

	m_InitIsCalled = true;
	return IMFX_ERR_NONE;
}

mfxStatus CIMFXPlugin::InitPluginParam(PLUGIN_INIT_PARAMS_S *pstPluginInitParams)
{
	MSDK_CHECK_POINTER(pstPluginInitParams, MFX_ERR_NULL_PTR);

	MSDK_ZERO_MEMORY(m_PluginVideoParams);

	m_PluginVideoParams.AsyncDepth = pstPluginInitParams->ulAsyncDepth; // the maximum number of tasks that can be submitted before any task execution finishes
	m_PluginVideoParams.vpp.In = pstPluginInitParams->FrameInfo;
	m_PluginVideoParams.vpp.Out = pstPluginInitParams->FrameInfo;

	m_PluginVideoParams.vpp.In.CropW = pstPluginInitParams->stInVideoAttr.ulWidth;
	m_PluginVideoParams.vpp.In.CropH = pstPluginInitParams->stInVideoAttr.ulHeight;
	m_PluginVideoParams.vpp.Out.CropW = pstPluginInitParams->stOutVideoAttr.ulWidth;
	m_PluginVideoParams.vpp.Out.CropH = pstPluginInitParams->stOutVideoAttr.ulHeight;

	m_PluginVideoParams.vpp.In.Width = MSDK_ALIGN16(m_PluginVideoParams.vpp.In.CropW);
	m_PluginVideoParams.vpp.In.Height =
		(MFX_PICSTRUCT_PROGRESSIVE == m_PluginVideoParams.vpp.In.PicStruct) ?
		MSDK_ALIGN16(m_PluginVideoParams.vpp.In.CropH) :
		MSDK_ALIGN32(m_PluginVideoParams.vpp.In.CropH);

	m_PluginVideoParams.vpp.Out.Width = MSDK_ALIGN16(m_PluginVideoParams.vpp.Out.CropW);
	m_PluginVideoParams.vpp.Out.Height =
		(MFX_PICSTRUCT_PROGRESSIVE == m_PluginVideoParams.vpp.Out.PicStruct) ?
		MSDK_ALIGN16(m_PluginVideoParams.vpp.Out.CropH) :
		MSDK_ALIGN32(m_PluginVideoParams.vpp.Out.CropH);

	//m_pluginVideoParams.vpp.In.FourCC = MFX_FOURCC_NV12;
	//m_pluginVideoParams.vpp.In.Width = m_pluginVideoParams.vpp.In.CropW = pstPluginInitParams->stInVideoAttr.ulWidth  & (~0x01);
	//m_pluginVideoParams.vpp.In.Height = m_pluginVideoParams.vpp.In.CropH = pstPluginInitParams->stInVideoAttr.ulHeight & (~0x01);
	//m_pluginVideoParams.vpp.Out.FourCC = MFX_FOURCC_NV12;
	//m_pluginVideoParams.vpp.Out.Width = m_pluginVideoParams.vpp.Out.CropW = pstPluginInitParams->stOutVideoAttr.ulWidth & (~0x01);
	//m_pluginVideoParams.vpp.Out.Height = m_pluginVideoParams.vpp.Out.CropH = pstPluginInitParams->stOutVideoAttr.ulHeight & (~0x01);

	if (m_enMemType != SYSTEM_MEMORY)
		m_PluginVideoParams.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY | MFX_IOPATTERN_OUT_VIDEO_MEMORY;

	return MFX_ERR_NONE;
}

mfxStatus CIMFXPlugin::AllocSurfaces()
{
	mfxStatus sts = MFX_ERR_NONE;

	/* allocate surface for input */
	memset(&m_RequestIn, 0, sizeof(m_RequestIn));

	m_RequestIn.NumFrameMin = m_ulAsyncDepth;
	m_RequestIn.NumFrameSuggested = m_RequestIn.NumFrameMin + 1;

	//m_RequestIn.Type = MFX_MEMTYPE_INTERNAL_FRAME;
	m_RequestIn.Type = MFX_MEMTYPE_EXTERNAL_FRAME;
	m_RequestIn.Type |= (D3D9_MEMORY == m_enMemType) ? (mfxU16)MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET : (mfxU16)MFX_MEMTYPE_SYSTEM_MEMORY;
	m_RequestIn.Type |= MFX_MEMTYPE_FROM_VPPOUT; // THIS IS A WORKAROUND, NEED TO ADJUST ALLOCATOR

	MSDK_MEMCPY_VAR(m_RequestIn.Info, &(m_PluginVideoParams.vpp.In), sizeof(mfxFrameInfo));

	// alloc frames for rotate input
	sts = m_pGeneralAllocator->Alloc(m_pGeneralAllocator->pthis, &m_RequestIn, &m_PluginResponseIn);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	m_numSurfaceIn = m_PluginResponseIn.NumFrameActual;
	m_pmfxOCLSurfacesIn = new mfxFrameSurface1_OCL[m_PluginResponseIn.NumFrameActual];
	
	msdk_tick m_tStartTime = msdk_time_get_tick();

	for (int i = 0; i < m_PluginResponseIn.NumFrameActual; i++)
	{
		memset(&(m_pmfxOCLSurfacesIn[i]), 0, sizeof(mfxFrameSurface1_OCL));
		memcpy(&(m_pmfxOCLSurfacesIn[i].Info), &(m_PluginVideoParams.vpp.In), sizeof(mfxFrameInfo));
		m_pmfxOCLSurfacesIn[i].Data.MemId = m_PluginResponseIn.mids[i];
			
		if(m_OCLStruct.m_dx9_media_sharing)
		{
			cl_dx9_surface_info_khr info;
			mfxHDLPair* mid_pair = static_cast<mfxHDLPair*>(m_PluginResponseIn.mids[i]);
			info.resource = (IDirect3DSurface9*)mid_pair->first;
			info.shared_handle = mid_pair->second;

			OCL_SAFE_CALL_NORET(( m_pmfxOCLSurfacesIn[i].OCL_Y = m_OCLStruct.m_clCreateFromDX9MediaSurfaceKHR(m_OCLStruct.m_clContext,0,CL_ADAPTER_D3D9EX_KHR,&info,0,&RET_STS),RET_STS));
			OCL_SAFE_CALL_NORET(( m_pmfxOCLSurfacesIn[i].OCL_UV = m_OCLStruct.m_clCreateFromDX9MediaSurfaceKHR(m_OCLStruct.m_clContext,0,CL_ADAPTER_D3D9EX_KHR,&info,1,&RET_STS),RET_STS));
		}
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);	
	}
	
	msdk_tick m_tEndTime = msdk_time_get_tick();
	double elapsed = (m_tEndTime - m_tStartTime) / (double)msdk_time_get_frequency();

	printf("CreateFromDX9MediaSurfaceKHR: %6.3f ms\n", elapsed * 1000);

	/* allocate surface for output */
	memset(&m_RequestOut, 0, sizeof(m_RequestOut));

	m_RequestOut.NumFrameMin = m_ulAsyncDepth;
	m_RequestOut.NumFrameSuggested = m_RequestOut.NumFrameMin + 1;

	m_RequestOut.Type = MFX_MEMTYPE_EXTERNAL_FRAME;
	m_RequestOut.Type |= (D3D9_MEMORY == m_enMemType) ? (mfxU16)MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET : (mfxU16)MFX_MEMTYPE_SYSTEM_MEMORY;
	m_RequestOut.Type |= MFX_MEMTYPE_FROM_VPPOUT; // THIS IS A WORKAROUND, NEED TO ADJUST ALLOCATOR

	MSDK_MEMCPY_VAR(m_RequestOut.Info, &(m_PluginVideoParams.vpp.In), sizeof(mfxFrameInfo));

	// alloc frames for rotate input
	sts = m_pGeneralAllocator->Alloc(m_pGeneralAllocator->pthis, &m_RequestOut, &m_PluginResponseOut);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	m_numSurfaceOut = m_PluginResponseOut.NumFrameActual;
	m_pmfxOCLSurfacesOut = new mfxFrameSurface1_OCL[m_PluginResponseOut.NumFrameActual];

	for (int i = 0; i < m_PluginResponseOut.NumFrameActual; i++)
	{
		memset(&(m_pmfxOCLSurfacesOut[i]), 0, sizeof(mfxFrameSurface1_OCL));
		memcpy(&(m_pmfxOCLSurfacesOut[i].Info), &(m_PluginVideoParams.vpp.In), sizeof(mfxFrameInfo));
		m_pmfxOCLSurfacesOut[i].Data.MemId = m_PluginResponseOut.mids[i];
		if(m_OCLStruct.m_dx9_media_sharing)
		{
			IDirect3DSurface9 *inputD3DSurf = NULL;
			//sts = m_pMFXAllocator->GetHDL(m_pMFXAllocator->pthis, m_PluginResponseOut.mids[i], reinterpret_cast<mfxHDL*>(&inputD3DSurf));
			//if(MFX_ERR_NONE != sts) return MFX_ERR_UNKNOWN;

			cl_dx9_surface_info_khr info;
			mfxHDLPair* mid_pair = static_cast<mfxHDLPair*>(m_PluginResponseOut.mids[i]);
			info.resource = (IDirect3DSurface9*)mid_pair->first;
			info.shared_handle = mid_pair->second;

			OCL_SAFE_CALL_NORET(( m_pmfxOCLSurfacesOut[i].OCL_Y = m_OCLStruct.m_clCreateFromDX9MediaSurfaceKHR(m_OCLStruct.m_clContext,0,CL_ADAPTER_D3D9EX_KHR,&info,0,&RET_STS),RET_STS));
			OCL_SAFE_CALL_NORET(( m_pmfxOCLSurfacesOut[i].OCL_UV = m_OCLStruct.m_clCreateFromDX9MediaSurfaceKHR(m_OCLStruct.m_clContext,0,CL_ADAPTER_D3D9EX_KHR,&info,1,&RET_STS),RET_STS));
		}
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}

	return MFX_ERR_NONE;
}

// function that scan frame array and return not locked frame
static mfxU16 GetFreeOCLSurfaceIndex(mfxFrameSurface1_OCL* pSurfacesPool, mfxU16 nPoolSize, mfxU16 start=0)
{
	if (pSurfacesPool)
	{
		for (mfxU16 i = 0; i < nPoolSize; i++)
		{
			mfxU16 ii = (start+i)%nPoolSize;
			if (0 == pSurfacesPool[ii].Data.Locked)
			{
				return ii;
			}
		}
	}

	return MSDK_INVALID_SURF_IDX;
}

IMFX_STS CIMFXPlugin::RunPlugin(mfxFrameSurface1 *pmfxSurfaceIn, mfxFrameSurface1 **pmfxSurfaceOut, mfxU32 in_num /*= 1*/, mfxU32 out_num /*= 1*/)
{
	MSDK_CHECK_POINTER(pmfxSurfaceIn, IMFX_ERR_NULL_PTR);

	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	//msdk_tick m_tStartTime = msdk_time_get_tick();
	mfxStatus           sts = MFX_ERR_NONE;
	mfxSyncPoint		SyncPoint = NULL; // ~ with  plugin call
	//mfxFrameSurface1_OCL *pmfxOCLSurfaceIn = (mfxFrameSurface1_OCL *)pmfxSurfaceIn;

	/* copy from surface in to ocl surface */
	//mfxU16 nOCLSurfIndexIn = GetFreeOCLSurfaceIndex(m_pmfxOCLSurfacesIn, m_numSurfaceIn);
	//MSDK_CHECK_ERROR(nOCLSurfIndexIn, MSDK_INVALID_SURF_IDX, IMFX_ERR_MEMORY_ALLOC);

	mfxFrameSurface1_OCL *pOclSurfaceIn = &m_pmfxOCLSurfacesIn[0];
	
	mfxHDLPair* mid_pair = static_cast<mfxHDLPair*>(pmfxSurfaceIn->Data.MemId);
	mfxHDLPair* mid_pairOCLIn = static_cast<mfxHDLPair*>(pOclSurfaceIn->Data.MemId);

	IDirect3DSurface9 *surfIn = (IDirect3DSurface9*)mid_pair->first;
	IDirect3DSurface9 *surfOCLIn = (IDirect3DSurface9*)mid_pairOCLIn->first;

#if 0
	RECT srcRect, dstRect;
	srcRect.left = srcRect.top = 0;
	srcRect.right = pmfxSurfaceIn->Info.Width;
	srcRect.bottom = pmfxSurfaceIn->Info.Height;`

	dstRect.left = dstRect.top = 0;
	dstRect.right = pOclSurfaceIn->Info.Width;
	dstRect.bottom = pOclSurfaceIn->Info.Height;
#endif
	//HRESULT hr = m_pD3DDevice->StretchRect(dxMemIdIn->m_surface, NULL, dxMemIdOCLIn->m_surface, NULL, D3DTEXF_NONE);
	//HRESULT hr = m_pD3DDevice->StretchRect(dxMemIdIn->m_surface, &srcRect, dxMemIdOCLIn->m_surface, &dstRect, D3DTEXF_LINEAR);
	HRESULT hr = m_pD3DDevice->StretchRect(surfIn, NULL, surfOCLIn, NULL, D3DTEXF_POINT);
	if (FAILED(hr))
	{
		return IMFX_ERR_UNKNOWN;
	}

	//msdk_tick m_tEndTime = msdk_time_get_tick();
	//double elapsed = (m_tEndTime - m_tStartTime) / (double)msdk_time_get_frequency();

	//printf("StretchRect: %6.3f ms\n", elapsed * 1000);

	do
	{
		//mfxU16 nOCLSurfIndexOut = GetFreeOCLSurfaceIndex(m_pmfxOCLSurfacesOut, m_numSurfaceOut);
		//MSDK_CHECK_ERROR(nOCLSurfIndexOut, MSDK_INVALID_SURF_IDX, IMFX_ERR_MEMORY_ALLOC);

		mfxHDL pSurfaceIn = pOclSurfaceIn;
		mfxHDL pSurfaceOut = &m_pmfxOCLSurfacesOut[0];

		do
		{
			sts = MFXVideoUSER_ProcessFrameAsync(*m_pmfxSession, &pSurfaceIn, 1, &pSurfaceOut, 1, &SyncPoint);
			if (MFX_WRN_DEVICE_BUSY == sts)
			{
				MSDK_SLEEP(1); // just wait and then repeat the same call
			}
			else
			{
				break;
			}
		} while (1);

		if (MFX_ERR_MORE_DATA == sts) { // will never happen actually
			continue;
		}
		else if (MFX_ERR_MORE_SURFACE == sts)
		{
			continue;
		}
		else if (MFX_ERR_NONE != sts) {
			break;
		}

		if (MFX_ERR_NONE == sts) {
			if (SyncPoint) 	{
				mfxStatus _sts = m_pmfxSession->SyncOperation(SyncPoint, MSDK_DEC_WAIT_INTERVAL);
				if (MFX_ERR_NONE != _sts) {
					return IMFX_ERR_MORE_DATA;
				}
				*pmfxSurfaceOut = (mfxFrameSurface1*)pSurfaceOut;
				break;
			}
			break;
		}
	} while (1);
	
	return (IMFX_STS)sts;
}

IMFX_STS CIMFXPlugin::RunPluginEx( mfxFrameSurface1 *pmfxSurfaceIn, mfxFrameSurface1 **pmfxSurfaceOut, int dstW, int dstH)
{
	MSDK_CHECK_POINTER(pmfxSurfaceIn, IMFX_ERR_NULL_PTR);

	if (false == m_InitIsCalled)
	{
		return IMFX_ERR_UNINITIALIZED;
	}

	//msdk_tick m_tStartTime = msdk_time_get_tick();
	mfxStatus           sts = MFX_ERR_NONE;
	mfxSyncPoint		SyncPoint = NULL; // ~ with  plugin call
	//mfxFrameSurface1_OCL *pmfxOCLSurfaceIn = (mfxFrameSurface1_OCL *)pmfxSurfaceIn;

	mfxFrameSurface1_OCL *pOclSurfaceIn = &m_pmfxOCLSurfacesIn[0];

	mfxHDLPair* mid_pair = static_cast<mfxHDLPair*>(pmfxSurfaceIn->Data.MemId);
	mfxHDLPair* mid_pairOCLIn = static_cast<mfxHDLPair*>(pOclSurfaceIn->Data.MemId);

	IDirect3DSurface9 *surfIn = (IDirect3DSurface9*)mid_pair->first;
	IDirect3DSurface9 *surfOCLIn = (IDirect3DSurface9*)mid_pairOCLIn->first;

	HRESULT hr = m_pD3DDevice->StretchRect(surfIn, NULL, surfOCLIn, NULL, D3DTEXF_LINEAR);
	if (FAILED(hr))
	{
		return IMFX_ERR_UNKNOWN;
	}

	//msdk_tick m_tEndTime = msdk_time_get_tick();
	//double elapsed = (m_tEndTime - m_tStartTime) / (double)msdk_time_get_frequency();

	//printf("StretchRect: %6.3f ms\n", elapsed * 1000);

	do
	{
		//mfxU16 nOCLSurfIndexOut = GetFreeOCLSurfaceIndex(m_pmfxOCLSurfacesOut, m_numSurfaceOut);
		//MSDK_CHECK_ERROR(nOCLSurfIndexOut, MSDK_INVALID_SURF_IDX, IMFX_ERR_MEMORY_ALLOC);

		mfxHDL pSurfaceIn = pOclSurfaceIn;
		mfxHDL pSurfaceOut = &m_pmfxOCLSurfacesOut[0];

		do
		{
			sts = MFXVideoUSER_ProcessFrameAsync(*m_pmfxSession, &pSurfaceIn, 1, &pSurfaceOut, 1, &SyncPoint);
			if (MFX_WRN_DEVICE_BUSY == sts)
			{
				MSDK_SLEEP(1); // just wait and then repeat the same call
			}
			else
			{
				break;
			}
		} while (1);

		if (MFX_ERR_MORE_DATA == sts) { // will never happen actually
			continue;
		}
		else if (MFX_ERR_MORE_SURFACE == sts)
		{
			continue;
		}
		else if (MFX_ERR_NONE != sts) {
			break;
		}

		if (MFX_ERR_NONE == sts) {
			if (SyncPoint) 	{
				mfxStatus _sts = m_pmfxSession->SyncOperation(SyncPoint, MSDK_DEC_WAIT_INTERVAL);
				if (MFX_ERR_NONE != _sts) {
					return IMFX_ERR_MORE_DATA;
				}
				*pmfxSurfaceOut = (mfxFrameSurface1*)pSurfaceOut;
				break;
			}
			break;
		}
	} while (1);

	return (IMFX_STS)sts;
}


void CIMFXPlugin::Close()
{
	if (false == m_InitIsCalled)
	{
		return;
	}

	MSDK_SAFE_DELETE_ARRAY(m_pmfxOCLSurfacesIn);
	MSDK_SAFE_DELETE_ARRAY(m_pmfxOCLSurfacesOut);

	MFXVideoUSER_Unregister(*m_pmfxSession, 0);
	MSDK_SAFE_DELETE(m_pOCLPlugin);

	if (m_PluginResponseIn.NumFrameActual > 0)
	{
		m_pGeneralAllocator->Free(m_pGeneralAllocator->pthis, &m_PluginResponseIn);
		MSDK_ZERO_MEMORY(m_PluginResponseIn);
	}

	if (m_PluginResponseOut.NumFrameActual > 0)
	{
		m_pGeneralAllocator->Free(m_pGeneralAllocator->pthis, &m_PluginResponseOut);
		MSDK_ZERO_MEMORY(m_PluginResponseOut);
	}

	m_pmfxSession   = NULL;
	m_pGeneralAllocator = NULL;

	memset(&m_PluginVideoParams, 0, sizeof(m_PluginVideoParams));
	memset(&m_RotateParams, 0, sizeof(m_RotateParams));

	m_InitIsCalled = false;
}

CIMFXPlugin::~CIMFXPlugin()
{
	Close();
}

mfxFrameSurface1 * CIMFXPlugin::GetFreeSurfaceIn()
{
	mfxU16 nOCLSurfIndexIn = GetFreeOCLSurfaceIndex(m_pmfxOCLSurfacesIn, m_numSurfaceIn);
	if (nOCLSurfIndexIn == MSDK_INVALID_SURF_IDX)
		return NULL;

	mfxFrameSurface1_OCL *pOclSurfaceIn = &m_pmfxOCLSurfacesIn[nOCLSurfIndexIn];

	return pOclSurfaceIn;
}
