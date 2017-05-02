#ifndef INCLUDED_LIBDSL_DHTTP_H
#define INCLUDED_LIBDSL_DHTTP_H

#include <libdsl/dslbase.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class DHttp
{
public:
	DHttp();
	~DHttp();
	/**
	 * 反序列化数据 填充DHttp结构体 
	 * 同一个Pdu可以分多次输入，但是必须保证第一个输入包中HTTP包头是完整的
	 * @param data 输入数据的首地址
	 * @param len 输入数据的长度，如果为负数表示输入的数据为字符串，函数内部将自动搜索字符串结尾
	 * @return 负数：body头不完整 输入的数据无消耗 0：body包体不完整 输入的数据全部消耗 正数：接收完毕 返回消耗的数据长度
	 * 当返回0时 就可以读取HTTP头域里的参数
	 */
	int fromStream(const char * data, int len = -1);

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
	 * @param headKey 头域的key
	 * @return 头域的value
	 */
	const char * GetHeadParam(const char * headKey) const; // 获取头域信息

	/**
	 * 获取HTTP包体
	 * @return HTTP包体的首地址
	 */
	const char * GetBody() const; // 获取HTTP包体

	/**
	 * 获取HTTP包体长度
	 * @return HTTP包体长度
	 */
	int GetBodyLen() const;	// 获取HTTP包体长度


	/**
	 * 设置请求包的首行数据
	 * @param requestLine 首行数据 包含 method url version 例如 POST / HTTP/1.1
	 * @return 0：成功  其他：失败
	 */
	int SetRequestLine(const char * requestLine);


	/**
	 * 设置回复包的首行数据
	 * @param responseLine 首行数据 包含 version status reason 例如 HTTP/1.1 200 OK
	 * @return 0：成功  其他：失败
	 */
	int SetResponseLine(const char * responseLine);

	/**
	 * 设置头域属性
	 * @param headKey 头域的key
	 * @param headValue 头域的value
	 * @return 0：成功  其他：失败
	 */
	int SetHeadParam(const char * headKey, const char * headValue);

	/**
	 * 设置HTTP包体数据
	 * @param data 数据的源地址
	 * @param len 数据的长度，如果为负数表示输入的数据为字符串，函数内部将自动搜索字符串结尾
	 * @return 0：成功  其他：失败
	 */
	int SetBody(const char * data, int len = -1);

	/**
	 * 序列化数据
	 * @param len 输出参数，返回序列化后的数据长度
	 * @return 0：成功  其他：失败
	 */
	const char* toStream(int &len);


	/**
	 * 复位数据
	 */
	void Reset();

protected:
	class DHttpImpl;
	class DHttpImpl * m_impl;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DHTTP_H

