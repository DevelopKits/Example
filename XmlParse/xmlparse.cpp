#include "xmlparse.h"

XmlParse::XmlParse(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_xmlParse = new XMLLocalConfig;
	int nret = m_xmlParse->LoadFile(XML_LOCALCONFIG);
	list<Login_Info_t> loginInfo;
	if (nret == XML_NO_EXIST)
	{
		Login_Info_t tmp;
		tmp.bRemPwd = false;
		tmp.nPort = 8000;
		tmp.strPassword = "123456";
		tmp.strUserName = "admin";
		tmp.strIP = "0.0.0.0";
		loginInfo.push_back(tmp);
		m_xmlParse->PacketXmlInside(loginInfo);
		m_xmlParse->SaveFile();
	}
	else
	{
		m_xmlParse->ParseXmlInside(loginInfo);
	}
}

XmlParse::~XmlParse()
{

}
