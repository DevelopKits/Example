#ifndef KEYMOUSEHOOKAPP_H
#define KEYMOUSEHOOKAPP_H

#include <QtWidgets/QWidget>
#include <qt_windows.h>
#include "ui_keymousehookapp.h"
#include "KeyMouseHook.h"
class KeyMouseHookApp : public QWidget
{
	Q_OBJECT

public:
	KeyMouseHookApp(QWidget *parent = 0);
	~KeyMouseHookApp();
	bool nativeEvent(const QByteArray &eventType, void *message, long *result);
private:
	Ui::KeyMouseHookAppClass ui;
};

#endif // KEYMOUSEHOOKAPP_H
