/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2011-09
 *
 * DCharsetConv.h : 字符集转换函数
 */

#ifndef INCLUDED_LIBDSL_DCHARSETCONV_H
#define INCLUDED_LIBDSL_DCHARSETCONV_H

#include <libdsl/dslbase.h>

#ifdef ANDROID
// android don't have iconv.h, create a fake one
typedef void * iconv_t;
inline iconv_t iconv_open( const char * tocode, const char * fromcode ) { return NULL; }
inline size_t iconv( iconv_t cd, char ** inbuf, size_t * inbytesleft, char ** outbuf, size_t * outbytesleft ) { return -1; }
inline int iconv_close( iconv_t cd ) { return -1; }
#elif ! defined(_WIN32)
#include <iconv.h>
#endif

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// 字符集转换，系统内部传输格式应该使用utf-8，
// 输入时，应该根据源格式转换成utf-8，输出时，再将utf-8转换成所需格式

enum {
	DSL_CP_UNKNOWN = 0,
	DSL_CP_UTF8,
	DSL_CP_UTF16,
	DSL_CP_UNICODE = DSL_CP_UTF16,
	DSL_CP_GB2312,
	DSL_CP_GBK,
	DSL_CP_GB18030, // 5
	DSL_CP_BIG5,

	DSL_CP_MAX
};

class LIBDSL_API DCharsetConv
{
public:
	DCharsetConv( int fromcp = DSL_CP_UTF8, int tocp = DSL_CP_UTF16 );
	~DCharsetConv();

	int SetCP( int fromcp = DSL_CP_UTF8, int tocp = DSL_CP_UTF16 );

	// 返回转换后的字符串长度，单位byte，如果出错返回负数
	// 出错时可能会有部分内容能被转换存在dst中，调用前最好先将dst清零
	int Conv( const char * src, int srclen, char * dst, int dstlen );
	
protected:
	int m_fromcp;  // 转换前字符串的编码格式
	int m_tocp;    // 转换后字符串的编码格式
#ifndef _WIN32
	iconv_t m_ic;
#endif
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_DCHARSETCONV_H

