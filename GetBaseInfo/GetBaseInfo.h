#pragma once
#include <QDialog>
#include <Windows.h>
#include <atlstr.h>
#include <TlHelp32.h>
#include <NTSecAPI.h>
#include <QString>
#include <Psapi.h>
#include "ui_GetBaseInfo.h"


class GetBaseInfo :public QDialog
{
	Q_OBJECT
public:
	GetBaseInfo(QDialog* parent = 0);
	~GetBaseInfo();
public slots:
	void onStart();
	void onCancel();
private:
	Ui::GetBaseInfo ui;
};

