#include <QtWidgets/QApplication>
#include <QtCore/QDir>
#include "libdsl/DPrintLog.h"
#include "WorkThread.h"
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(a.applicationDirPath());
	dsl::DBaseLib::Init("log/MsgQue");
	int nLevel = DLOG_LEVEL_INFO;
	DLOG_SET_LEVEL(nLevel);
	DLOG_DEBUG("DEBUG");
	DLOG_INFO("INFO");
	DLOG_ERR("ERROR");
	WorkThread* pcWorkThread = new WorkThread;
	DLOG_INFO("pcWorkThread->start();");
	pcWorkThread->start();
	DLOG_INFO("pcWorkThread->stop();");
	pcWorkThread->stop();
	return a.exec();
}
