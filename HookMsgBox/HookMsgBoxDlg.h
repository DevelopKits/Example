// HookMsgBoxDlg.h : ͷ�ļ�
//

#pragma once


// CHookMsgBoxDlg �Ի���
class CHookMsgBoxDlg : public CDialog
{
// ����
public:
	CHookMsgBoxDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_HOOKMSGBOX_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStartHook();
public:
	afx_msg void OnBnClickedBtnCallMsgBox();
	afx_msg void OnBnClickedBtnStopHook();
};
