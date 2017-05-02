/* Copyright (c) 2011, 浙江大华技术股份有限公司, All rights reserved.
 * 2012-12
 *
 * DHttp.h : Http解析库，封装一条HTTP Message，包括body和header
 */

#ifndef INCLUDED_LIBDSL_DHTTP_H
#define INCLUDED_LIBDSL_DHTTP_H

#include <libdsl/dslbase.h>
#include <libdsl/DStr.h>
#include <libdsl/DRefObj.h>

#include <map>
#include <vector>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

enum {
	DHTTP_AUTH_UNKNOWN = 0,
	DHTTP_AUTH_BASIC_AUTHEN,   // 服务端发送给客户端的Basic认证请求
	DHTTP_AUTH_BASIC_AUTHOR,   // 客户端发送给服务端的Basic认证应答
	DHTTP_AUTH_DIGEST_AUTHEN,  // 服务端发送给客户端的Digest认证请求
	DHTTP_AUTH_DIGEST_AUTHOR,  // 客户端发送给服务端的Digest认证应答
};

class LIBDSL_API DHttpAuthInfo
{
public:
	int m_type;
	DStr m_realm;
	DStr m_username;
	// 下列字段仅用于Basic
	DStr m_password; // 密码
	// 下列字段仅用于Digest
	DStr m_nonce;    // 服务端生成的随机字串，参与计算
	DStr m_opaque;   // 服务端设置，客户端原样返回，不参与计算
	DStr m_uri;      // 请求的URI字符串，参与计算
	DStr m_qop;      // 为空，或者取值“auth”
	DStr m_nc;       // qop取值“auth”时有效，用于防止重放
	DStr m_cnonce;   // qop取值“auth”时有效，客户端生成的随机字串，参与计算
	DStr m_response; // 计算生成的应答
	DStr m_method;   // GET/PUT等方法名称，参与计算
public:
	DHttpAuthInfo() : m_type( DHTTP_AUTH_UNKNOWN ) {}
	void Reset();
	inline int Parse( const DStr & name, const DStr & value ) { return Parse( name.c_str(), value.c_str() ); }
	int Parse( const char * name, const char * value );
	static int ReadParam( const char * str, DStr * name, DStr * value );
	int Output( DStr * name, DStr * value ) const;
	
	inline int SetBasicAuthen( const DStr & realm ) { return SetBasicAuthen( realm.c_str() ); }
	int SetBasicAuthen( const char * realm );
	inline int SetBasicAuthor( const DStr & username, const DStr & password ) { return SetBasicAuthor( username.c_str(), password.c_str() ); }
	int SetBasicAuthor( const char * username, const char * password );
	inline int SetDigestAuthen( const DStr & realm, const DStr & qop = DStr(), const DStr & nonce = DStr(), const DStr & opaque = DStr() )
	{
		return SetDigestAuthen( realm.c_str(), qop.c_str(), nonce.c_str(), opaque.c_str() );
	}
	int SetDigestAuthen( const char * realm, const char * qop = NULL, const char * nonce = NULL, const char * opaque = NULL );
	// 调用SetDigestAuthor()之前，Authen相关的字段要有值，另外password要求是明文的值
	inline int SetDigestAuthor( const DStr & username, const DStr & password, const DStr & method, const DStr & uri, 
		const DStr & nc = DStr(), const DStr & cnonce = DStr() )
	{
		return SetDigestAuthor( username.c_str(), password.c_str(), method.c_str(), uri.c_str(), nc.c_str(), cnonce.c_str() );
	}
	int SetDigestAuthor( const char * username, const char * password, const char * method, const char * uri, 
			const char * nc = NULL, const char * cnonce = NULL );
	static DStr GenerateRandStr();
	// 计算Digest的HA1，即MD5（user ":" realm ":" password）
	static inline DStr GenerateHA1( const DStr & username, const DStr & realm, const DStr & password )
	{
		return GenerateHA1( username.c_str(), realm.c_str(), password.c_str() );
	}
	static DStr GenerateHA1( const char * username, const char * realm, const char * password );
	inline DStr GenerateDigest( const DStr & ha1 ) const { return GenerateDigest( ha1.c_str() ); }
	DStr GenerateDigest( const char * ha1 ) const;

	// 验证AUTHOR，对于Digest，password可以是明文或者HA1
	inline bool CheckAuthor( const DStr & username, const DStr & password ) const
	{
		return CheckAuthor( username.c_str(), password.c_str() );
	}
	bool CheckAuthor( const char * username, const char * password ) const;
};

// DHttp类的header和body是分开处理，相互不影响
// 但如果DHTTP_FLAG_UPDATE_CONTENT_LENGTH设置了（缺省有设置）
// 则当更改body长度时，会自动设置header的Content-Length的值，（反之则不然）
// 缺省启用，方便构造待发送的http message，对于parse接收到的http message，一定要显式关闭
#define DHTTP_FLAG_UPDATE_CONTENT_LENGTH	(1<<0)  

class LIBDSL_API DHttp : virtual public DRefObj
{
public:
	DHttp();
	virtual ~DHttp();

	// fname是文件名或者扩展名，如 index.html 或 html，返回minetype值：text/html
	static const char * GetFileMimeType( const char * fname );
	static const char * GetStatusStr( int status );

public:
	/**
	 * 复位数据
	 */
	virtual void Reset();
	void Clone(const DHttp& oHttp);

	int GetFlag() const { return m_flag; }
	void SetFlag( int flag ) { m_flag = flag; return; }

public: // header 的处理
	/**
	 * 反序列化数据，分析Http Header，填充DHttp结构体 
	 * @param data 输入数据的首地址
	 * @param len 输入数据的长度，如果为负数表示输入的数据为字符串，函数内部将自动搜索字符串结尾
	 * @return 负数：分析数据出错，不合法的报文
	 *            0：header包体不完整，输入的数据全部消耗，等待后续数据
	 *         正数：接收header并分析完毕，返回本次消耗的数据长度
	 */
	// DSL_DEPRECATED int fromStream(const char * data, int len = -1);
	int ParseHeader(const DStr & data);
	int ParseHeader(const char * data, int len = -1);

	/**
	 * 判断是否为请求包
	 * @return true:请求包 false:回复包
	 */
	bool IsRequest() const;

	/**
	 * 获取协议的方法
	 * @return 协议描述 如 GET POST ...
	 */
	const char * GetMethod() const;

	/**
	 * 获取URL
	 * @return URL字段
	 */
	const char * GetUrl() const;

	/**
	 * 获取状态码
	 * @return 状态码
	 */
	int GetStatus() const;

	/**
	 * 获取状态码描述字段
	 * @return 状态码描述字段
	 */
	const char * GetReason() const;

	/**
	 * 获取版本号
	 * @return 版本号
	 */
	const char * GetVersion() const;

	/**
	 * 获取头域信息
	 * @param key 头域的key
	 * @return 头域的value，如果头域不存在，则返回""。
	 */
	// const char * GetHeadParam(const char * headKey) const; // 获取头域信息
	const char * GetHeader(const char * key) const; // 获取头域信息
	bool IsHeader(const char * key)  const;
	
	/**
	 * 获取认证信息，存放在DHttpAuthInfo中
	 */
	int GetHeaderAuth( DHttpAuthInfo * auth );

	/**
	 * 设置请求包的首行数据
	 * @param requestLine 首行数据 包含 method url version 例如 POST / HTTP/1.1
	 * @return 0：成功  其他：失败
	 */

	int SetRequestLine(const char * method, const char * url, const char * version);

	/**
	 * 设置回复包的首行数据
	 * @param responseLine 首行数据 包含 version status reason 例如 HTTP/1.1 200 OK
	 * @return 0：成功  其他：失败
	 */
	// int SetResponseLine(const char * responseLine);
	int SetResponseLine(const char * version, int status, const char * reason );

	/**
	 * 设置头域属性
	 * @param headKey 头域的key
	 * @param headValue 头域的value
	 * @return 0：成功  其他：失败
	 */
	int SetHeader(const char * head, const char * value);

	/**
	 * 设置认证信息
	 */
	int SetHeaderAuth( const DHttpAuthInfo & auth );

	/**
	 * 序列化数据
	 * @param buf 输出参数，返回序列化后的数据
	 * @param maxlen 输入参数，接收缓存的大小
	 * @return > 0 序列化后的长度  其他：失败
	 */
	int OutputHeader( char * buf, int maxlen ) const;
	int OutputHeader( DStr * buf ) const;
	DStr OutputHeader() const;

public:  // body的处理
	/**
	 * 获取HTTP包体长度
	 * @return HTTP包体长度
	 */
	int GetBodyLen() const;	// 获取HTTP包体长度

	/**
	 * 获取HTTP包体
	 * @return HTTP包体的首地址
	 */
	const char * GetBody() const; // 获取HTTP包体

	/**
	 * 设置HTTP包体数据
	 * @param data 数据的源地址
	 * @param len 数据的长度，如果为负数表示输入的数据为字符串，函数内部将自动搜索字符串结尾
	 * @return 0：成功  其他：失败
	 */
	int SetBody(const char * data, int len = -1);
	int AppendBody(const char * data, int len = -1);

	int SetBodyLen( int len );
	char * GetBodyBuf();

protected:
	int parse_first_line(char * data);
	int parse_head_line(char * data);

protected:
	int 		m_flag;
	dsl::DStr	m_recvHead;		// 已收到但不完整的header数据

	bool		m_isRequest;
	dsl::DStr	m_method;
	dsl::DStr	m_url;
	dsl::DStr	m_version;
	int			m_status;
	dsl::DStr	m_reason;

	// FIXME : header的name是大小写不敏感的
	std::map<dsl::DStr, dsl::DStr, dsl::DStrCaseCompare> m_headers;

	std::vector<char> m_body;  // 存放body数据，比实际长度多1字节，保证'\0'结束
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DHTTP_H
