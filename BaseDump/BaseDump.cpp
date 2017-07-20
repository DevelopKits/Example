// BaseDump.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "pcap.h"
#include "remote-ext.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"Packet.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	pcap_if_t* alldevs;
	pcap_if_t *d;
	int i = 0;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	if (pcap_findalldevs_ex(PCAP_SRC_FILE_STRING, NULL, &alldevs, errbuf)==-1)
	{
		fprintf(stderr, "Error in pcap_findalldevs_ex:%s\n", errbuf);
		exit(1);
	}
	/* Print the list */
	for (d = alldevs; d != NULL; d = d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}
	if (i == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return 0;
	}

	/* We don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);
	getchar();
	return 0;
}

