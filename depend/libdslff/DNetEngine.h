#ifndef	INCLUDED_LIBDSL_DNETENGINE_H
#define	INCLUDED_LIBDSL_DNETENGINE_H

#include <libdsl/DRefObj.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

typedef size_t					SOCKID; // Declared socket handle
#define INVALID_SOCK_ID			(dsl::SOCKID(-1))
#define DEFAULT_TIME_OUT		3000   // ms
#define DEFAULT_SEND_BUFFER		(256 * 1024)		// ���ͻ���Ĭ�ϴ�С
#define MAX_SEND_BUFFER			(4 * 1024 * 1024)	// ���ͻ������ֵ

#define MAX_SUPPORT_SOCKET		63000 // ÿ��engine�û��ܷ�������Socket����
#define DEFAULT_SUPPORT_SOCKET	2048
#define MAX_NET_ENGINE_THREAD	1024
#define DEFAULT_NET_ENGINE_THREAD	4

#define DEFAULT_CONNECT_TIMEOUT_MS   5000

// �ַ�����ʽ�������ַ��ipv6�����46�ֽڣ�ipv4�����16�ֽڣ�
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
	NETERR_CREATE		= -10000000, //��6λ������ʾ�ײ������
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


// NOTE : DSocketHandler�ص���������DNetEngine���ڲ��̷߳���ġ�
//   �ص����������У���Ӧ���������߽��г�ʱ����㣬����Ӱ�������¼��Ĵ���
//   ����ʹ����Ϣ���С�������еȷ�ʽ֪ͨ�����߳̽��д���

class DSocketHandler : public DRefInterface
{
public:
	DSocketHandler(DRefObj * refObj) : DRefInterface( refObj ) { }
	virtual ~DSocketHandler() { }

	// TcpServer���յ�������ʱ�����ô˺�����Udp�����á��ص�����������*ppEvent��ֵΪ��NULLʱ����ʾ���ܴ����ӣ�����ܾ���
	// �ص������У����Զ�newClientִ��Send����CloseSocket���������ߵȼ��ڽ�*ppEvent��ֵ��NULL��
	virtual void OnConnect( SOCKID hSocket, SOCKID newClient, const char * remoteAddr, int remotePort, 
						DRefInterfacePointer<DSocketHandler> * ppEvent ) = 0;
	
	// ��TCP����UDP���յ�����ʱ����ص��˺�����TCP�ص�ʱ��remoteAddrΪNULL��
	virtual void OnData( SOCKID hSocket, const char * pBuffer, int len, const char* remoteAddr, int remotePort ) = 0;
	
	// �ⲿ����CloseSocket�������ڲ����������ӶϿ����������緢�ͳ�������ص��˺�����
	// �ص���ŻὫDSocketHandler�����ü������١�
	virtual void OnClose( SOCKID hSocket, int error ) = 0;
};



class DNetEngine
{
public:
	DNetEngine();
	virtual ~DNetEngine();

	int StartEngine(NetEngineType type = NET_ENGINE_DEFAULT,int max_socket = DEFAULT_SUPPORT_SOCKET, int thread_num = DEFAULT_NET_ENGINE_THREAD );
	int StopEngine();
	
	// ÿ����һ��socket�����ᴥ��һ��OnClose()���ٴ��ڼ�DSocketHandler���ᱻ��������
	SOCKID CreateSocket( DSocketHandler * pEvent, int * error = NULL );
	int CloseSocket(SOCKID hSocket);
	
	int StartTcpServer( SOCKID hSocket, const char * localAddr, int localPort );
	int StartTcpClient( SOCKID hSocket, const char * remoteAddr, int remotePort, int timeout_ms = DEFAULT_CONNECT_TIMEOUT_MS );
	int StartUDP( SOCKID hSocket, const char * localAddr, int localPort, const char * remoteAddr = NULL, int remotePort = 0 );

	// ��ȡsocket�ı��غ�Զ�˵�ַ
	int GetSockAddr( SOCKID hSocket, char ret_localAddr[DSL_NET_ADDR_LEN], int * ret_localPort, char ret_remoteAddr[DSL_NET_ADDR_LEN], int * ret_remotePort );
	static int ResolveAddr( const char * host, char ret_addr[DSL_NET_ADDR_LEN] );
	static int CheckTcpConnect(RemoteAddrStat * addressArray, int arraySize);

	// ���������д�����TCP���ݻ������Ĵ�С��
	int SetSendBufSize( SOCKID hSocket, int size );

	// ���͵���UDP���ĳ��Ȳ��ܳ���1500 �������ǵĻ������޷����� 
	int Send( SOCKID hSocket, const char * data, int dataLen ); // ֧��TCP��UDP������UDP����������ʱ�ṩ��remoteAddr��
	int SendTo( SOCKID hSocket, const char * data, int dataLen, const char * remoteAddr, int remotePort ); // udpSocket������ͬ��ַ��ʱ��ר�ýӿ�

private:
	DNetEngineBackend * m_pNetEngineBase;
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL

#endif // INCLUDED_LIBDSL_DNETENGINE_H
