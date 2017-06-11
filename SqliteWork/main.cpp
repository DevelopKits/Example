#include <QtWidgets/QApplication>
#include <iostream>
#include <QString>
#include <QtCore/QDir>
#include <QTextCodec>
#include <QFont>
#include <QFile>
#include <QTranslator>
#include "CppSQLite3DB.h"
#include "MyLogger.h"

using namespace std;
MyLogger * pMyLogger = NULL;

//读取样式表
static QString readQssFromFile(QString strFilePath)
{
	QString strData;
	if (!QFile::exists(strFilePath))
		return strData;

	QFile file(strFilePath);
	if (file.open(QIODevice::ReadOnly))
	{
		QByteArray byte = file.readAll();
		strData = QString::fromLocal8Bit(byte.data());
		file.close();
		return strData;
	}
	return strData;
}


//加载翻译文件
static bool setTranslator(const QString strPath)
{
	bool bRet = false;
	if (strPath.isEmpty() || !QFile::exists(strPath))
	{
		return bRet;
	}
	QTranslator * pTrans = new QTranslator();
	if (pTrans->load(strPath))
	{
		QApplication::installTranslator(pTrans);
		bRet = true;
	}
	else
	{
		delete pTrans;
		pTrans = NULL;
	}
	return bRet;
}


//读取翻译文件
static void searchQmFile(const QString & strPath)
{
	QDir dir(strPath);
	if (!dir.exists())
	{
		return;
	}
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::DirsFirst);	// 文件夹优先
	// 转换成一个List
	QFileInfoList list = dir.entryInfoList();
	if (list.size() < 1)
	{
		return;
	}
	int i = 0;
	do
	{
		QFileInfo fileInfo = list.at(i);
		QString tt = fileInfo.fileName();
		// 如果是文件夹
		bool bisDir = fileInfo.isDir();
		if (bisDir)
		{
			searchQmFile(fileInfo.filePath());
		}
		else
		{
			bool bQm = fileInfo.fileName().endsWith(".qm");
			setTranslator(fileInfo.filePath());
		}
		i++;
	} while (i < list.size());
}

//判断是否是IP地址
static bool IsIP(QString IP) {
	QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
	return RegExp.exactMatch(IP);
}

//目前只是测试，demo里面会去打开 创建两个数据库
//一个UserTable 学习基本的数据库插件和插入操作
//第二个PlansTable，里面会插入一个基本的数据类型，同时进行一些查询操作
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(a.applicationDirPath());

	QFont font("Arial", 9);
	a.setFont(font);

	pMyLogger = MyLogger::getInstance();
	QString strLanPath = QObject::tr("%1\\Language_zh").arg(QDir::currentPath());
	strLanPath = QDir::toNativeSeparators(strLanPath);
	//读取翻译文件
	searchQmFile(strLanPath);

	//将当前目录设置为程序的所在目录
	QString strQssPath = QObject::tr("%1\\skin\\style.qss").arg(QDir::currentPath());
	strQssPath = QDir::toNativeSeparators(strQssPath);
	//设置全局的样式表
	QString strData = readQssFromFile(strQssPath);
	a.setStyleSheet(strData);

	ERROR_LOG("the application started");

	IDataBase* m_pcDataBase = new(std::nothrow) CppSQLite3DB;
	m_pcDataBase->OpenDataBase("Swartz.db");
	PLANS_INFO_S* pPlansInfo = new PLANS_INFO_S;
	memset(pPlansInfo->sPlanID, 0x0, ID_LEN);
	pPlansInfo->bCheckFlag = 1;
	memcpy(pPlansInfo->sPlanID, "Swartz1", strlen("Swartz1"));
	m_pcDataBase->InsertToPlansTable(pPlansInfo);
	memset(pPlansInfo->sPlanID, 0x0, ID_LEN);
	pPlansInfo->bCheckFlag = 2;
	memcpy(pPlansInfo->sPlanID, "Swartz2", strlen("Swartz2"));
	m_pcDataBase->InsertToPlansTable(pPlansInfo);
	PlanInfoList st_planinfo;
	int nret =m_pcDataBase->GetPlansTable(st_planinfo);
	PlanInfoList::iterator itor = st_planinfo.begin();
	for (itor; itor != st_planinfo.end();itor++)
	{
		DEBUG_LOG("sPlanID = "<<itor->sPlanID<<" ,bCheckFlag =  "<<itor->bCheckFlag<<"!");
	}
	return 0;
}
