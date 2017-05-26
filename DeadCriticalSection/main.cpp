#include <QtWidgets/QApplication>
#include <windows.h>

CRITICAL_SECTION cs1;
CRITICAL_SECTION cs2;
DWORD __stdcall thread1(LPVOID lp)
{
	EnterCriticalSection(&cs1);
	Sleep(10);
	EnterCriticalSection(&cs2);
	return 0;
}
DWORD __stdcall thread2(LPVOID lp)
{
	EnterCriticalSection(&cs2);
	Sleep(10);
	EnterCriticalSection(&cs1);
	return 0;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	InitializeCriticalSection(&cs1);
	InitializeCriticalSection(&cs2);
	CreateThread(NULL, 0, thread1, 0, 0, NULL);
	CreateThread(NULL, 0, thread2, 0, 0, NULL);

	return a.exec();
}
