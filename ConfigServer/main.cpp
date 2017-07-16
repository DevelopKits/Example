
#include <stdio.h>

#ifdef WIN32
#include <winsock.h>
#pragma comment(lib, "Wsock32.lib")
#define LOCAL_IP "127.0.0.1"
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define  GROUP_IP	"239.255.0.1" //224.0.0.0-239.255.255.255
#define  PORT		8058

void  InitSocket()
{
#ifdef WIN32
	int Error;
	WORD VersionRequested;
	WSADATA WsaData;
	VersionRequested = MAKEWORD(2, 2);
	Error = WSAStartup(VersionRequested, &WsaData); 
	if (Error != 0)
	{
		return ;
	}
	else
	{
		if (LOBYTE(WsaData.wVersion) != 2 || HIBYTE(WsaData.wHighVersion) != 2)
		{
			WSACleanup();
			return ;
		}
	}
#endif
}


int main(int argc, char** argv)
{
	InitSocket();

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	//设置端口重用
	int reuse = 1;
	int nret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));


	struct sockaddr_in localInterface;

	memset((char *)&localInterface, 0, sizeof(localInterface));
	localInterface.sin_family = AF_INET;
	localInterface.sin_port = htons(PORT);
	localInterface.sin_addr.s_addr = INADDR_ANY;
	bind(sockfd, (struct sockaddr*)&localInterface, sizeof(localInterface));

	struct ip_mreq groupSock;

	memset((char *)&groupSock, 0, sizeof(groupSock));
	groupSock.imr_multiaddr.s_addr = inet_addr(GROUP_IP);
	groupSock.imr_interface.s_addr = INADDR_ANY;


	//设置组播发送
	nret =setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&groupSock, sizeof(groupSock));
	int nRecvLen = -1;
	const int recvSize = 1024 * 1000;
	unsigned char* databuf = new unsigned char[recvSize];
	while (1)
	{
		struct sockaddr_in from;
		int fromlen = sizeof(from);
		nRecvLen = recvfrom(sockfd, (char*)databuf, recvSize, 0, (struct sockaddr*)&from, (int*)&fromlen);
		if (nRecvLen > 0)
		{
			//	continue;
		}
		sendto(sockfd, "hello", 5, 0, (struct sockaddr*)&from, fromlen);
		printf("recv=%s\n", databuf);
	}
	return 0;
}

