#include "MsTimer.h"
#include "MyLogger.h"

MyLogger * pMyLogger = NULL;

VOID Test(IN unsigned int ulTimerID, IN unsigned int ulmsg, IN ULONG ulUser, IN unsigned int ul1, IN unsigned int ul2)
{
	DEBUG_LOG("ulTimerID");
}

int main()
{
	pMyLogger = MyLogger::getInstance();
	MsTimer* timer = new MsTimer;
	timer->OSTIME_TimeStart();
	timer->OstimeSetEvent(30, 0, Test, NULL,OSTIME_PERIODIC);
	int n = 5;
	while (n--)
	{
		Sleep(1000 * 1);
	}
	timer->OSTIME_TimeStop();
	return 0;
}