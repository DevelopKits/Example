/* Copyright (c) 2011, 浙江大华技术股份有限公司,  All rights reserved.
 * 2014-04
 * 
 * DSerialPort.h : 串口读写函数
 * 
 */

#ifndef	INCLUDED_LIBDSL_DSERIALPORT_H
#define	INCLUDED_LIBDSL_DSERIALPORT_H

#include"libdsl/dslbase.h"

#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#endif


BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

enum{
	FC_XONXOFF = 0x00,
	FC_DTRDSR  = 0x01,
	FC_RTSCTS  = 0x02,
};

enum{
	ASCII_XON  = 0x11,
	ASCII_XOFF = 0x13,
};

class SerialData
{
public:
	int8_t   byteSize;		// 数据位
	int8_t   stopBits;		// 停止位 0, 1, 2
	int8_t   parity;		// 奇偶校验 0:NONE 1:Odd 2:Even 3:Mark 4:Space
	uint32_t baudRate;		// 波特率
	int8_t   flowCtrl;		// 流控
	uint32_t fBinary;		// Binary Mode
	SerialData() : byteSize(8), stopBits(0), parity(0), baudRate(9600), flowCtrl(FC_XONXOFF), fBinary(1){}
};


class LIBDSL_API DSerialPort
{
public:
	DSerialPort();
	~DSerialPort();

	// serialIndex从1开始
	int OpenDev( int serialIndex, SerialData & serialData, int readTimeout );
	void CloseDev();

	// 同步读写,Read支持超时返回
	// Read时一次读取数据的最大长度为dataLen-1, 保证data以'\0'结束
	int Read( char * data, int dataLen );
	int Write( char * data, int dataLen );

	inline int GetTimeOut(){ return m_readTimeout; }
private:
	int InitDev( SerialData & serialData );

private:
	int			m_readTimeout;	// 默认读超时为100ms
#ifndef WIN32
	int			m_fdSerial;
	termios		m_oldtermios;
#else
	HANDLE		m_handleSerial;
#endif
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif //INCLUDED_LIBDSL_DSERIALPORT_H
