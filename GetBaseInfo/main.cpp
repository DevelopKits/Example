#include <QtWidgets/QApplication>
#include "GetBaseInfo.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	GetBaseInfo w;
	w.show();
	return a.exec();
}
