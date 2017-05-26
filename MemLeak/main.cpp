#include <QtWidgets/QApplication>
#include "MemLeak.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MemLeak memleak;
	memleak.start();
	return a.exec();
}
