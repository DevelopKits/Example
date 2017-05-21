#include "XMLLocalConfig.h"


XMLLocalConfig::XMLLocalConfig()
{
	m_xmlname = NULL;
}


XMLLocalConfig::~XMLLocalConfig()
{
}

int XMLLocalConfig::LoadFile(char *pFilename, bool bcreate)
{
	XMLError ret = m_doc.LoadFile(pFilename);
	m_xmlname = pFilename;
	if (XML_SUCCESS != ret)
	{
		if (true == bcreate)
		{
			FILE* fp = NULL;
			fp = fopen(pFilename, "w+");//创建空xml文件
			fclose(fp);
		}
		return XML_NO_EXIST;
	}
	
	return XML_OK;
}

void XMLLocalConfig::SaveFile()
{
	m_doc.SaveFile(m_xmlname);
}

void XMLLocalConfig::ParseXmlInside(list<Login_Info_t>& loginInfo)
{

	XMLElement *rootEm = m_doc.RootElement();
	XMLElement *TmpEm  = rootEm->FirstChildElement("login");
	while (TmpEm)
	{
		Login_Info_t loginfo_s;
		XMLElement * childElem = NULL;
		childElem = TmpEm->FirstChildElement(XML_TAG_USERNAME);
		if (childElem != NULL)
			loginfo_s.strUserName = childElem->GetText();
		childElem = TmpEm->FirstChildElement(XML_TAG_PASSWORD);
		if (childElem != NULL)
			loginfo_s.strPassword = childElem->GetText();
		childElem = TmpEm->FirstChildElement(XML_TAG_IP);
		if (childElem != NULL)
			loginfo_s.strIP = childElem->GetText();
		childElem = TmpEm->FirstChildElement(XML_TAG_PORT);
		if (childElem != NULL)
			loginfo_s.nPort = atoi(childElem->GetText());
		childElem = TmpEm->FirstChildElement(XML_TAG_REMEMBERPWD);
		if (childElem != NULL)
			loginfo_s.bRemPwd = atoi(childElem->GetText()) ? true : false;
		loginInfo.push_back(loginfo_s);
		TmpEm = TmpEm->NextSiblingElement();
	}
	return ;
}

void XMLLocalConfig::PacketXmlInside(list<Login_Info_t> loginInfo)
{
	

	XMLDeclaration *pDecl = m_doc.NewDeclaration("xml=version=\"1.0\" encoding=\"UTF-8\"");
	m_doc.LinkEndChild(pDecl);
	XMLElement* userinfo = m_doc.NewElement("UserInfo");
	m_doc.InsertEndChild(userinfo);
	list<Login_Info_t>::iterator itor = loginInfo.begin();
	for (; itor != loginInfo.end();itor++)
	{
		XMLElement* loginEm = m_doc.NewElement(XML_TAG_LOGIN);
		userinfo->InsertEndChild(loginEm);

		XMLElement* UsrEm = m_doc.NewElement(XML_TAG_USERNAME);
		UsrEm->LinkEndChild(m_doc.NewText(itor->strUserName.c_str()));
		loginEm->InsertEndChild(UsrEm);

		XMLElement* pwdEm = m_doc.NewElement(XML_TAG_PASSWORD);
		pwdEm->LinkEndChild(m_doc.NewText(itor->strPassword.c_str()));
		loginEm->InsertEndChild(pwdEm);

		XMLElement* ipEm = m_doc.NewElement(XML_TAG_IP);
		ipEm->LinkEndChild(m_doc.NewText(itor->strIP.c_str()));
		loginEm->InsertEndChild(ipEm);

		XMLElement* portEm = m_doc.NewElement(XML_TAG_PORT);
		char szPort[10] = { 0 };
		itoa(itor->nPort, szPort, 10);
		portEm->LinkEndChild(m_doc.NewText(szPort));
		loginEm->InsertEndChild(portEm);

		XMLElement* rwdEm = m_doc.NewElement(XML_TAG_REMEMBERPWD);
		char szRwd[10] = { 0 };
		itoa((itor->bRemPwd ? 1 : 0), szRwd, 10);
		rwdEm->LinkEndChild(m_doc.NewText(szRwd));
		loginEm->InsertEndChild(rwdEm);
	}
	


}
