/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
 * 2013-07
 *
 * StreamAnalysis.h : 视频流处分析类
 */

#ifndef INCLUDED_LIBDSL_STREAM_H
#define INCLUDED_LIBDSL_STREAM_H

#include <libdsl/dslbase.h>
#include <map>

enum StreamType 
{ 
	ESTREAM_TYPE_UNKNOW = -1, 
	ESTREAM_TYPE_DHAV = 0,//预留项 
	ESTREAM_TYPE_HIK = 1,//海康码流 
	ESTREAM_TYPE_HANBANG = 2,//汉邦码流 
	ESTREAM_TYPE_UNVIEW = 3,//宇视码流 
	ESTREAM_TYPE_3RD_PS = 4,//国标PS流 
	ESTREAM_TYPE_XINCH = 5,//信产码流 
	ESTREAM_TYPE_LIYUAN = 6,//立元码流 
	ESTREAM_TYPE_BIT = 7,//比特码流 
	ESTREAM_TYPE_3RD_RAW,//其他第三方码流 
};

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

// NOTE : 新的媒体分析库（MediaParser）已经支持第三方厂家类型分析，因此本类废弃

class LIBDSL_API DSL_DEPRECATED StreamAnalysis
{
public:
	StreamAnalysis();
    ~StreamAnalysis();

public:
	int GetExtendheadStreamtype(unsigned char *data, int len);

private:
	void InitLenTable();
private:
	std::map<int,int> _lenTable;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_STREAM_H
