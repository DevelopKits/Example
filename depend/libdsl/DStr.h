/* Copyright (c) 2012, 浙江大华技术股份有限公司, All rights reserved.
 * 2012-09
 *
 * DStr.h : 字符串处理函数
 */

#ifndef INCLUDED_LIBDSL_DSTR_H
#define INCLUDED_LIBDSL_DSTR_H

#include <libdsl/dslbase.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API DStr
{
public:  // 一些静态函数，封装基础功能，进行安全增强
	// 增强的strcpy，保证不越界且以'\0'结尾，返回实际写入长度（不含'\0'）
	static int strcpy_x( char * dst, int dst_max_len, const char * src );

	// 增强的strcat，保证不越界且以'\0'结尾，返回实际写入长度（不含'\0'）
	static int strcat_x( char * dst, int dst_max_len, const char * src );

	// 增强的sprintf，保证不越界且以'\0'结尾，返回实际写入长度（不含'\0'）
	// 方便支持 len += sprintf_x( buf + len, maxlen - len, "xxxx", ... ); 的连续写法
	// 当实际buffer不够时，保证\'0'，返回maxlen - 1，（原版snprintf，VC会返回-1且不保证'\0'，gcc会返回假设buffer足够时的写入长度）
	// 当但返回maxlen-1时无法区分长度刚刚好还是出错了，可以简化都当出错处理，或者都当不出错不处理
	static int sprintf_x( char * dst, int dst_max_len, const char * fmt, ... );

	// 字符串拷贝，*不*保证最后以'\0'结尾，最多只复制max_copy_len个字节，如果src长度不足，剩余空间以'\0'填充
	static char * strncpy( char * dst, const char * src, int max_copy_len );

	static int strcmp( const char * src, const char * dst );
	static int strncmp( const char * src, const char * dst, int len );
	static int stricmp( const char * src, const char * dst );
	static int strnicmp( const char * src, const char * dst, int len );
	static int strcasecmp( const char * src, const char * dst );
	static int strncasecmp( const char * src, const char * dst, int len );

	static int atoi( const char * src );
	static int64_t atoi64( const char * src );
	static double atod( const char * src );
	// FIXME : need buf size ? use sprintf instead ?
	static char * itoa( int val, char * buf );
	static char * i64toa( int64_t val, char * buf );
	static char * dtoa( double val, char * buf );
	static bool space(char ch); // 判断字符是否为不可见字符
	static DStr trim(const char * src, int len); // 去掉字符串首尾的不可见字段

public:
	DStr();
	explicit DStr( const char * src, int len = -1 );
	DStr( const DStr & src );
	~DStr();

	// 保留足够空间，可能会预留更多，足够时不会减少
	int reserve( int len );
	inline bool empty() const { return m_len == 0; }
	inline const char * c_str() const { return m_str; }
	// FIXME : if overflow, return empty char
	inline char operator [] ( int pos ) const { if( pos >= 0 && pos < m_len ) return m_str[pos]; else return '\0'; }
	inline int length() const { return m_len; }
	// int size() const; // FIXME ? len or cap ?

public:
	inline DStr & assign( const DStr & str ) { return assign( str.c_str(), str.length() ); }
	DStr & assign( const char * s, int len = -1 );
	DStr & assignfmt( const char * fmt, ... );
	inline DStr & operator = ( const char * s ) { return assign( s ); }
	inline DStr & operator = ( const DStr & str ) { return assign( str ); }

	inline DStr & append( const DStr & str ) { return append( str.c_str(), str.length() ); }
	DStr & append( const char * s, int len = -1 );
	DStr & appendfmt( const char * fmt, ... );
	inline DStr & operator += ( const char * src ) { return append( src ); }
	inline DStr & operator += ( const DStr & str ) { return append( str ); }

	friend LIBDSL_API DStr operator + ( const DStr & left, const DStr & right );
	friend LIBDSL_API DStr operator + ( const DStr & left, const char * right );

	int insert( int start, const char * target );
	inline int insert( int start, DStr & target ) { return insert( start, target.c_str() ); }
	int erase( int start, int len );

	DStr substr( int start, int len ) const;
	// 返回找到的字符串的位置，找不到返回-1
	int findstr( const char * str ) const;
	int findcasestr( const char * str ) const;
	// 返回找到的字符的位置，找不到返回-1
	int findchar( int ch ) const;
	// 返回从字符串开头，出现的accept_chs中的字符的个数，没有就返回0
	int countaccept( const char * accept_chs ) const;
	// 返回从字符串开头，不出现的reject_chs中的字符的个数，如果第一个字符就在chs中，则返回0
	int countreject( const char * reject_chs ) const;

public:
	inline int cmp( const DStr & str ) const { return cmp( str.c_str(), -1 ); }
	int cmp( const char * s, int len = -1 ) const;
	inline int casecmp( const DStr & str ) const { return casecmp( str.c_str(), -1 ); }
	int casecmp( const char * str, int len = -1 ) const;

	friend LIBDSL_API bool operator == ( const DStr & left, const DStr & right );
	friend LIBDSL_API bool operator == ( const DStr & left, const char * right );
	friend LIBDSL_API bool operator != ( const DStr & left, const DStr & right );
	friend LIBDSL_API bool operator != ( const DStr & left, const char * right );
	friend LIBDSL_API bool operator < ( const DStr & left, const DStr & right );
	friend LIBDSL_API bool operator < ( const DStr & left, const char * right );
	friend LIBDSL_API bool operator <= ( const DStr & left, const DStr & right );
	friend LIBDSL_API bool operator <= ( const DStr & left, const char * right );
	friend LIBDSL_API bool operator > ( const DStr & left, const DStr & right );
	friend LIBDSL_API bool operator > ( const DStr & left, const char * right );
	friend LIBDSL_API bool operator >= ( const DStr & left, const DStr & right );
	friend LIBDSL_API bool operator >= ( const DStr & left, const char * right );

public:
	double asDouble() const;
	int64_t asInt64() const;
	int	asInt()const;

	void setValue(double val);
	void setValue(int64_t val);
	void setValue(int val);

	void tolower();
	void toupper();

protected:
	char * m_str;
	int m_len; 
	int m_cap;
};

LIBDSL_API bool operator == ( const DStr & left, const DStr & right );
LIBDSL_API bool operator == ( const DStr & left, const char * right );
LIBDSL_API bool operator != ( const DStr & left, const DStr & right );
LIBDSL_API bool operator != ( const DStr & left, const char * right );
LIBDSL_API bool operator < ( const DStr & left, const DStr & right );
LIBDSL_API bool operator < ( const DStr & left, const char * right );
LIBDSL_API bool operator <= ( const DStr & left, const DStr & right );
LIBDSL_API bool operator <= ( const DStr & left, const char * right );
LIBDSL_API bool operator > ( const DStr & left, const DStr & right );
LIBDSL_API bool operator > ( const DStr & left, const char * right );
LIBDSL_API bool operator >= ( const DStr & left, const DStr & right );
LIBDSL_API bool operator >= ( const DStr & left, const char * right );

LIBDSL_API DStr operator + ( const DStr & left, const DStr & right );
LIBDSL_API DStr operator + ( const DStr & left, const char * right );

// 不区分大小写的比较算子，可用于std::map等
class LIBDSL_API DStrCaseCompare
{
public:
	bool operator () ( const DStr & s1, const DStr & s2 ) const { return s1.casecmp( s2 ) < 0; }
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DSTR_H

