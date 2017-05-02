#ifndef	INCLUDED_LIBDSL_DNETENGINE_H
#define	INCLUDED_LIBDSL_DNETENGINE_H

#include <libdsl/DRefObj.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

typedef size_t					SOCKID; // Declared socket handle
#define INVALID_SOCK_ID			(dsl::SOCKID(-1))
#define DEFAULT_TIME_OUT		3000   // ms
#define DEFAULT_SEND_BUFFER		(256 * 1024)		// 发送缓冲默认大小
#define MAX_SEND_BUFFER			(4 * 1024 * 1024)	// 发送缓冲最大值

#define MAX_SUPPORT_SOCKET		63000 // 每个engine用户能分配的最大Socket数量
#define DEFAULT_SUPPORT_SOCKET	2048
#define MAX_NET_ENGINE_THREAD	1024
#define DEFAULT_NET_ENGINE_THREAD	4

#define DEFAULT_CONNECT_TIMEOUT_MS   5000

// 字符串形式的网络地址，ipv6最多用46字节，ipv4最多用16字节，
#define DSL_NET_ADDR_LEN		46	

class DNetEngineBackend;

enum NetEngineType
{
	NET_ENGINE_INVALID = 0,
	NET_ENGINE_DEFAULT,
	NET_ENGINE_SELECT,
	NET_ENGINE_IOCP,
	NET_ENGINE_EPOLL,
};

enum NetErrorCode
{
	NETERR_NO_ERROR		= 0,
	NETERR_CREATE		= -10000000, //后6位用来表示底层错误码
	NETERR_BIND			= -11000000,
	NETERR_LISTEN		= -12000000,
	NETERR_CONNECT		= -13000000,
	NETERR_PARAM		= -14000000,
	NETERR_SOCK_LIMITED = -15000000,
	NETERR_SEND			= -16000000, 
};

struct RemoteAddrStat
{
	char	addr[DSL_NET_ADDR_LEN];
	int		port;
	bool	reachable;
};


// NOTE : DSocketHandler回调函数是由DNetEngine的内部线程发起的。
//   回调函数处理中，不应该阻塞或者进行长时间计算，以免影响其他事件的处理。
//   可以使用消息队列、任务队列等方式通知其他线程进行处理。

class DSocketHandler : public DRefInterface
{
public:
	DSocketHandler(DRefObj * refObj) : DRefInterface( refObj ) { }
	virtual ~DSocketHandler() { }

	// TcpServer接收到新链接时，调用此函数，Udp不调用。回调函数中设置*ppEvent的值为非NULL时，表示接受此连接，否则拒绝。
	// 回调函数中，可以对newClient执行Send或者CloseSocket操作，后者等价于将*ppEvent的值置NULL。
	virtual void OnConnect( SOCKID hSocket, SOCKID newClient, const char * remoteAddr, int remotePort, 
						DRefInterfacePointer<DSocketHandler> * ppEvent ) = 0;
	
	// 当TCP或者UDP接收到数据时，会回调此函数。TCP回调时，remoteAddr为NULL。
	virtual void OnData( SOCKID hSocket, const char * pBuffer, int len, const char* remoteAddr, int remotePort ) = 0;
	
	// 外部调用CloseSocket，或者内部出错（如连接断开，或者网络发送出错），会回调此函数。
	// 回调完才会将DSocketHandler的引用计数减少。
	virtual void OnClose( SOCKID hSocket, int error ) = 0;
};



class DNetEngine
{
public:
	DNetEngine();
	virtual ~DNetEngine();

	int StartEngine(NetEngineType type = NET_ENGINE_DEFAULT,int max_socket = DEFAULT_SUPPORT_SOCKET, int thread_num = DEFAULT_NET_ENGINE_THREAD );
	int StopEngine();
	
	// 每创建一个socket，都会触发一次OnClose()，再此期间DSocketHandler都会被保持引用
	SOCKID CreateSocket( DSocketHandler * pEvent, int * error = NULL );
	int CloseSocket(SOCKID hSocket);
	
	int StartTcpServer( SOCKID hSocket, const char * localAddr, int localPort );
	int StartTcpClient( SOCKID hSocket, const char * remoteAddr, int remotePort, int timeout_ms = DEFAULT_CONNECT_TIMEOUT_MS );
	int StartUDP( SOCKID hSocket, const char * localAddr, int localPort, const char * remoteAddr = NULL, int remotePort = 0 );

	// 获取socket的本地和远端地址
	int GetSockAddr( SOCKID hSocket, char ret_localAddr[DSL_NET_ADDR_LEN], int * ret_localPort, char ret_remoteAddr[DSL_NET_ADDR_LEN], int * ret_remotePort );
	static int ResolveAddr( const char * host, char ret_addr[DSL_NET_ADDR_LEN] );
	static int CheckTcpConnect(RemoteAddrStat * addressArray, int arraySize);

	// 设置引擎中待发送TCP数据缓存区的大小，
	int SetSendBufSize( SOCKID hSocket, int size );

	// 发送单个UDP包的长度不能超过1500 否则我们的基础库无法接受 
	int Send( SOCKID hSocket, const char * data, int dataLen ); // 支持TCP和UDP；对于UDP，发到创建时提供的remoteAddr中
	int SendTo( SOCKID hSocket, const char * data, int dataLen, const char * remoteAddr, int remotePort ); // udpSocket向多个不同地址发时的专用接口

private:
	DNetEngineBackend * m_pNetEngineBase;
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DNETENGINE_H
