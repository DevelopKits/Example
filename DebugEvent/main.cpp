#include <QtWidgets/QApplication>
#include <windows.h>

HANDLE g_hEvent = NULL;

DWORD WINAPI	thread1(PVOID param)
{
	WaitForSingleObject(g_hEvent, INFINITE);
	//²Ù×÷Êý¾Ý
	ResetEvent(g_hEvent);
	return 1;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	g_hEvent = CreateEvent(NULL, true, false, NULL);

	CreateThread(NULL, 0, thread1, 0, 0, NULL);
	return a.exec();
}
