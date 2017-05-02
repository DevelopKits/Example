/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* ESBParser.h : ESBЭ�����
* ����    ��������
* ������ڣ�2014��10��15��
*
* ��ǰ�汾��1.0
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

