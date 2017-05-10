#pragma once
#include <QDialog>
#include <QWidget>
#include "ui_MyMessage.h"
class MyMessage:public QDialog
{
	Q_OBJECT
public:
	MyMessage(QDialog* parent = 0);
	~MyMessage();
public slots:
	void onInfo();
	void onWarn();
	void onError();
private:
	Ui::MyMessage ui;
};

