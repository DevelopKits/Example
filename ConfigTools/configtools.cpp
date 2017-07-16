#include "configtools.h"

#define  GROUP_IP	"239.255.0.1"//224.0.0.0-239.255.255.255
#define  PORT		8058
#define LOCAL_IP "127.0.0.1"//�޸��Լ���ip
void  InitSocket()
{
#ifdef WIN32

#endif
}

ConfigTools::ConfigTools(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	QList<QHostAddress> AddressS = QNetworkInterface().allAddresses();
	for (int i = 0; i < AddressS.size(); i++)
	{
		QRegExp regx("((?:(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d))))");
		if (AddressS[i].protocol() == QAbstractSocket::IPv4Protocol)
		{
			if (regx.exactMatch(AddressS[i].toString()) && AddressS[i].toString() != "127.0.0.1")
			{
				ui.m_cmbIp->addItem(AddressS[i].toString());
			}
		}
	}
	connect(ui.m_btnSearch, SIGNAL(clicked()), this, SLOT(onSearch()));

	int Error;
	WORD VersionRequested;
	WSADATA WsaData;
	VersionRequested = MAKEWORD(2, 2);
	Error = WSAStartup(VersionRequested, &WsaData);
	if (Error != 0)
	{
		return;
	}
	else
	{
		if (LOBYTE(WsaData.wVersion) != 2 || HIBYTE(WsaData.wHighVersion) != 2)
		{
			WSACleanup();
			return;
		}
	}
	//�����׽���,ipv4,���ģ�udpЭ��
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);

	//Ŀ��sock��ַ����
	//IPv4�汾
	memset((char *)&m_sockDesAddress, 0, sizeof(m_sockDesAddress));
	m_sockDesAddress.sin_family = AF_INET;
	//�˿�
	m_sockDesAddress.sin_port = htons(PORT);
	//��ַ
	m_sockDesAddress.sin_addr.s_addr = inet_addr(GROUP_IP);

	
	localInterface.s_addr = inet_addr("192.168.1.108");
	int nret = setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface));
}

ConfigTools::~ConfigTools()
{

}

void ConfigTools::onSearch()
{
	int addr_len = sizeof(struct sockaddr_in);

	char bufSend[BUFSIZ] = "Test motion";
	char szbuf[BUFSIZ] = { 0 };
	int nret = sendto(m_socket, bufSend, sizeof(bufSend), 0, (struct sockaddr*)&m_sockDesAddress, sizeof(m_sockDesAddress));
	nret = recvfrom(m_socket, szbuf, sizeof(szbuf), 0, (struct sockaddr*)&m_sockDesAddress, &addr_len);
}
