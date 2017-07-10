#include "hookallmsgboxapp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HooKAllMsgBoxApp w;
	w.show();
	return a.exec();
}
