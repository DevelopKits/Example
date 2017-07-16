#include "configtools.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ConfigTools w;
	w.show();

	return a.exec();
}
