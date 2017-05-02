/* Copyright (c) 2012, 浙江大华技术股份有限公司, All rights reserved.
 * 2013-05
 *
 * DStatus.h : 统计系统资源情况
 */

#ifndef INCLUDED_LIBDSL_DSTATUS_H
#define INCLUDED_LIBDSL_DSTATUS_H

#include <libdsl/dslbase.h>
#include <vector>

#define MAX_ADAPTER_NAME_LEN		256
#define MAX_MAC_ADDRESS_LEN			32

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

struct AdapterInfo
{
	char adapterName[MAX_ADAPTER_NAME_LEN + 4];
	char macAddr[MAX_MAC_ADDRESS_LEN];
	char hostAddr[MAX_MAC_ADDRESS_LEN];
	char subNetMask[MAX_MAC_ADDRESS_LEN];
	char gateWayAddr[MAX_MAC_ADDRESS_LEN];
};

class DStatusImp;

class LIBDSL_API DStatus
{
public:  
	DStatus();
	~DStatus();

	static DStatus * instance() { return &m_status; }

	// 获取当前进程CPU使用率 0~100
	int getCpuUsage();	

	// 获取当前进程已使用虚拟内存大小   
	int getMemoryUsage(uint64_t * mem);

	// 获取当前进程IO操作字节数
	int getIOBytes(uint64_t * readBytes, uint64_t * writeBytes);

	// 获取指定路径所在磁盘的使用信息
	int getDiskUsage(const char * dir, uint64_t * freeBytes, uint64_t * totalBytes);

	// 获取网络带宽 index 表示网卡序号 
	// sendRate表示网卡当前发送码率(单位 bit/s)
	// recvRate表示网卡当前就收码率(单位 bit/s)
	// 最小查询粒度为1秒，LINUX下返为两次获取时间段内的平均发送和接受码率
	// bandwidth表示网卡最大带宽(单位 bit/s)
	// 返回值0 表示成功 <0 表示失败
	int getNetState(int index,uint64_t * sendRate,uint64_t * recvRate,uint64_t * bandwidth);

public:
	// 获取网卡的名称,Mac、主机、网关(目前只支持windows)、子网掩码地址
	int getAdapterInfo( std::vector<AdapterInfo> &vecAdapterInfo );

	// 获取机器SN序列号 
	// windows下获取第一块网卡的Mac地址, linux下若获取序列号失败则取eth0网卡的Mac地址
	int getSerialNum( char *machineSN, int serialNumSize );
	
private:
#if !defined(WIN32) && !defined(MAC)
	// 获取当前发送和接受的总字节数
	int  getCurrentByte( );
	void getBandWidth( int index );
	int  readSerial( char *SN );
#endif

private:
	static DStatus m_status;

	DStatusImp * m_impl;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif  // INCLUDED_LIBDSL_DSTATUS_H

