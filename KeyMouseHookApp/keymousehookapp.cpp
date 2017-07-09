#include "keymousehookapp.h"

KeyMouseHookApp::KeyMouseHookApp(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	InitHook((HWND)this->winId());

}

KeyMouseHookApp::~KeyMouseHookApp()
{
	UninitHook();
}

bool KeyMouseHookApp::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if (eventType == "windows_generic_MSG")
	{
		PMSG msg = (PMSG)message;
		if (msg->message==WM_MYKEY)
		{
			ui.m_TextEdit->insertPlainText("WM_MYKEY\n");
		}
		if (msg->message == WM_MYMOUSE)
		{
			ui.m_TextEdit->insertPlainText("WM_MYMOUSE\n");
		}

	}
	return false;
}
