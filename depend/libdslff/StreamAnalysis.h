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

class StreamAnalysis
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
