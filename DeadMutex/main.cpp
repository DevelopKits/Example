#include <QtWidgets/QApplication>
#include <windows.h>

HANDLE g_hMutex = NULL;

DWORD __stdcall thread1(LPVOID lp)
{
	Sleep(10);
	WaitForSingleObject(g_hMutex,INFINITE);
	ReleaseMutex(g_hMutex);
	return 0;
}
DWORD __stdcall thread2(LPVOID lp)
{
	WaitForSingleObject(g_hMutex, INFINITE);
	while (1)
	{
		Sleep(10);
	}
	ReleaseMutex(g_hMutex);
	return 0;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//第二个参数设置为false,表示该互斥量不为该线程拥有
	g_hMutex = CreateMutex(NULL, false, NULL);

	CreateThread(NULL, 0, thread1, 0, 0, NULL);
	CreateThread(NULL, 0, thread2, 0, 0, NULL);

	return a.exec();
}
