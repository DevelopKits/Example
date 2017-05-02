/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* IProfiles.h : 配置文件处理模块
* 作者    ：吕少卿
* 完成日期：2014年9月28日
*
* 当前版本：1.0
*/

#pragma once

#include <libdsl/DRefObj.h>
#include <pugixmldsl/pugixml.hpp>
#include <pugixmldsl/pugiassist.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API IProfiles : virtual public DRefObj
{
public:
	IProfiles(void) {}
	virtual ~IProfiles(void) {}

protected:
	// 子类重载该函数，在该函数中处理配置文件信息
	virtual int OnRead() = 0;

	pugi::xml_document	m_xmlDoc;


private:
	friend class DMsgHandler;
	int ReadProfiles() { return OnRead(); }
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL
