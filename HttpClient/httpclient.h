#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QtWidgets/QDialog>
#include "ui_httpclient.h"

class HttpClient : public QDialog
{
	Q_OBJECT

public:
	HttpClient(QWidget *parent = 0);
	~HttpClient();

	
private:
	Ui::HttpClientClass ui;
};

#endif // HTTPCLIENT_H
