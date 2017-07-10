#ifndef HOOKALLMSGBOXAPP_H
#define HOOKALLMSGBOXAPP_H

#include <QtWidgets/QWidget>
#include <qt_windows.h>
#include "ui_hookallmsgboxapp.h"
#define WM_MYMOUSE WM_USER + 306 //自定义消息，用于和主程序通信
class HooKAllMsgBoxApp : public QWidget
{
	Q_OBJECT

public:
	HooKAllMsgBoxApp(QWidget *parent = 0);
	~HooKAllMsgBoxApp();
	bool nativeEvent(const QByteArray &eventType, void *message, long *result);
	HINSTANCE g_hInst ;
public slots:
	void start();
	void stop();
	void sendMsgBox();

private:
	Ui::HooKAllMsgBoxAppClass ui;
};

#endif // HOOKALLMSGBOXAPP_H
