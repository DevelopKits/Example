#ifndef INCLUDED_LIBDSL_DSTR_H
#define INCLUDED_LIBDSL_DSTR_H

#include <libdsl/dslbase.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DStr
{
public:  // һЩ��̬��������װ�������ܣ����а�ȫ��ǿ
	// ��ǿ��strcpy����֤��Խ������'\0'��β������ʵ��д�볤�ȣ�����'\0'��
	static int strcpy_x( char * dst, int dst_max_len, const char * src );

	// ��ǿ��strcat����֤��Խ������'\0'��β������ʵ��д�볤�ȣ�����'\0'��
	static int strcat_x( char * dst, int dst_max_len, const char * src );

	// ��ǿ��sprintf����֤��Խ������'\0'��β������ʵ��д�볤�ȣ�����'\0'��
	// ����֧�� len += sprintf_x( buf + len, maxlen - len, "xxxx", ... ); ������д��
	// ��ʵ��buffer����ʱ����֤\'0'������maxlen - 1����ԭ��snprintf��VC�᷵��-1�Ҳ���֤'\0'��gcc�᷵�ؼ���buffer�㹻ʱ��д�볤�ȣ�
	// ��������maxlen-1ʱ�޷����ֳ��ȸոպû��ǳ����ˣ����Լ򻯶������������߶�������������
	static int sprintf_x( char * dst, int dst_max_len, const char * fmt, ... );

	static int strcmp( const char * src, const char * dst );
	static int stricmp( const char * src, const char * dst );
	static int strcasecmp( const char * src, const char * dst );

	static int atoi( const char * src );
	static int64_t atoi64( const char * src );
	static double atod( const char * src );
	// FIXME : need buf size ? use sprintf instead ?
	static char * itoa( int val, char * buf );
	static char * i64toa( int64_t val, char * buf );
	static char * dtoa( double val, char * buf );
	static bool space(char ch); // �ж��ַ��Ƿ�Ϊ���ɼ��ַ�
	static DStr trim(const char * src, int len); // ȥ���ַ�����β�Ĳ��ɼ��ֶ�

public:
	DStr();
	explicit DStr( const char * src, int len = -1 );
	DStr( const DStr & src );
	~DStr();

	// �����㹻�ռ䣬���ܻ�Ԥ�����࣬�㹻ʱ�������
	int reserve( int len );
	bool empty() const { return m_len == 0; }
	const char * c_str() const { return m_str; }
	// FIXME : if overflow, return empty char
	char operator [] ( int pos ) const { if( pos >= 0 && pos < m_len ) return m_str[pos]; else return '\0'; }
	int length() const { return m_len; }
	// int size() const; // FIXME ? len or cap ?

public:
	DStr & assign( const DStr & str ) { return assign( str.c_str(), str.length() ); }
	DStr & assign( const char * s, int len = -1 );
	DStr & assignfmt( const char * fmt, ... );
	DStr & operator = ( const char * s );
	DStr & operator = ( const DStr & str );

	DStr & append( const DStr & str ) { return append( str.c_str(), str.length() ); }
	DStr & append( const char * s, int len = -1 );
	DStr & appendfmt( const char * fmt, ... );
	DStr & operator += ( const char * src );
	DStr & operator += ( const DStr & str ) { append( str ); return *this; }

	friend DStr operator + ( const DStr & left, const DStr & right );
	friend DStr operator + ( const DStr & left, const char * right );

	int insert( int start, const char * target );
	int insert( int start, DStr & target ) { return insert( start, target.c_str() ); }
	int erase( int start, int len );

	DStr substr( int start, int len ) const;
	// FIXME : need this ? use c_str() then strchr() or strstr()
//	int find_first_of(char target, unsigned int start=0) const;
//	int find_first_of(const char*  target, unsigned int start=0) const;

public:
	int cmp( const DStr & str ) const { return cmp( str.c_str(), -1 ); }
	int cmp( const char * s, int len = -1 ) const;
	int casecmp( const DStr & str ) const { return casecmp( str.c_str(), -1 ); }
	int casecmp( const char * str, int len = -1 ) const;

	friend bool operator == ( const DStr & left, const DStr & right );
	friend bool operator == ( const DStr & left, const char * right );
	friend bool operator != ( const DStr & left, const DStr & right );
	friend bool operator != ( const DStr & left, const char * right );
	friend bool operator < ( const DStr & left, const DStr & right );
	friend bool operator < ( const DStr & left, const char * right );
	friend bool operator <= ( const DStr & left, const DStr & right );
	friend bool operator <= ( const DStr & left, const char * right );
	friend bool operator > ( const DStr & left, const DStr & right );
	friend bool operator > ( const DStr & left, const char * right );
	friend bool operator >= ( const DStr & left, const DStr & right );
	friend bool operator >= ( const DStr & left, const char * right );

public:
	double asDouble() const;
	int64_t asInt64() const;
	int	asInt()const;

	void setValue(double val);
	void setValue(int64_t val);
	void setValue(int val);




protected:
	char * m_str;
	int m_len; 
	int m_cap;
};

bool operator == ( const DStr & left, const DStr & right );
bool operator == ( const DStr & left, const char * right );
bool operator != ( const DStr & left, const DStr & right );
bool operator != ( const DStr & left, const char * right );
bool operator < ( const DStr & left, const DStr & right );
bool operator < ( const DStr & left, const char * right );
bool operator <= ( const DStr & left, const DStr & right );
bool operator <= ( const DStr & left, const char * right );
bool operator > ( const DStr & left, const DStr & right );
bool operator > ( const DStr & left, const char * right );
bool operator >= ( const DStr & left, const DStr & right );
bool operator >= ( const DStr & left, const char * right );

DStr operator + ( const DStr & left, const DStr & right );
DStr operator + ( const DStr & left, const char * right );

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DSTR_H

