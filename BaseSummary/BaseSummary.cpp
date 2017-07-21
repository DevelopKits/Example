// BaseSummary.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "pcap.h"
#include "remote-ext.h"
#include <string>
using namespace  std;
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"wpcap.lib")
#pragma comment(lib,"Packet.lib")

void dispatcher_handler(u_char *, const struct pcap_pkthdr *, const u_char *);

void _tmain(int argc, _TCHAR* argv[])
{
	pcap_if_t* alldevs;
	pcap_if_t *d;
	int nDevNum = 0;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
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
		return ;
	}
	int inum = 1;
	/*printf("Enter the interface number (1-%d):", nDevNum);
	scanf("%d", &inum);*/

	int  i = 0;
	for (d = alldevs, i = 0; i < inum - 1; d = d->next, i++);

	pcap_t* adhandle;
	/* Open the device */
	if ((adhandle = pcap_open_live(d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, errbuf)) == NULL)
	{
		fprintf(stderr, "\nError opening adapter\n");
		return ;
	}

	u_int netmask;
	if (d->addresses != NULL)
		/* Retrieve the mask of the first address of the interface */
		netmask = ((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* If the interface is without an address we suppose to be in a C class network */
		netmask = 0xffffff;

	/* Put the interface in statstics mode */
	if (pcap_setmode(adhandle, MODE_STAT) < 0)
	{
		fprintf(stderr, "\nError setting the mode.\n");
		pcap_close(adhandle);
		/* Free the device list */
		return;
	}

	/* We don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);

	/* Start the main loop */
	struct timeval st_ts;
	pcap_loop(adhandle, 0, dispatcher_handler, (PUCHAR)&st_ts);

	pcap_close(adhandle);
	return ;
}
void dispatcher_handler(u_char *state, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct timeval *old_ts = (struct timeval *)state;
	u_int delay;
	LARGE_INTEGER Bps, Pps;
	struct tm ltime;
	char timestr[16];
	time_t local_tv_sec;

	/* Calculate the delay in microseconds from the last sample. */
	/* This value is obtained from the timestamp that the associated with the sample. */
	delay = (header->ts.tv_sec - old_ts->tv_sec) * 1000000 - old_ts->tv_usec + header->ts.tv_usec;
	/* Get the number of Bits per second */
	Bps.QuadPart = (((*(LONGLONG*)(pkt_data + 8)) * 8 * 1000000) / (delay));
	/*                                            ^      ^
	|      |
	|      |
	|      |
	converts bytes in bits --       |
	|
	delay is expressed in microseconds --
	*/

	/* Get the number of Packets per second */
	Pps.QuadPart = (((*(LONGLONG*)(pkt_data)) * 1000000) / (delay));

	/* Convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	localtime_s(&ltime, &local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", &ltime);

	/* Print timestamp*/
	printf("%s ", timestr);

	/* Print the samples */
	printf("BPS=%I64u ", Bps.QuadPart);
	printf("PPS=%I64u\n", Pps.QuadPart);

	//store current timestamp  
	old_ts->tv_sec = header->ts.tv_sec;
	old_ts->tv_usec = header->ts.tv_usec;
}
