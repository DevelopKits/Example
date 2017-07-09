#include "keymousehookapp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	KeyMouseHookApp w;
	w.show();
	return a.exec();
}
