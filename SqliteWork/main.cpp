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

//��ȡ��ʽ��
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


//���ط����ļ�
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


//��ȡ�����ļ�
static void searchQmFile(const QString & strPath)
{
	QDir dir(strPath);
	if (!dir.exists())
	{
		return;
	}
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::DirsFirst);	// �ļ�������
	// ת����һ��List
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
		// ������ļ���
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

//�ж��Ƿ���IP��ַ
static bool IsIP(QString IP) {
	QRegExp RegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
	return RegExp.exactMatch(IP);
}

//Ŀǰֻ�ǲ��ԣ�demo�����ȥ�� �����������ݿ�
//һ��UserTable ѧϰ���������ݿ����Ͳ������
//�ڶ���PlansTable����������һ���������������ͣ�ͬʱ����һЩ��ѯ����
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QDir::setCurrent(a.applicationDirPath());

	QFont font("Arial", 9);
	a.setFont(font);

	pMyLogger = MyLogger::getInstance();
	QString strLanPath = QObject::tr("%1\\Language_zh").arg(QDir::currentPath());
	strLanPath = QDir::toNativeSeparators(strLanPath);
	//��ȡ�����ļ�
	searchQmFile(strLanPath);

	//����ǰĿ¼����Ϊ���������Ŀ¼
	QString strQssPath = QObject::tr("%1\\skin\\style.qss").arg(QDir::currentPath());
	strQssPath = QDir::toNativeSeparators(strQssPath);
	//����ȫ�ֵ���ʽ��
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
