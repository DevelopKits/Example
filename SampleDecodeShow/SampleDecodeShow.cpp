// SampleDecodeShow.cpp : 定义控制台应用程序的入口点。
//
#include <windows.h>
#include "stdafx.h"
#if defined(WIN64)
#include "StreamParserAPI.h"
#else
#include "MediaParserAPI.h"
#endif
#include "imfx_interface.h"
#include "MyLogger.h" 

HWND hwnd = NULL;
mfx_hdl mfx = NULL;
#pragma comment(lib,"log4cplusU.lib")
MyLogger * pMyLogger = NULL;
RAW_DATA_S raw_data;

void StreamParseFile(const char* srcFile)
{
	SP_RESULT ret = SP_SUCCESS;
	// 初始化码流解析库
	ret = SP_LoadLibrary();
	if (SP_SUCCESS != ret)
	{
		printf("Can't load parser library!!! ErrorCode = %d\n", ret);
		return ;
	}
	// 创建流分析器，设置其缓存大小为1MB
	void* hParser = SP_CreateStreamParser(1024 * 1024);
	if (NULL == hParser)
	{
		ret = SP_GetLastError(hParser);
		printf("Can't Create parser object!!! ErrorCode = %d\n", ret);
		return ;
	}
	FILE* fpSrc = fopen(srcFile, "rb");
	if (NULL == fpSrc)
	{
		printf("Can't open file: \n", fpSrc);
		return;
	}
	// 设置流方式每次输入128K数据
	const int nBufferLength = 1024 * 128;
	unsigned char pBuffer[nBufferLength];
	size_t nRead = 0;
	bool bIframeFound = false;
	// 循环从文件中读取128KB数据，直到文件结束
	while ((nRead = fread(pBuffer, 1, nBufferLength, fpSrc)) ==nBufferLength)
	{
		// 将每次从文件中读取的数据输入到流分析器中
		ret = SP_ParseData(hParser, pBuffer, nRead);
		if (SP_SUCCESS != ret)
		{
			printf("Found error while parsing file!!! ErrorCode = %d\n", ret);
			// 解析失败可能是由于数据不足等原因引起的，这里应该继续解析流程
		}

		// 因为SP_GetOneFrame的特性，循环获取帧信息
		do
		{
			// 用于存放获取到的帧信息
			SP_FRAME_INFO frameInfo = { 0 };
			ret = SP_GetOneFrame(hParser, &frameInfo);
			// 帧信息列表为空，表明已经获取了所有帧信息
			if (SP_ERROR_LIST_EMPTY == ret)
			{
				break;
			}
			else if (SP_SUCCESS != ret)
			{
				printf("Failed to get frame information!!! ErrorCode = %d\n", ret);
				break;
			}

			// 获取帧数据的地址和长度
			unsigned char* framePointer = frameInfo.framePointer;
			int			    frameLen = frameInfo.frameLen;
			// 获取裸数据的地址和长度
			unsigned char* streamPointer = frameInfo.streamPointer;
			int			    streamLen = frameInfo.streamLen;

			if (!((SP_FRAME_SUB_TYPE_VIDEO_I_FRAME == frameInfo.frameSubType&&SP_FRAME_TYPE_VIDEO == frameInfo.frameType) || bIframeFound))
			{
				continue;
			}
			if (SP_FRAME_TYPE_VIDEO != frameInfo.frameType)
			{
				continue;
			}
			bIframeFound = true;
			// TODO: 帧数据的其他操作
			IMFX_STS sts;
			FRAME_DATA_S frame_data;
			memset(&frame_data, 0, sizeof(FRAME_DATA_S));
			frame_data.enCodecID = IMFX_CODEC_AVC;
			frame_data.pcDataAddr = (mfx_i8 *)streamPointer;
			frame_data.ulDataSize = streamLen;
			frame_data.ulDataOffset = 0;
			frame_data.ulMaxLength = streamLen;
#if 0
			sts = IMFX_decode(mfx, &frame_data, 0, 0);
			if (IMFX_ERR_NONE != sts)
			{
				IMFX_ERR("IMFX_decode failed :%d\n", sts);
				LOG4CPLUS_ERROR(pMyLogger->logger, "IMFX_decode error" << sts << "frame datalen " << streamLen);
				break;
			}
			sts = IMFX_display(mfx);
			if (IMFX_ERR_NONE != sts)
			{
				IMFX_ERR("IMFX_display failed :%d\n", sts);
				LOG4CPLUS_ERROR(pMyLogger->logger, "IMFX_display error" << sts << "frame datalen " << streamLen);
				break;
			}
#else
			RECT m_rtViewport;
			GetClientRect(hwnd, &m_rtViewport);
			sts = IMFX_decode_to_system(mfx, &frame_data, m_rtViewport.right - m_rtViewport.left ,m_rtViewport.bottom - m_rtViewport.top, &raw_data);
			LOG4CPLUS_DEBUG(pMyLogger->logger, "raw_data weight =" << raw_data.ulWidth << "  height= " << raw_data.ulHeight);
			if (IMFX_ERR_NONE != sts)
			{
				IMFX_ERR("IMFX_decode_to_system failed :%d\n", sts);
				LOG4CPLUS_ERROR(pMyLogger->logger, "IMFX_decode_to_system error" << sts << "frame datalen " << streamLen);
				break;
			}
			sts = IMFX_displayEx(mfx, &raw_data);
			if (IMFX_ERR_NONE != sts)
			{
				IMFX_ERR("IMFX_displayEx failed :%d\n", sts);
				LOG4CPLUS_ERROR(pMyLogger->logger, "IMFX_displayEx error");
				break;
			}
#endif
			Sleep(40);

		} while (true);
	}

	fclose(fpSrc);
}

void releaseLibrary()
{
	// 释放码流解析库的资源
	SP_RESULT ret = SP_ReleaseLibrary();
	if (SP_SUCCESS != ret)
	{
		printf("Fail to release parser library! ErrorCode = %d\n", ret);
	}
}

void destroyStreamParser(void* handle)
{
	// 销毁流分析器，释放资源
	SP_RESULT ret = SP_Destroy(handle);
	if (SP_SUCCESS != ret)
	{
		printf("Can't destroy parser object!!! ErrorCode = %d\n", ret);
	}
	return releaseLibrary();
}

LRESULT WINAPI MyWndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam)
{
	return DefWindowProc(hwnd, msg, wparma, lparam);
}

const int screen_w = 500, screen_h = 500;



int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
	/*int n = 1;
	while (n)
	{
	Sleep(100);
	}*/
	pMyLogger = MyLogger::getInstance();
	LOG4CPLUS_TRACE(pMyLogger->logger, "WinMain");

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(wc);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpfnWndProc = (WNDPROC)MyWndProc;
	wc.lpszClassName = L"D3D";
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClassEx(&wc);


	hwnd = CreateWindow(L"D3D", L"Simplest Video Play Direct3D (Texture)", WS_OVERLAPPEDWINDOW, 100, 100, screen_w, screen_h, NULL, NULL, GetModuleHandle(NULL), NULL);
	if (hwnd == NULL){
		return -1;
	}

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

//	hwnd = (HWND)(0x00000000000A0952);
//	hwnd = (HWND)(0x000000000011088C);
	IMFX_STS sts;
	mfx = IMFX_init(IMFX_CODEC_AVC, IMFX_FOURCC_RGB4, hwnd, &sts);
	if (NULL == mfx)
	{
		IMFX_ERR("error occurred: %d\n", sts);
		LOG4CPLUS_ERROR(pMyLogger->logger, "IMFX_init error"<<sts);
		return -1;
	}

	raw_data.pcDataAddr = new mfx_i8[4000 * 3000 * 3];
	if (NULL == raw_data.pcDataAddr)
	{
		IMFX_ERR("alloc failed!\n");
		return -1;
	}
	raw_data.ulMaxLength = 4000 * 3000 * 3;
	StreamParseFile("2M.dav");


	return 0;
}

