#include "httpclient.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HttpClient w;
	w.show();
	return a.exec();
}
