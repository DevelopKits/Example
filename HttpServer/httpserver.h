#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtWidgets/QDialog>
#include "ui_httpserver.h"

class HttpServer : public QDialog
{
	Q_OBJECT

public:
	HttpServer(QWidget *parent = 0);
	~HttpServer();

private:
	Ui::HttpServerClass ui;
};

#endif // HTTPSERVER_H
