#include <QtWidgets/QApplication>
#include <QDir>
#include <windows.h>
#include <QString>
#include <io.h>
#include "ExcelBase.h"

#define MAX_PATH          260
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QDir::setCurrent(a.applicationDirPath());
	
	char strExePathTmp[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, strExePathTmp, MAX_PATH);
	std::string strExePath(strExePathTmp);
	strExePath.resize(strExePath.find_last_of('\\'));
	SetCurrentDirectoryA(strExePath.c_str());
	ExcelBase* pcxls = new ExcelBase;
	strExePath += "\\Test.xls";
	if (0 == access(strExePath.c_str(), 0))
	{
		DeleteFileA(strExePath.c_str());
	}
	bool bret = pcxls->create(strExePath.c_str());
	QList< QList<QVariant> > m_datas;
	for (int i = 0; i < 10; ++i)
	{
		QList<QVariant> rows;
		for (int j = 0; j < 10; ++j)
		{
			rows.append(i*j);
		}
		m_datas.append(rows);
	}
	bret = pcxls->setCurrentSheet(1);
	pcxls->setSheetName("Swartz1");
	pcxls->writeCurrentSheet(m_datas);
	pcxls->addSheet("aaaaaaa");
	bret = pcxls->setCurrentSheet(2);
	m_datas.clear();
	for (int i = 5; i < 10; ++i)
	{
		QList<QVariant> rows;
		for (int j = 5; j < 10; ++j)
		{
			rows.append(i*j);
		}
		m_datas.append(rows);
	}
	pcxls->writeCurrentSheet(m_datas);
	pcxls->setSheetName("Swartz2");
	pcxls->save();
	pcxls->close();
	return a.exec();
}
