/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* IProfiles.h : ESB�����ļ�
* ����    ��������
* ������ڣ�2014��9��28��
*
* ��ǰ�汾��1.0
*/

#pragma once

#include <vector>
#include <libdsl/ESB/IProfiles.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API ESBProfiles : public IProfiles
{
public:
	virtual ~ESBProfiles(void) {}

protected:
	// �������ظú������ڸú����д��������ļ���Ϣ
	virtual int OnRead();

private:
	int GeneralConfig(pugi::xml_node &nodeESB);
	int Interconnection(pugi::xml_node &nodeESB);

public:
	std::string m_sLocalPort;
	std::string m_sLogLevel;

	class InterConnItem
	{
	public:
		InterConnItem() {}
 		InterConnItem(const InterConnItem& icItem)
 		{
 			m_sIP = icItem.m_sIP;
 			m_sPort = icItem.m_sPort;
 		}
		std::string m_sIP;
		std::string m_sPort;
	};
	std::vector<ESBProfiles::InterConnItem> m_vctInterConns;
};

/////////////////////////////////////////////////
END_NAMESPACE_DSL
