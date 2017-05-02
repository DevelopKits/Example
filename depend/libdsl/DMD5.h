/* Copyright (c) 2014, 浙江大华技术股份有限公司, All rights reserved.
 * 2014-04
 *
 * DMD5.h : MD5 hash
 */

#ifndef INCLUDED_LIBDSL_DMD5_H
#define INCLUDED_LIBDSL_DMD5_H

#include <libdsl/dslbase.h>
#include <libdsl/DStr.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API DMD5
{
public:
	DMD5() {}
	~DMD5() {}

	void Init();
	void Update( const char * input, int inputLen );
	void Final( char digest[16] );
	void Final( DStr * digest_str ); // return 32 byte string hash

	static void Digest( const char *pszInput, int nInputSize, char pszOutPut[16] );
	static void Digest( const char *pszInput, int nInputSize, DStr * output ); // return 32 byte string hash

protected:
	static void encode( unsigned char * output, const uint32_t * input, int len );
	static void decode( uint32_t * output, const unsigned char * input, int len );
	void transform( unsigned char block[64] );

protected:
	uint32_t m_state[4];
	uint32_t m_count[2];
	unsigned char m_buffer[64]; 
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif	/* INCLUDED_LIBDSL_DMD5_H */
