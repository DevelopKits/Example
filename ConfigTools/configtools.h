#ifndef CONFIGTOOLS_H
#define CONFIGTOOLS_H

#include <QtWidgets/QWidget>
#include "ui_configtools.h"
#include <winsock.h>/
#include <QtNetwork>
#include <QList>

#pragma comment(lib, "Wsock32.lib")
class ConfigTools : public QWidget
{
	Q_OBJECT

public:
	ConfigTools(QWidget *parent = 0);
	~ConfigTools();

public slots:
	void onSearch();
private:
	Ui::ConfigToolsClass ui;

private:
	SOCKET m_socket;       //socket
	struct sockaddr_in m_sockDesAddress;   //Ä¿µÄsockµØÖ·
	struct in_addr localInterface;
};

#endif // CONFIGTOOLS_H
