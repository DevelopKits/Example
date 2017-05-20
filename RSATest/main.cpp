#include "rsatest.h"
#include <QtWidgets/QApplication>
#include <QtCore/QDir>
#include <QtCore/QTextCodec>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(a.applicationDirPath());
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	RSATest w;
	w.show();
	return a.exec();
}
