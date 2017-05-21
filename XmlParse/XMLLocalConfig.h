#ifndef _XMLLOCALCONDFIG_
#define _XMLLOCALCONDFIG_

#include <map>
#include <vector>
#include <list>
#include <string>
#include "tinyxml2.h"
using namespace std;

#define XML_LOCALCONFIG "SwartzConfigTool.xml"

//登陆信息节点
#define XML_TAG_LOGIN								"login"
#define XML_TAG_USERNAME							"userName"
#define XML_TAG_PASSWORD							"password"
#define XML_TAG_IP									"ip"
#define XML_TAG_PORT								"port"
#define XML_TAG_REMEMBERPWD							"rememberPwd"
#define XML_NO_EXIST   -1
#define XML_OK    0
using namespace tinyxml2;

typedef struct Login_Info_t
{
	std::string strUserName;
	std::string strPassword;
	std::string strIP;
	int			nPort;
	bool		bRemPwd;
	Login_Info_t()
	{
		strUserName = "";
		strPassword = "";
		strIP = "";
		nPort = 0;
		bRemPwd = false;
	}

	Login_Info_t& operator=(Login_Info_t& src)
	{
		strUserName = src.strUserName;
		strPassword = src.strPassword;
		strIP = src.strIP;
		nPort = src.nPort;
		bRemPwd = src.bRemPwd;
		return (*this);
	}
}Login_Info_t;

class XMLLocalConfig
{
public:
	XMLLocalConfig();
	~XMLLocalConfig();

public:
	int LoadFile( char *pFilename,bool bcreate = true);
	void ParseXmlInside(list<Login_Info_t>& loginInfo);
	void PacketXmlInside(list<Login_Info_t> loginInfo);
	void SaveFile();

public:
	char* m_xmlname;
	XMLDocument m_doc;

};

#endif // 
