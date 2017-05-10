#include <QtWidgets/QApplication>
#include "MyMessage.h"
#include <QDir>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(a.applicationDirPath());
	MyMessage w;
	w.show();
	return a.exec();
}
