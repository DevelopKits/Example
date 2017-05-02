/* Copyright (c) 2013, 浙江大华技术股份有限公司, All rights reserved.
* 2013-10
*
* 版本    ：1.0
* ESBParser.h : ESB协议解析
* 作者    ：吕少卿
* 完成日期：2014年10月15日
*
* 当前版本：1.0
*/

#pragma once

#include <libdsl/ESB/DMsgHandler.h>
#include <libdsl/ESB/ESBProtocal.h>

BEGIN_NAMESPACE_DSL
/////////////////////////////////////////////////

class LIBDSL_API ESBParser : public DMsgHandler 
{
public:
	virtual ~ESBParser() {}

	void OnRequestParser(DMsg* pMsg);

	DECLARE_FUNC_MAP(ESBParser)
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL

