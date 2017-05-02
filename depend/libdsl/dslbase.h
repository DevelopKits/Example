/* Copyright (c) 2011, 浙江大华技术股份有限公司
 * All rights reserved.
 *
 * 文件名称：dslbase.h
 * 摘要：基础库的通用定义
 *
 */

#ifndef INCLUDED_LIBDSL_DSLBASE_H
#define INCLUDED_LIBDSL_DSLBASE_H

// BEGIN -- 跨平台相关的定义，使用者可以跳过

#ifdef _WIN32

#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int int16_t;
typedef unsigned short int uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif

#include <winsock2.h>
#include <windows.h>

// 放在class关键字后面，以及函数或者变量定义的前面
#define DSL_DEPRECATED	__declspec(deprecated)

#if defined(LIBDSL_EXPORTS)
#define LIBDSL_API __declspec(dllexport)
#else
#define LIBDSL_API __declspec(dllimport)
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4251) /* class 'A' needs to have dll interface for to be used by clients of class 'B'. */
#endif

#else

#include <stddef.h> // NULL
#include <stdint.h>
#include <errno.h>

// 放在class关键字后面，以及函数或者变量定义的前面，（gcc实际可放置的地方更灵活）
#define DSL_DEPRECATED	__attribute__ ((deprecated))

#define LIBDSL_API

#endif

// END -- 跨平台相关的定义，使用者可以跳过

extern "C" LIBDSL_API const char * GetLibInfo_libdsl();
extern "C" LIBDSL_API const char * GetLibInfo_libdsl_1_3_X();

#define BEGIN_NAMESPACE_DSL namespace dsl {
#define END_NAMESPACE_DSL }

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// 函数一般返回0成功，返回负数表示失败
// 一些通用的错误码，小于100，模块
#define DSL_ERROR_OK			0	// 成功
#define DSL_ERROR_FAILED		-11	// 常规错误码，执行失败
#define DSL_ERROR_INVALID_PARAM	-12	// 参数错误

#define SAFE_DELETE(obj)		do { if(obj) {delete obj; obj = NULL; } } while(0)
#define SAFE_M_DELETE(obj)		do { if(obj) {delete [] obj; obj = NULL; } } while(0)
#define SAFE_RELEASE(obj)		do { if(obj) {obj->release(); obj = NULL;} } while(0)
// #define SAFE_RET_CALL(fun)		{int nErr = (fun); if(nErr != DSL_ERROR_OK) return nErr;}

class LIBDSL_API DBaseLib
{
public:
	static const char * GetLibInfo();
	static const char * GetLibInfo_1_3_X();

	// 可以重复Init，会自动记录调用次数，需要调用通用多次的Uninit
	// szLogFile为NULL表示不改变日志文件，不会关闭日志，需要关闭可调用DLOG_SET_FILE(NULL)
	static int Init(const char* szLogFile);
	static int Uninit();

protected:
	static int m_ref;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

namespace DahuaSoftwareLine = dsl;

#endif // INCLUDED_LIBDSL_DSLBASE_H

