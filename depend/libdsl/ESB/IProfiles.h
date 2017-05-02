/* Copyright (c) 2013, �㽭�󻪼����ɷ����޹�˾, All rights reserved.
* 2013-10
*
* �汾    ��1.0
* IProfiles.h : �����ļ�����ģ��
* ����    ��������
* ������ڣ�2014��9��28��
*
* ��ǰ�汾��1.0
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
	// �������ظú������ڸú����д��������ļ���Ϣ
	virtual int OnRead() = 0;

	pugi::xml_document	m_xmlDoc;


private:
	friend class DMsgHandler;
	int ReadProfiles() { return OnRead(); }
};


/////////////////////////////////////////////////
END_NAMESPACE_DSL
