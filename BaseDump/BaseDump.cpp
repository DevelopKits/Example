// BaseDump.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "pcap.h"
#include "remote-ext.h"
#include <string>
using namespace  std;
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"Packet.lib")

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

int _tmain(int argc, _TCHAR* argv[])
{
	pcap_if_t* alldevs;
	pcap_if_t *d;
	int nDevNum = 0;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf)==-1)
	{
		fprintf(stderr, "Error in pcap_findalldevs_ex:%s\n", errbuf);
		exit(1);
	}
	/* Print the list */
	for (d = alldevs; d != NULL; d = d->next)
	{
		printf("%d. %s", ++nDevNum, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}
	if (nDevNum == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return 0;
	}
	int inum = 1;
	/*printf("Enter the interface number (1-%d):", nDevNum);
	scanf("%d", &inum);*/

	int  i = 0;
	for (d = alldevs, i = 0;i < inum - 1; d = d->next, i++);

	pcap_t* adhandle;
	/* Open the device */
	if ((adhandle = pcap_open_live(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, errbuf)) == NULL)
	{
		fprintf(stderr, "\nError opening adapter\n");
		return -1;
	}

	u_int netmask;
	if (d->addresses != NULL)
		/* Retrieve the mask of the first address of the interface */
		netmask = ((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* If the interface is without an address we suppose to be in a C class network */
		netmask = 0xffffff;

	struct bpf_program fcode;
	char packet_filter[] = "udp port 8000";
	//compile the filter  
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) < 0)
	{
		fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	//set the filter  
	if (pcap_setfilter(adhandle, &fcode) < 0)
	{
		fprintf(stderr, "\nError setting the filter.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* We don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);

	pcap_dumper_t *dumpfile;

	char strExePathTmp[260] = { 0 };
	GetModuleFileNameA(NULL, strExePathTmp, 260);
	string strExePath(strExePathTmp);
	strExePath.resize(strExePath.find_last_of('\\'));
	SetCurrentDirectoryA(strExePath.c_str());

	time_t     nNowTime;
	struct tm* pcTimeStruct;
	time(&nNowTime);
	pcTimeStruct = localtime(&nNowTime);
	char RecPath[MAX_PATH] = { 0 };

	sprintf(RecPath, "%04d%02d%02d.pcap",  pcTimeStruct->tm_year + 1900, pcTimeStruct->tm_mon + 1, pcTimeStruct->tm_mday);

	dumpfile = pcap_dump_open(adhandle, RecPath);
	if (dumpfile == NULL)
	{
		fprintf(stderr, "\nError opening output file\n");
		return -1;
	}
	pcap_loop(adhandle, 0, packet_handler, (unsigned char *)dumpfile);

	pcap_close(adhandle);
	getchar();
	return 0;
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *dumpfile, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	/* save the packet on the dump file */
	pcap_dump(dumpfile, header, pkt_data);
}
