/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* DBase64.h : base64编解码
*/

#ifndef	INCLUDED_LIBDSL_DBASE64_H
#define	INCLUDED_LIBDSL_DBASE64_H

#include <libdsl/dslbase.h>
#include <libdsl/DStr.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API DBase64
{
public:
	static DStr Encode( const DStr & src );
	static DStr Decode( const DStr & src );

	// 返回dst实际长度，dstmaxlen应该有足够空间
	static int Encode( const char * src, int srclen, char * dst, int dstmaxlen );
	static int Decode( const char * src, int srclen, char * dst, int dstmaxlen );
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DBASE64_H

