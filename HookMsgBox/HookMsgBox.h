// HookMsgBox.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CHookMsgBoxApp:
// �йش����ʵ�֣������ HookMsgBox.cpp
//

class CHookMsgBoxApp : public CWinApp
{
public:
	CHookMsgBoxApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CHookMsgBoxApp theApp;