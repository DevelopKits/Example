#include <QtWidgets/QApplication>
#include <QtCore/QDir>
#include "libdsl/DPrintLog.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QDir::setCurrent(app.applicationDirPath());
	dsl::DBaseLib::Init("log/BaseLog");
	int nLevel = DLOG_LEVEL_INFO;
	DLOG_SET_LEVEL(nLevel);
	DLOG_DEBUG("DEBUG");
	DLOG_INFO("INFO");
	DLOG_ERR("ERROR");
	return app.exec();
}
