/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* IProfiles.h : ESB配置文件
* 作者    ：吕少卿
* 完成日期：2014年9月28日
*
* 当前版本：1.0
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
	// 子类重载该函数，在该函数中处理配置文件信息
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
