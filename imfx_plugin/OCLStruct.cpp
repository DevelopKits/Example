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
#if defined(WIN32) || defined(WIN64)
#include <string.h>
#include "OCLStruct.h"
#include "vm/thread_defs.h"

#pragma warning(disable : 4996)

static MSDKMutex m_Mutex4OCL;

// initialization OpenCL stuff
void OCLStruct::OCLInit(IDirect3DDevice9Ex *pd3dDevice)
{
	//this sample support only GPU device
	const cl_device_type  ReqDeviceType = CL_DEVICE_TYPE_GPU;
	// init variables by 0
	m_NumDevices = 0;
	m_clContext = 0;
	m_clCommandQueue = 0;

	{// Get Intel platforms (MAX_PLATFORM_NUM maximum)
		int index = 0;
		m_PlatformNum = MAX_PLATFORM_NUM;

		m_Mutex4OCL.Lock();
		OCL_SAFE_CALL_NORET(RET_STS = clGetPlatformIDs(m_PlatformNum, m_PlatformIDs, &m_PlatformNum));
		m_Mutex4OCL.Unlock();

		// GetNames
		for (cl_uint i = 0; i < m_PlatformNum; i++)
		{
			OCL_SAFE_CALL_NORET(RET_STS = clGetPlatformInfo(m_PlatformIDs[i], CL_PLATFORM_NAME, sizeof(m_PlatformNames[index]), m_PlatformNames[index], NULL));
			if(strstr(m_PlatformNames[index],"Intel"))
			{//keep only Intel platfroms
				printf("OpenCL platform \"%s\" is used\n",m_PlatformNames[index]);
				m_PlatformIDs[index] = m_PlatformIDs[i];
				index++;
			}
		}
		m_PlatformNum = index;
	}
	if(m_PlatformNum<=0)
	{
		printf("ERROR: Could not find Intel platform\n");
		return;
	}

	// choose first platform
	m_PlatformCurrent = 0;

	cl_platform_id  clPlatform = m_PlatformIDs[m_PlatformCurrent];
	m_dx9_media_sharing = InitDX9MediaFunctions(clPlatform);

	{// detect if D3D9 sharing is able for platform.
		char    str[1024*4];
		size_t  len = 0;
		OCL_SAFE_CALL_NORET(RET_STS = clGetPlatformInfo(clPlatform,CL_PLATFORM_EXTENSIONS,sizeof(str),str,&len));
		printf("Platform extensions: %s\n",str);
		if(NULL==strstr(str,"cl_khr_dx9_media_sharing"))
		{
			m_dx9_media_sharing = 0;
		}
	}

	// Get devices and create context
	m_NumDevices = 0;
	if (m_dx9_media_sharing && pd3dDevice)
	{// get device and create context for DXVA sharing
		const int AdapterNum = 1;
		cl_dx9_media_adapter_type_khr   pAdapterTypes[AdapterNum] = {CL_ADAPTER_D3D9EX_KHR};
		void* pAdapters[AdapterNum] = {pd3dDevice};
		OCL_SAFE_CALL_NORET(RET_STS = m_clGetDeviceIDsFromDX9MediaAdapterKHR(
			clPlatform,
			AdapterNum,
			pAdapterTypes,
			pAdapters,
			CL_ALL_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR,
			sizeof(m_Devices)/sizeof(m_Devices[0]),
			m_Devices,
			&m_NumDevices));

		cl_uint j = 0;

		for (cl_uint i = 0;i < m_NumDevices; i++)
		{//keep only req type
			cl_device_type type;
			OCL_SAFE_CALL_NORET(RET_STS = clGetDeviceInfo(m_Devices[i],CL_DEVICE_TYPE,sizeof(cl_device_type),&type,NULL));
			if(type != ReqDeviceType) 
			{
				continue;
			}
			else
			{
				m_Devices[j++] = m_Devices[i];
			}
		}

		m_NumDevices = j;
		if(m_NumDevices<=0)
		{
			printf("ERROR: Could not find Intel GPU device for media sharing\n");
			return;
		}
		const cl_context_properties props[] = {CL_CONTEXT_ADAPTER_D3D9EX_KHR,(cl_context_properties)pd3dDevice,NULL};
		OCL_SAFE_CALL_ACT(m_clContext = clCreateContext(props, m_NumDevices, m_Devices, NULL, NULL, &RET_STS),m_dx9_media_sharing=0);
	}
	else
	{// create context as without D3D9 device connection
		OCL_SAFE_CALL_NORET(RET_STS = clGetDeviceIDs(clPlatform, ReqDeviceType, sizeof(m_Devices)/sizeof(m_Devices[0]), m_Devices, &m_NumDevices));
		if(m_NumDevices<=0)
		{
			printf("ERROR: Could not find Intel GPU device\n");
			return;
		}
		OCL_SAFE_CALL_NORET(m_clContext = clCreateContext(0, m_NumDevices, m_Devices, NULL, NULL, &RET_STS));
	}

	// Create command queue for device
	OCL_SAFE_CALL_NORET(m_clCommandQueue = clCreateCommandQueue(m_clContext, m_Devices[0], CL_QUEUE_PROFILING_ENABLE, &RET_STS));

	printf("m_dx9_media_sharing = %d\n",m_dx9_media_sharing);
}

// release OpenCL stuff
//OCLStruct::~OCLStruct()
//{
//    if(m_clCommandQueue)
//    {
//        OCL_SAFE_CALL_NORET(RET_STS=clFinish(m_clCommandQueue));
//        OCL_SAFE_CALL_NORET(RET_STS=clReleaseCommandQueue(m_clCommandQueue));
//    }
//    SAFE_FREE(m_clContext,clReleaseContext);
//}
#endif