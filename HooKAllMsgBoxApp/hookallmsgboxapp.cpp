#include "hookallmsgboxapp.h"
#include <tchar.h>
HooKAllMsgBoxApp::HooKAllMsgBoxApp(QWidget *parent)
	: QWidget(parent)
{
	g_hInst = NULL;
	ui.setupUi(this);
	connect(ui.m_btnStart, SIGNAL(clicked()), this, SLOT(start()));
	connect(ui.m_btnstop, SIGNAL(clicked()), this, SLOT(stop()));
	connect(ui.m_btnMsg, SIGNAL(clicked()), this, SLOT(sendMsgBox()));
	
}

HooKAllMsgBoxApp::~HooKAllMsgBoxApp()
{
	
}

bool HooKAllMsgBoxApp::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if (eventType == "windows_generic_MSG")
	{
		PMSG msg = (PMSG)message;
		
		if (msg->message == WM_MYMOUSE)
		{
			TCHAR wndTitle[256];
			::GetWindowText((HWND)msg->lParam, wndTitle, 256);
			QString ret = QString::fromWCharArray(wndTitle);
			ui.m_TextEdit->insertPlainText(ret);
			ui.m_TextEdit->insertPlainText("\n");
		}

	}
	return false;
}

void HooKAllMsgBoxApp::start()
{
	g_hInst = ::LoadLibrary(_T("HookAllMsgBox.dll"));
	typedef int(__stdcall * InitHook)(HWND hWnd);
	InitHook Hook;
	Hook = (InitHook)::GetProcAddress(g_hInst, "InitHook");
	Hook((HWND)this->winId());
}

void HooKAllMsgBoxApp::stop()
{
	typedef int(__stdcall* StopHook)();
	StopHook UnHook;
	UnHook = (StopHook)::GetProcAddress(g_hInst, "UninitHook");
	UnHook();
	FreeLibrary(g_hInst);
	g_hInst = NULL;
}

void HooKAllMsgBoxApp::sendMsgBox()
{
	::MessageBox(NULL, TEXT("Msg bBox"), TEXT("MSG"), MB_ICONINFORMATION);
}
