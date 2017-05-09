// Copyright (c) 2009-2011 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include <iostream>
#include <fstream>
#include <windows.h>
#include "sample_defs.h"
#include "opencl_plugin.h"
#include "basic.hpp"


#pragma warning(disable : 4996)

using namespace std;

#define OCL_SAFE_CALL(FUNC) OCL_SAFE_CALL_ACT(FUNC,return MFX_ERR_DEVICE_FAILED)

// Constructor for openCL plugin
OCLPlugin::OCLPlugin(IDirect3DDevice9Ex *pd3dDevice, bool FastRelaxedMath)
{
	MSDK_TRACE(__FUNCTION__);

	m_OCLFlag = true;
	m_pOCLStruct = NULL;
	//m_pMFXFrameAllocator = NULL;

	m_FlagFastRelaxedMath = FastRelaxedMath;
	m_Param1 = 50;
	m_Param2 = 50;
	m_MouseX = -1;
	m_MouseY = -1;
	m_ButtonFlag = 0;
	m_TimeAver = 0;
	m_OCLBuffer = NULL;
	m_OCLBufferSize = 0;

	// MFX
	m_bInited = false;
	m_pmfxCore = NULL;
	m_pTasks = NULL;
	m_MaxNumTasks = 0;
	memset(&m_mfxVideoParam, 0, sizeof(m_mfxVideoParam));
	memset(&m_mfxPluginParam, 0, sizeof(m_mfxPluginParam));
	// fill mfxPluginParam
	m_mfxPluginParam.ThreadPolicy = MFX_THREADPOLICY_SERIAL;
	m_mfxPluginParam.MaxThreadNum = 1;

	// OpenCL
	m_ProgramNum = 0;
	for (int i = 0; i < MAX_PROGRAM_NUM; ++i)
	{
		m_pProgramSource[i] = 0;
		m_pProgramBuildLOG[i] = 0;
		m_clProgram[i] = 0;
		m_clKernelProcessY[i] = 0;
		m_clKernelMouse[i] = 0;
		m_clKernelProcessUV[i] = 0;
	}
	m_GlobalWorkSizeY[0] = m_GlobalWorkSizeY[1] = 0;
	m_GlobalWorkSizeUV[0] = m_GlobalWorkSizeUV[1] = 0;

	//m_hDevice = NULL;

	//m_pd3dDevice = pd3dDevice;

	m_PictureH = 0;
	m_PictureW = 0;
	m_PictureBytesUV = 0;
	m_PictureBytesY = 0;

	/*新增初始化*/
	isFirst = false;
	mean = NULL;
	weight = NULL;
	varance = NULL;
	/*新增初始化*/

	m_fTherad = NULL;
}

// destruct OpenCL Plugin
OCLPlugin::~OCLPlugin()
{
	MSDK_TRACE(__FUNCTION__);
	Close();
	PluginClose();
}


// set current program that will be executed for video frame
void    OCLPlugin::SetProgram(int index)
{
	MSDK_TRACE(__FUNCTION__);
	if (m_pOCLStruct == NULL) 
	{
		return;
	}

	if (m_ProgramIndex != index)
	{
		// clear internal OCL buffer
		void *pData;
		cl_int err;
		pData = clEnqueueMapBuffer(m_pOCLStruct->m_clCommandQueue, m_OCLBuffer, true, CL_MAP_READ, 0, m_OCLBufferSize, 0, NULL, NULL, &err);
		memset(pData, 0, m_OCLBufferSize);
		err = clEnqueueUnmapMemObject(m_pOCLStruct->m_clCommandQueue, m_OCLBuffer, pData, 0, NULL, NULL);
	}
	m_ProgramIndex = index;
}

// return name of currently executed or given program
WCHAR   *OCLPlugin::GetProgramName(int index)
{
	MSDK_TRACE(__FUNCTION__);
	if (index < 0) 
	{
		index = m_ProgramIndex;
	}
	
	if (index < 0 || index >= m_ProgramNum) 
	{
		return NULL;
	}
	
	return m_ProgramName[index];
};

// return source code of currently executed or given program
char   *OCLPlugin::GetProgramSRC(int index)
{
	MSDK_TRACE(__FUNCTION__);
	if (index < 0) 
	{
		index = m_ProgramIndex;
	}
	if (index < 0 || index >= m_ProgramNum) 
	{
		return NULL;
	}
	
	return m_pProgramSource[index];
};

// return build log of currently executed or given program
char   *OCLPlugin::GetProgramBuildLOG(int index)
{
	MSDK_TRACE(__FUNCTION__);
	if (index < 0)
	{
		index = m_ProgramIndex;
	}
	if (index < 0 || index >= m_ProgramNum)
	{
		return NULL;
	}
	
	return m_pProgramBuildLOG[index];
};

//communication protocol between particular version of plugin and application
// auxParam contains parameters
mfxStatus OCLPlugin::SetAuxParams(void *auxParam, int auxParamSize)
{
	MSDK_TRACE(__FUNCTION__);
	// set frame allocator
	//m_pMFXFrameAllocator = *(BaseFrameAllocator**)auxParam;
	return MFX_ERR_NONE;
}
// Initialization
// mfxParam contains input and output video parameters (resolution, etc).


mfxStatus OCLPlugin::SetParam(float &fParams)
{
	m_fTherad = &fParams;
	m_mTherad = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float), m_fTherad, NULL);
	return MFX_ERR_NONE;
}
mfxStatus OCLPlugin::Init(mfxVideoParam *mfxParam)
{
	MSDK_TRACE(__FUNCTION__);

	// check validity of parameters
	MSDK_CHECK_POINTER(mfxParam, MFX_ERR_NULL_PTR);
	// only NV12 color format is supported

	if (MFX_FOURCC_NV12 != mfxParam->vpp.In.FourCC || MFX_FOURCC_NV12 != mfxParam->vpp.Out.FourCC)
	{
		return MFX_ERR_UNSUPPORTED;
	}

	// save mfxVideoParam
	m_mfxVideoParam = *mfxParam;

	// set maximum nuber of internal task
	m_MaxNumTasks = m_mfxVideoParam.AsyncDepth + 1;
	// Create internal task pool
	m_pTasks = new MFXTask[m_MaxNumTasks];
	MSDK_CHECK_POINTER(m_pTasks, MFX_ERR_MEMORY_ALLOC);
	memset(m_pTasks, 0, sizeof(MFXTask) * m_MaxNumTasks);

	// Picture size
	m_PictureW = mfxParam->vpp.In.Width;
	m_PictureH = mfxParam->vpp.In.Height;
	m_PictureBytesY = m_PictureH * m_PictureW;
	m_PictureBytesUV = m_PictureH * m_PictureW / 2;

	// Global and loacal work sizes to process each Y component
	// Each workitem process 1 pixel
	m_GlobalWorkSizeY[0] = m_PictureW;
	m_GlobalWorkSizeY[1] = m_PictureH;
	m_LocalWorkSizeY[0] = 8;
	m_LocalWorkSizeY[1] = 8;
	m_GlobalWorkSizeY[0] = m_LocalWorkSizeY[0] * (m_GlobalWorkSizeY[0] / m_LocalWorkSizeY[0]);
	m_GlobalWorkSizeY[1] = m_LocalWorkSizeY[1] * (m_GlobalWorkSizeY[1] / m_LocalWorkSizeY[1]);

	// Global and loacal work sizes to process each UV component
	// Each workitem process 4 neirghborhood pixels
	m_GlobalWorkSizeUV[0] = m_PictureW / 2;
	m_GlobalWorkSizeUV[1] = m_PictureH / 2;
	m_LocalWorkSizeUV[0] = 8;
	m_LocalWorkSizeUV[1] = 8;
	m_GlobalWorkSizeUV[0] = m_LocalWorkSizeUV[0] * (m_GlobalWorkSizeUV[0] / m_LocalWorkSizeUV[0]);
	m_GlobalWorkSizeUV[1] = m_LocalWorkSizeUV[1] * (m_GlobalWorkSizeUV[1] / m_LocalWorkSizeUV[1]);

	printf("Image size %dx%d\n", m_PictureW, m_PictureH);
	printf("Y global size %dx%d\n", m_GlobalWorkSizeY[0], m_GlobalWorkSizeY[1]);
	printf("Y local size %dx%d\n", m_LocalWorkSizeY[0], m_LocalWorkSizeY[1]);
	printf("UV global size %dx%d\n", m_GlobalWorkSizeUV[0], m_GlobalWorkSizeUV[1]);
	printf("UV local size %dx%d\n", m_LocalWorkSizeUV[0], m_LocalWorkSizeUV[1]);

	// recreate internal buffer according new image size
	SAFE_FREE(m_OCLBuffer, clReleaseMemObject);
	m_OCLBufferSize = 0;
	if (m_pOCLStruct && (m_PictureW * m_PictureH) > 0)
	{
		// create internal buffer for OCL programs initialized by 0
		assert(m_pOCLStruct->m_clContext);
		m_OCLBufferSize = sizeof(float) * m_PictureW * m_PictureH;
		OCL_SAFE_CALL(m_OCLBuffer = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_WRITE, m_OCLBufferSize, NULL, &RET_STS));
		{
			// clear internal OCL buffer
			void *pData = NULL;
			cl_int RET_STS = CL_SUCCESS;

			pData = clEnqueueMapBuffer(m_pOCLStruct->m_clCommandQueue, m_OCLBuffer, true, CL_MAP_READ, 0, m_OCLBufferSize, 0, NULL, NULL, &RET_STS);
			if (RET_STS != CL_SUCCESS)
			{
				IMFX_ERR("clEnqueueMapBuffer failed with error: %d", RET_STS);
				return MFX_ERR_NULL_PTR;
			}
			if (NULL == pData)
			{
				return MFX_ERR_NULL_PTR;
			}
			memset(pData, 0, m_OCLBufferSize);
			OCL_SAFE_CALL_NORET(RET_STS = clEnqueueUnmapMemObject(m_pOCLStruct->m_clCommandQueue, m_OCLBuffer, pData, 0, NULL, NULL));
		}
	}

	// Initialization completed
	m_bInited = true;
	//**初始化
	int pictureSize = m_PictureW * m_PictureH;
	gmmNUM = 4;

	mean = (float *)malloc(pictureSize * gmmNUM * sizeof(float));
	weight = (float *)malloc(pictureSize * gmmNUM * sizeof(float));
	varance = (float *)malloc(pictureSize * gmmNUM * sizeof(float));
	m_pK = (int *)malloc(sizeof(int) * pictureSize * gmmNUM);

	for (int i = 0; i < m_PictureH; i++)
	{
		for (int j = 0; j < m_PictureW; j++)
		{
			for (int k = 0; k < gmmNUM; k++)
			{
				/*mean[i*width*C+j*C+k] = cvRandReal(&state)*255; */
				mean[i * m_PictureW * gmmNUM + j * gmmNUM + k] = 127;
				weight[i * m_PictureW * gmmNUM + j * gmmNUM + k] = (float)1 / gmmNUM;
				//sd[i*width*C+j*C+k] = sd_init;
				varance[i * m_PictureW * gmmNUM + j * gmmNUM + k] = 80;
			}
			m_pK[i * m_PictureW + j] = 1;
		}
	}

	m_Mean = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, pictureSize * gmmNUM * sizeof(float), mean, NULL);
	m_Mweight = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, pictureSize * gmmNUM * sizeof(float), weight, NULL);
	m_Mvarance = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, pictureSize * gmmNUM * sizeof(float), varance, NULL);
	m_MpK = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(int) * pictureSize * gmmNUM, m_pK , NULL);

	m_fTherad = (float *)malloc(sizeof(float));
	*m_fTherad = 1.0;
	m_mTherad = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float), m_fTherad, NULL);

	return MFX_ERR_NONE;
}

// clear list of OCL programm
void OCLPlugin::ClearProgramList()
{
	MSDK_TRACE(__FUNCTION__);
	for (int i = 0; i < m_ProgramNum; ++i)
	{
		SAFE_FREE(m_clKernelProcessUV[i], clReleaseKernel);
		SAFE_FREE(m_clKernelProcessY[i], clReleaseKernel);
		SAFE_FREE(m_clKernelMouse[i], clReleaseKernel);
		SAFE_FREE(m_clProgram[i], clReleaseProgram);
		MSDK_SAFE_DELETE_ARRAY(m_pProgramSource[i]);
		MSDK_SAFE_DELETE_ARRAY(m_pProgramBuildLOG[i]);
	}
	m_ProgramNum = 0;
	printf("Plugin list was cleared!!!\n");
}

// init OCL stuff for given platform
mfxStatus OCLPlugin::InitOCL(msdk_char *psrcProgramName, OCLStruct *pOCLStruct)
{
	MSDK_TRACE(__FUNCTION__);
	if (m_pOCLStruct)
	{
		for (int i = 0; i < m_MaxNumTasks; ++i)
		{
			if (m_pTasks[i].bBusy)
			{
				printf("ERROR!!! could not release OCL buffers because OCL filter is wroking NOW!\n");
			}
		}

		ClearProgramList();

		SAFE_FREE(m_OCLBuffer, clReleaseMemObject);
		m_pOCLStruct = NULL;
	}
	m_pOCLStruct = pOCLStruct;
	if (m_pOCLStruct == NULL)
	{
		return MFX_ERR_NONE;
	}

	// fill list of programs
	bool bretval = UpdateProgramList(psrcProgramName);
	if (false == bretval)
	{
		printf("build binary plugin error\n");
		return MFX_ERR_UNKNOWN;
	}

	// recreate internal buffer according new image size
	SAFE_FREE(m_OCLBuffer, clReleaseMemObject);
	m_OCLBufferSize = 0;
	if (m_pOCLStruct && (m_PictureW * m_PictureH) > 0)
	{
		// create internal buffer for OCL programs initialized by 0
		assert(m_pOCLStruct->m_clContext);
		m_OCLBufferSize = sizeof(float) * m_PictureW * m_PictureH;
		OCL_SAFE_CALL(m_OCLBuffer = clCreateBuffer(m_pOCLStruct->m_clContext, CL_MEM_READ_WRITE, m_OCLBufferSize, NULL, &RET_STS));
		{
			// clear internal OCL buffer
			void *pData = NULL;
			OCL_SAFE_CALL_NORET(pData = clEnqueueMapBuffer(m_pOCLStruct->m_clCommandQueue, m_OCLBuffer, true, CL_MAP_READ, 0, m_OCLBufferSize, 0, NULL, NULL, &RET_STS));
			memset(pData, 0, m_OCLBufferSize);
			OCL_SAFE_CALL_NORET(RET_STS = clEnqueueUnmapMemObject(m_pOCLStruct->m_clCommandQueue, m_OCLBuffer, pData, 0, NULL, NULL));
		}
	}

	return MFX_ERR_NONE;
}// InitOCL

#ifdef OCL_BIN
// scan folder and recreate the list of programs if there is any change in folder
bool OCLPlugin::UpdateProgramList(msdk_char   *pcFileName)
{
	MSDK_TRACE(__FUNCTION__);
	bool flagUpdate = false;
	mfxStatus sts;

	sts = UpdateProgram(pcFileName);
	if (IMFX_ERR_NONE != sts)
	{
		return false;
	}
	SetProgram(0);
	return true;
}
#else
// scan folder and recreate the list of programs if there is any change in folder
bool OCLPlugin::UpdateProgramList(msdk_char *  pcFileName)
{
	MSDK_TRACE(__FUNCTION__);
	bool flagUpdate = false;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	//hFind = FindFirstFile(FULL_PATH_W("_*.bin"), &FindFileData);
	hFind = FindFirstFile(FULL_PATH_W("*.cl"), &FindFileData);
	//hFind = FindFirstFile(FULL_PATH_W("_gmm.cl"), &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		if(m_ProgramNum>0) flagUpdate = true;
	}
	else if(m_ProgramNum<=0)
	{
		flagUpdate = true;
	}
	else
	{
		int index=0;

		for(;;)
		{
			if( wcsncmp(m_ProgramName[index],FindFileData.cFileName,sizeof(WCHAR)*MAX_PATH) !=0 )
			{
				flagUpdate = true;
				break;
			}

			index++;
			if(!FindNextFile(hFind,&FindFileData))
			{
				if(index<m_ProgramNum)
				flagUpdate = true;
				break;
			}

			if(index>=m_ProgramNum)
			{
				flagUpdate = true;
				break;
			}
		}
		FindClose(hFind);
	}

	if(flagUpdate)
	{// new list of programms will be created
		WCHAR ProgramName[MAX_PATH];
		int     index = m_ProgramIndex;
		if(index >=0 && index < m_ProgramNum)
		{
			memcpy(ProgramName,m_ProgramName[index],sizeof(WCHAR)*MAX_PATH);
			index = -1;
		}
		else
		{
			index = 0;
		}

		ClearProgramList();
		printf("New list of pulgins is creating!\n");

		hFind = FindFirstFile(FULL_PATH_W("*.cl"), &FindFileData);
		//hFind = FindFirstFile(FULL_PATH_W("_*.bin"), &FindFileData);

		for(;hFind != INVALID_HANDLE_VALUE && m_ProgramNum < MAX_PROGRAM_NUM;)
		{
			memcpy(m_ProgramName[m_ProgramNum],FindFileData.cFileName,sizeof(WCHAR)*MAX_PATH);
			if( index<0 && wcsncmp(m_ProgramName[m_ProgramNum],ProgramName,sizeof(WCHAR)*MAX_PATH) ==0 )
			{
				index = m_ProgramNum;
			}
			m_ProgramNum++;
			// load kernel code and compile it
			UpdateProgram(m_ProgramNum-1);
			if(!FindNextFile(hFind,&FindFileData)) break;
		}
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);
		}

		if(index<0 && m_ProgramIndex>=m_ProgramNum)
		index = m_ProgramNum-1;
		SetProgram(index);
	}

	// return is list updated or not
	return flagUpdate;
}

#endif

#ifndef OCL_BIN
// check if curretn program source file is changed and rebuild it
mfxStatus OCLPlugin::UpdateProgram(int  index)
{
	MSDK_TRACE(__FUNCTION__);
	int RebuildFlag = 0;
	if(index<0) index = m_ProgramIndex;
	if(index<0 || index >= m_ProgramNum) return MFX_ERR_UNSUPPORTED;
	char* pSrc = NULL;
	size_t SrcSize = 0;

	{// read file from disk
		WCHAR* pSrcFile = m_ProgramName[index];
		ifstream is;
		is.open((exe_dir_w()+std::wstring(pSrcFile)).c_str(), ios::binary);
		if(!is.good())
		{
			printf("ERROR: Could not open cl kernel to read\n");
			return MFX_ERR_UNSUPPORTED;
		}

		is.seekg (0, ios::end);
		SrcSize = (size_t)is.tellg();
		is.seekg (0, ios::beg);

		if(SrcSize == 0)
		{
			return MFX_ERR_UNSUPPORTED;
		}

		pSrc = new char[SrcSize+1];
		if(!pSrc)
		{
			return MFX_ERR_UNSUPPORTED;
		}

		is.read(pSrc, SrcSize);
		is.close();
		pSrc[SrcSize]=0;

		if(m_pProgramSource[index]==NULL)RebuildFlag=1;

		int i;
		for(i=0;!RebuildFlag && pSrc[i] && m_pProgramSource[index][i];i++)
		{
			if(pSrc[i] != m_pProgramSource[index][i])
			{
				RebuildFlag=1;
				break;
			}
		}
		if(pSrc && m_pProgramSource[index] && pSrc[i] != m_pProgramSource[index][i])
		{
			RebuildFlag=1;
		}
	}// end read code;


	SAFE_FREE(m_clKernelProcessUV[index], clReleaseKernel);
	SAFE_FREE(m_clKernelProcessY[index], clReleaseKernel);
	SAFE_FREE(m_clKernelMouse[index], clReleaseKernel);
	SAFE_FREE(m_clProgram[index], clReleaseProgram);
	MSDK_SAFE_DELETE_ARRAY(m_pProgramBuildLOG[index]);
	MSDK_SAFE_DELETE_ARRAY(m_pProgramSource[index]);
	m_pProgramSource[index] = pSrc;

	if(NULL == m_pOCLStruct) 
	return MFX_ERR_NULL_PTR;

	cl_int errcode = 0;

	//if(strstr(m_pProgramSource[index],"//disabled")==NULL)
	{
		//const char * binary = source_.c_str();
		//size_t binarySize = source_.size();
		//m_clProgram[index] = clCreateProgramWithBinary(m_pOCLStruct->m_clContext, 1,&m_pOCLStruct->m_Devices[index],(const size_t *)&binarySize,(const unsigned char**)&binary,NULL,NULL);

		//Create the program object
		OCL_SAFE_CALL(m_clProgram[index] = clCreateProgramWithSource(m_pOCLStruct->m_clContext, 1, (const char **)&pSrc, &SrcSize, &errcode));
		//Build program
		//cl_int err = clBuildProgram(m_clProgram[index], 1, &m_pOCLStruct->m_Devices[0], m_FlagFastRelaxedMath ? "-cl-fast-relaxed-math" : NULL, NULL, NULL);
		cl_int err = clBuildProgram(m_clProgram[index], 1, &m_pOCLStruct->m_Devices[0], 
		NULL, NULL, NULL);
		
		{
			size_t paramValueSizeRet = 0;
			clGetProgramBuildInfo(m_clProgram[index], m_pOCLStruct->GetDevice(), CL_PROGRAM_BUILD_LOG, 0, NULL, &paramValueSizeRet);
			m_pProgramBuildLOG[index] = new char[paramValueSizeRet+1];
			clGetProgramBuildInfo(m_clProgram[index], m_pOCLStruct->GetDevice(), CL_PROGRAM_BUILD_LOG, paramValueSizeRet, m_pProgramBuildLOG[index], &paramValueSizeRet);
			m_pProgramBuildLOG[index][paramValueSizeRet] = 0;
			if(err!=CL_SUCCESS)printf("Build Log:\n%s\n",m_pProgramBuildLOG[index]);
			fflush(NULL);
		}
		if(err==CL_SUCCESS)
		{
			cl_int err;
			// Create kernels if kernel exist
			//if (strstr(m_pProgramSource[index],"ProcessY"))
			m_clKernelProcessY[index] = clCreateKernel(m_clProgram[index], "ProcessY", &err);
			//if(strstr(m_pProgramSource[index],"ProcessUV"))
			//	m_clKernelProcessUV[index] = clCreateKernel(m_clProgram[index], "ProcessUV", &err);
		}
	}
	return MFX_ERR_NONE;
}// UpdateKrenel
#else
// check if curretn program source file is changed and rebuild it
mfxStatus OCLPlugin::UpdateProgram(msdk_char *pSrcFile)
{
	MSDK_TRACE(__FUNCTION__);
	int RebuildFlag = 0;
	//int index=0;
	//if(index<0) index = m_ProgramIndex;
	//if(index<0 || index >= m_ProgramNum) return MFX_ERR_UNSUPPORTED;

	FILE *input = NULL;
	size_t size = 0;
	char *binary = NULL;

	MSDK_FOPEN(input, pSrcFile, MSDK_STRING("rb"));
	//input = fopen("I:\\_gmm.bin", "rb");
	if(input == NULL)
	{
		return MFX_ERR_NULL_PTR;
	}

	m_ProgramIndex = -1;
	m_ProgramNum = 1;

	fseek(input, 0L, SEEK_END);
	size = ftell(input);
	//指向文件起始位置
	rewind(input);
	binary = (char *)malloc(size);
	if (binary == NULL)
	{
		return MFX_ERR_NULL_PTR;
	}
	int rdbytes = fread(binary, sizeof(char), size, input);
	fclose(input);
	//source_.assign(binary,size);
	//delete old kernel
	int index = 0;

	SAFE_FREE(m_clKernelProcessY[index], clReleaseKernel);
	SAFE_FREE(m_clKernelProcessUV[index], clReleaseKernel);
	SAFE_FREE(m_clProgram[index], clReleaseProgram);
	MSDK_SAFE_DELETE_ARRAY(m_pProgramBuildLOG[index]);
	MSDK_SAFE_DELETE_ARRAY(m_pProgramSource[index]);
	//m_pProgramSource[index] = pSrc;

	if (m_pOCLStruct == NULL)
	{
		return MFX_ERR_NULL_PTR;
	}

	cl_int errcode = 0;
	m_clProgram[index] = clCreateProgramWithBinary(m_pOCLStruct->m_clContext, 1, &m_pOCLStruct->m_Devices[index], (const size_t *)&size, (const unsigned char **)&binary, NULL, &errcode);
	//OCL_SAFE_CALL(m_clProgram[index] = clCreateProgramWithSource(m_pOCLStruct->m_clContext, 1, (const char **)&binary, &size, &RET_STS));
	//Create the program object
	//OCL_SAFE_CALL(m_clProgram[index] = clCreateProgramWithSource(m_pOCLStruct->m_clContext, 1, (const char **)&m_pProgramSource[index], &SrcSize, &RET_STS));
	// Build program
	if(NULL == m_clProgram[index])
	{
		IMFX_ERR("clCreateProgramWithBinary failed with error: %d", errcode);
		return MFX_ERR_UNKNOWN;
	}
	else
	{
		IMFX_INFO("clCreateProgramWithBinary success");
	}

	cl_int err = clBuildProgram(m_clProgram[index], 0, NULL, m_FlagFastRelaxedMath ? "-cl-fast-relaxed-math" : NULL, NULL, NULL);
	{
		if (0 != err)
		{
			printf("************clBuildProgram  error\n");
			return MFX_ERR_UNKNOWN;
		}

		size_t paramValueSizeRet = 0;
		clGetProgramBuildInfo(m_clProgram[index], m_pOCLStruct->GetDevice(), CL_PROGRAM_BUILD_LOG, 0, NULL, &paramValueSizeRet);
		m_pProgramBuildLOG[index] = new char[paramValueSizeRet + 1];
		clGetProgramBuildInfo(m_clProgram[index], m_pOCLStruct->GetDevice(), CL_PROGRAM_BUILD_LOG, paramValueSizeRet, m_pProgramBuildLOG[index], &paramValueSizeRet);
		m_pProgramBuildLOG[index][paramValueSizeRet] = 0;
		if(err != CL_SUCCESS)printf("Build Log:\n%s\n", m_pProgramBuildLOG[index]);
		fflush(NULL);
	}

	if(err == CL_SUCCESS)
	{
		cl_int err;
		// Create kernels if kernel exist
		/*二进制，不能实现strstr(m_pProgramSource[index],"ProcessY"*/
		//if (strstr(m_pProgramSource[index],"ProcessY"))
		//{
		m_clKernelProcessY[index] = clCreateKernel(m_clProgram[index], "ProcessY", &err);
		//}
		//if(strstr(m_pProgramSource[index],"ProcessUV"))
		//{
		m_clKernelProcessUV[index] = clCreateKernel(m_clProgram[index], "ProcessUV", &err);
		//}
	}


	free(binary);
	return MFX_ERR_NONE;
}// UpdateKrenel
#endif

mfxStatus OCLPlugin::Close()
{
	if (NULL != mean)
	{
		free(mean);
		mean = NULL;
	}
	if (NULL != weight)
	{
		free(weight);
		weight = NULL;
	}
	if (NULL != varance)
	{
		free(varance);
		varance = NULL;
	}
	if (NULL != m_pK)
	{
		free(m_pK);
		varance = NULL;
	}

	if (NULL != m_Mean)
	{
		clReleaseMemObject(m_Mean);
		m_Mean = NULL;
	}
	if (NULL != m_Mweight)
	{
		clReleaseMemObject(m_Mweight);
		m_Mweight = NULL;
	}
	if (NULL != m_Mvarance)
	{
		clReleaseMemObject(m_Mvarance);
		m_Mvarance = NULL;
	}
	if (NULL != m_MpK)
	{
		clReleaseMemObject(m_MpK);
		m_MpK = NULL;
	}

	MSDK_TRACE(__FUNCTION__);
	//if(m_pOCLStruct && m_pOCLStruct->m_dx9_media_sharing)
	//{
	//	mfxStatus sts = MFX_ERR_NONE;
	//	for (int i = 0; i < m_mfxOCLINResponse.NumFrameActual; i++)
	//	{
	//		m_pOCLStruct->m_clEnqueueReleaseDX9MediaSurfacesKHR(m_pOCLStruct->m_clCommandQueue,1,&(m_pTasks[i].InBuffer.OCL_Y),0,NULL,NULL);
	//		m_pOCLStruct->m_clEnqueueReleaseDX9MediaSurfacesKHR(m_pOCLStruct->m_clCommandQueue,1,&(m_pTasks[i].InBuffer.OCL_UV),0,NULL,NULL);
	//		OCL_SAFE_CALL_NORET(RET_STS = clFinish(m_pOCLStruct->m_clCommandQueue));
	//		SAFE_FREE(m_pTasks[i].InBuffer.OCL_Y,clReleaseMemObject);
	//		SAFE_FREE(m_pTasks[i].InBuffer.OCL_UV,clReleaseMemObject);
	//	}
	//	sts = m_pMFXFrameAllocator->Free(m_pMFXFrameAllocator->pthis,&m_mfxOCLINResponse);
	//	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	//}
	// Release OpenCL Objects
	InitOCL(NULL);
	// Release MFXTask array
	MSDK_SAFE_DELETE_ARRAY(m_pTasks);
	memset(&m_mfxPluginParam, 0, sizeof(m_mfxPluginParam));

	//if(m_hDevice && m_pMFXFrameAllocator)
	//{
	//	HRESULT hr;
	//	IDirect3DDeviceManager9* pDM = ((D3DFrameAllocator*)m_pMFXFrameAllocator)->GetDeviceManager();
	//	V(pDM->CloseDeviceHandle(m_hDevice));
	//	m_hDevice = NULL;
	//}
	m_bInited = false;

	return MFX_ERR_NONE;
}

// working function that feeds OCL queue by tasks
mfxStatus OCLPlugin::EnqueueOCLTasks(MFXTask *pTask)
{

	MSDK_TRACE(__FUNCTION__);
	int Processed = 0;
	int KernelIndex = m_ProgramIndex;

	if (m_pOCLStruct == NULL)
	{
		return MFX_ERR_NULL_PTR;
	}

	//clear event
	pTask->clEventStop = NULL;

	// if(m_OCLFlag && KernelIndex>=0 && KernelIndex<m_ProgramNum)
	//{
	cl_mem    surfaces[4] = {pTask->In->OCL_Y, pTask->In->OCL_UV, pTask->Out->OCL_Y, pTask->Out->OCL_UV};
	if (m_pOCLStruct->m_dx9_media_sharing)
	{
		// look d3d10 texture
		OCL_SAFE_CALL(RET_STS = m_pOCLStruct->m_clEnqueueAcquireDX9MediaSurfacesKHR(m_pOCLStruct->m_clCommandQueue, 4, surfaces, 0, NULL, NULL));
	}

	if (m_clKernelProcessY[KernelIndex])
	{
		// run kernel to process Y components
		cl_kernel kernel = m_clKernelProcessY[KernelIndex];
		Processed = 1;

		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&pTask->In->OCL_Y));
		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&pTask->In->OCL_UV));
		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&pTask->Out->OCL_Y));
		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&pTask->Out->OCL_UV));

		if (false == isFirst)
		{
			OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 4, sizeof(cl_int), (void *)&m_PictureW));
			OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 5, sizeof(cl_int), (void *)&m_PictureH));
			OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 6, sizeof(cl_mem), (void *)&m_Mean));
			OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *)&m_Mweight));
			OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 8, sizeof(cl_mem), (void *)&m_Mvarance));
			OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 9, sizeof(cl_mem), (void *)&m_MpK));

			isFirst = true;
		}
		//OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 10, sizeof(cl_mem), (void*)&m_MOut));
		// Enqueue KernelProcessY
		OCL_SAFE_CALL(RET_STS = clEnqueueNDRangeKernel(m_pOCLStruct->m_clCommandQueue, kernel, 2, NULL, m_GlobalWorkSizeY, m_LocalWorkSizeY, 0, NULL, NULL));
	}

	if (m_clKernelProcessUV[KernelIndex])
	{
		// run kernel to process Y components
		cl_kernel kernel = m_clKernelProcessUV[KernelIndex];

		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&pTask->In->OCL_Y));
		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&pTask->In->OCL_UV));
		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&pTask->Out->OCL_Y));
		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&pTask->Out->OCL_UV));
		OCL_SAFE_CALL(RET_STS = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&m_mTherad));
		// Enqueue KernelProcessY
		OCL_SAFE_CALL(RET_STS = clEnqueueNDRangeKernel(m_pOCLStruct->m_clCommandQueue, kernel, 2, NULL, m_GlobalWorkSizeUV, m_LocalWorkSizeUV, 0, NULL, NULL));
	}

	cl_int err = clFinish(m_pOCLStruct->m_clCommandQueue);

	if (m_pOCLStruct->m_dx9_media_sharing)
	{
		// unlook d3d9 texture
		OCL_SAFE_CALL(RET_STS = m_pOCLStruct->m_clEnqueueReleaseDX9MediaSurfacesKHR(m_pOCLStruct->m_clCommandQueue, 4, surfaces, 0, NULL, NULL));
	}
	//}
	//OCL1.1 call for GPU device
	OCL_SAFE_CALL(RET_STS = clEnqueueMarker(m_pOCLStruct->m_clCommandQueue, &pTask->clEventStop));
	//OCL1.2 call
	//OCL_SAFE_CALL(RET_STS = clEnqueueMarkerWithWaitList(m_pOCLStruct->m_clCommandQueue,0,0,&pTask->clEventStop));
	// Flush queue
	OCL_SAFE_CALL(RET_STS = clFlush(m_pOCLStruct->m_clCommandQueue));

	return MFX_TASK_WORKING;
}

// Check status of OCL execution
mfxStatus OCLPlugin::QueryStatus(MFXTask *pTask)
{
	MSDK_TRACE(__FUNCTION__);
	// check event
	if (!pTask->clEventStop)
	{
		return MFX_ERR_DEVICE_FAILED;
	}
	
	cl_int sts = -1;
	size_t size_ret = 0;
	OCL_SAFE_CALL_ACT(RET_STS = clGetEventInfo(pTask->clEventStop, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(sts), &sts, &size_ret), return MFX_ERR_DEVICE_FAILED);
	if (sts < 0)
	{
		return MFX_ERR_DEVICE_FAILED;
	}
	if (sts != CL_COMPLETE)
	{
		return MFX_TASK_WORKING;
	}
	SAFE_FREE(pTask->clEventStop, clReleaseEvent);
	return MFX_TASK_DONE;
}

// function that called to start task execution and check status of execution
mfxStatus OCLPlugin::Execute(mfxThreadTask task, mfxU32 uid_p, mfxU32 uid_a)
{
	MSDK_TRACE(__FUNCTION__);
	mfxStatus       sts;

	uid_p = uid_p;

	MFXTask        *pTask = (MFXTask *)task;

	MSDK_CHECK_ERROR(m_bInited, false, MFX_ERR_NOT_INITIALIZED);
	MSDK_CHECK_POINTER(m_pmfxCore, MFX_ERR_NOT_INITIALIZED);
	MSDK_CHECK_POINTER(m_pOCLStruct, MFX_ERR_NOT_INITIALIZED);

	if (0 == uid_a)
	{
		// first call
		// get start counter
		//QueryPerformanceCounter(&(pTask->ticks[0]));
		// feed OCL queue by tasks
		sts = EnqueueOCLTasks(pTask);
	}

	// check is OCL finish working or not
	sts = QueryStatus(pTask);

	if(MFX_TASK_DONE == sts)
	{
		// OCL finished.
		//get clocks and calc execution time
		//LARGE_INTEGER F;
		//QueryPerformanceFrequency(&F);
		//QueryPerformanceCounter(&(pTask->ticks[1]));
		//float T = (float)(pTask->ticks[1].QuadPart-pTask->ticks[0].QuadPart);
		//m_TimeAver = m_TimeAver*0.95f + 0.05f*(1000.0f*T)/(float)F.QuadPart;
	}

	return sts;
}//OCLPlugin::mfxExecute

// called to submit MediaSDK task but not execute it
mfxStatus OCLPlugin::Submit(const mfxHDL *in, mfxU32 in_num, const mfxHDL *out, mfxU32 out_num, mfxThreadTask *task)
{
	MSDK_TRACE(__FUNCTION__);

	MSDK_CHECK_POINTER(in, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(out, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(*in, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(*out, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(task, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(m_pOCLStruct, MFX_ERR_NULL_PTR);
	MSDK_CHECK_NOT_EQUAL(in_num, 1, MFX_ERR_UNSUPPORTED);
	MSDK_CHECK_NOT_EQUAL(out_num, 1, MFX_ERR_UNSUPPORTED);
	MSDK_CHECK_POINTER(m_pmfxCore, MFX_ERR_NOT_INITIALIZED);
	MSDK_CHECK_ERROR(m_bInited, false, MFX_ERR_NOT_INITIALIZED);

	mfxFrameSurface1_OCL *surface_in = (mfxFrameSurface1_OCL *)in[0];
	mfxFrameSurface1_OCL *surface_out = (mfxFrameSurface1_OCL *)out[0];

	{
		// check validity of parameters
		mfxFrameInfo *pIn = &surface_in->Info;
		mfxFrameInfo *pOut = &surface_out->Info;
		if ((pIn->CropW != m_mfxVideoParam.vpp.In.CropW ||
			 pIn->CropH != m_mfxVideoParam.vpp.In.CropH ||
			 pIn->FourCC != m_mfxVideoParam.vpp.In.FourCC ||
			 pOut->CropW != m_mfxVideoParam.vpp.Out.CropW ||
			 pOut->CropH != m_mfxVideoParam.vpp.Out.CropH ||
			 pOut->FourCC != m_mfxVideoParam.vpp.Out.FourCC) && (pOut->FourCC != MFX_FOURCC_RGB4))
		{
			return MFX_ERR_INVALID_VIDEO_PARAM;
		}
	}

	// find free task in task array
	int ind;
	for (ind = 0; ind < m_MaxNumTasks; ind++)
	{
		if (!m_pTasks[ind].bBusy)
		{
			break;
		}
	}

	if (ind >= m_MaxNumTasks)
	{
		return MFX_WRN_DEVICE_BUSY; // currently there are no free tasks available
	}

	MFXTask *pTask = &m_pTasks[ind];
	//if(m_pOCLStruct->m_dx9_media_sharing)
	//{// copy input data into internal buffer
	//    IDirect3DDevice9*   pd3dDevice = NULL;
	//    HRESULT hr;
	//    IDirect3DDeviceManager9* pDM = ((D3DFrameAllocator*)m_pMFXFrameAllocator)->GetDeviceManager();
	//    if(m_hDevice==NULL)
	//    {
	//        V(pDM->OpenDeviceHandle(&m_hDevice));
	//    }
	//    V(pDM->LockDevice(m_hDevice, &pd3dDevice, true));
	//    V(pd3dDevice->StretchRect(
	//        (IDirect3DSurface9*)surface_in->Data.MemId, NULL,
	//        (IDirect3DSurface9*)pTask->InBuffer.Data.MemId, NULL,
	//        D3DTEXF_POINT));
	//    pd3dDevice->Release();
	//    V(pDM->UnlockDevice(m_hDevice, true));
	//    pTask->In = &(pTask->InBuffer);
	//}
	//else
	{// store the reference to input surface for further processing
		pTask->In = surface_in;
		m_pmfxCore->IncreaseReference(m_pmfxCore->pthis, &(surface_in->Data));
	}

	pTask->Out = surface_out;
	m_pmfxCore->IncreaseReference(m_pmfxCore->pthis, &(surface_out->Data));

	pTask->bBusy = true;
	*task = (mfxThreadTask)pTask;

	return MFX_ERR_NONE;
}// OCLPlugin::mfxSubmit

// free task and releated resources
mfxStatus OCLPlugin::FreeResources(mfxThreadTask task, mfxStatus sts)
{
	MSDK_TRACE(__FUNCTION__);
	sts = sts;
	MSDK_CHECK_ERROR(m_bInited, false, MFX_ERR_NOT_INITIALIZED);
	MSDK_CHECK_POINTER(m_pmfxCore, MFX_ERR_NOT_INITIALIZED);

	MFXTask *pTask = (MFXTask *)task;
	if (!m_pOCLStruct->m_dx9_media_sharing)
	{
		m_pmfxCore->DecreaseReference(m_pmfxCore->pthis, &(pTask->In->Data));
	}

	m_pmfxCore->DecreaseReference(m_pmfxCore->pthis, &(pTask->Out->Data));
	SAFE_FREE(pTask->clEventStop, clReleaseEvent);

	pTask->bBusy = false;

	return MFX_ERR_NONE;
}//OCLPlugin::mfxFreeResources

mfxStatus OCLPlugin::PluginInit(mfxCoreInterface *core)
{
	MSDK_TRACE(__FUNCTION__);

	MSDK_CHECK_POINTER(core, MFX_ERR_NULL_PTR);

	MSDK_SAFE_DELETE(m_pmfxCore);

	m_pmfxCore = new mfxCoreInterface;
	MSDK_CHECK_POINTER(m_pmfxCore, MFX_ERR_MEMORY_ALLOC);
	*m_pmfxCore = *core;

	return MFX_ERR_NONE;
}

mfxStatus OCLPlugin::PluginClose()
{
	MSDK_TRACE(__FUNCTION__);

	MSDK_SAFE_DELETE(m_pmfxCore);

	return MFX_ERR_NONE;
}

mfxStatus OCLPlugin::GetPluginParam(mfxPluginParam *par)
{
	MSDK_TRACE(__FUNCTION__);

	MSDK_CHECK_POINTER(par, MFX_ERR_NULL_PTR);

	*par = *(mfxPluginParam *)&m_mfxPluginParam;

	return MFX_ERR_NONE;
}

mfxStatus OCLPlugin::QueryIOSurf(mfxVideoParam *par, mfxFrameAllocRequest *in, mfxFrameAllocRequest *out)
{
	MSDK_CHECK_POINTER(par, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(in, MFX_ERR_NULL_PTR);
	MSDK_CHECK_POINTER(out, MFX_ERR_NULL_PTR);

	in->Info = par->vpp.In;
	in->NumFrameMin = 1;
	in->NumFrameSuggested = in->NumFrameMin + par->AsyncDepth;

	out->Info = par->vpp.Out;
	out->NumFrameMin = 1;
	out->NumFrameSuggested = out->NumFrameMin + par->AsyncDepth;

	return MFX_ERR_NONE;
}