#include "xmlparse.h"
#include <QtWidgets/QApplication>
#include <QtCore/QDir>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(a.applicationDirPath());
	XmlParse w;
	w.show();
	return a.exec();
}
