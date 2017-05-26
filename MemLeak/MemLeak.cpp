#include "MemLeak.h"
#include <windows.h>

MemLeak::MemLeak()
{
}


MemLeak::~MemLeak()
{
}

void MemLeak::run()
{
	int n = 1000;
	while (n--)
	{
		char* szbuf = new char[1024*10];
	}
	while (1)
	{
		Sleep(1000);
	}
}
