// HookMsgBoxDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HookMsgBox.h"
#include "HookMsgBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CHookMsgBoxDlg �Ի���




CHookMsgBoxDlg::CHookMsgBoxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHookMsgBoxDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHookMsgBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHookMsgBoxDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_START_HOOK, &CHookMsgBoxDlg::OnBnClickedBtnStartHook)
	ON_BN_CLICKED(IDC_BTN_CALL_MSG_BOX, &CHookMsgBoxDlg::OnBnClickedBtnCallMsgBox)
	ON_BN_CLICKED(IDC_BTN_STOP_HOOK, &CHookMsgBoxDlg::OnBnClickedBtnStopHook)
END_MESSAGE_MAP()


// CHookMsgBoxDlg ��Ϣ�������

BOOL CHookMsgBoxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CHookMsgBoxDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CHookMsgBoxDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CHookMsgBoxDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//ԭ�������Ͷ���
typedef int (WINAPI* MsgBoxW)(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType);
MsgBoxW OldMsgBoxW=NULL;//ָ��ԭ������ָ��
FARPROC pfOldMsgBoxW;  //ָ������Զָ��
BYTE OldCode[5]; //ԭϵͳAPI��ڴ���
BYTE NewCode[5]; //ԭϵͳAPI�µ���ڴ��� (jmp xxxxxxxx)

HANDLE hProcess=NULL;//��������̾��
HINSTANCE hInst=NULL;//API���ڵ�dll�ļ����

void HookOn();
void HookOff();
int WINAPI MyMessageBoxW(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType)
{
	TRACE(lpText);
	HookOff();//����ԭ����֮ǰ���ǵ��Ȼָ�HOOKѽ����Ȼ�ǵ��ò�����
			  //������ָ�HOOK���͵���ԭ�������������ѭ��
			  //�Ͼ����õĻ������ǵĺ������Ӷ���ɶ�ջ��������������

	int nRet=::MessageBoxW(hWnd,_T("������MessageBoxW��HOOK��"),lpCaption,uType);

	HookOn();//������ԭ�����󣬼ǵü�������HOOK����Ȼ�´λ�HOOK������ 

	return nRet;
}



//�������ӵĺ���
void HookOn() 
{ 
 ASSERT(hProcess!=NULL);

 DWORD dwTemp=0;
 DWORD dwOldProtect;
 
 //�޸�API�������ǰ5���ֽ�Ϊjmp xxxxxx
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,PAGE_READWRITE,&dwOldProtect); 
 WriteProcessMemory(hProcess,pfOldMsgBoxW,NewCode,5,0);
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,dwOldProtect,&dwTemp);

}

//�رչ��ӵĺ���
void HookOff()
{ 
 ASSERT(hProcess!=NULL);

 DWORD dwTemp=0;
 DWORD dwOldProtect;

 //�ָ�API�������ǰ5���ֽ�
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,PAGE_READWRITE,&dwOldProtect); 
 WriteProcessMemory(hProcess,pfOldMsgBoxW,OldCode,5,0); 
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,dwOldProtect,&dwTemp);  
}

//��ȡAPI�������ǰ5���ֽ�
//�����ǰ5���ֽڱ�����ǰ�涨����ֽ�����BYTE OldCode[5]
//�����ǰ5���ֽڱ�����ǰ�涨����ֽ�����BYTE NewCode[5]
void GetApiEntrance()
{
 
  //��ȡԭAPI��ڵ�ַ
  HMODULE hmod=::LoadLibrary(_T("User32.dll"));
  OldMsgBoxW=(MsgBoxW)::GetProcAddress(hmod,"MessageBoxW");
  pfOldMsgBoxW=(FARPROC)OldMsgBoxW;
  
  if (pfOldMsgBoxW==NULL)
  {
	MessageBox(NULL,_T("��ȡԭAPI��ڵ�ַ����"),_T("error!"),0);
	return;
  }

  // ��ԭAPI�����ǰ5���ֽڴ��뱣�浽OldCode[]
  _asm 
  { 
   lea edi,OldCode		//��ȡOldCode����ĵ�ַ,�ŵ�edi
   mov esi,pfOldMsgBoxW //��ȡԭAPI��ڵ�ַ���ŵ�esi
   cld	  //�����־λ��Ϊ��������ָ����׼��
   movsd //����ԭAPI���ǰ4���ֽڵ�OldCode����
   movsb //����ԭAPI��ڵ�5���ֽڵ�OldCode����
  }


  NewCode[0]=0xe9;//ʵ����0xe9���൱��jmpָ��

  //��ȡMyMessageBoxW����Ե�ַ,ΪJmp��׼��
  //int nAddr= UserFunAddr �C SysFunAddr - �����Ƕ��Ƶ�����ָ��Ĵ�С��;
  //Jmp nAddr;
  //�����Ƕ��Ƶ�����ָ��Ĵ�С��, ������5��5���ֽ���
  _asm 
  { 
   lea eax,MyMessageBoxW //��ȡ���ǵ�MyMessageBoxW������ַ
   mov ebx,pfOldMsgBoxW  //ԭϵͳAPI������ַ
   sub eax,ebx			 //int nAddr= UserFunAddr �C SysFunAddr
   sub eax,5			 //nAddr=nAddr-5
   mov dword ptr [NewCode+1],eax //������ĵ�ַnAddr���浽NewCode����4���ֽ�
								 //ע��һ��������ַռ4���ֽ�
  } 
 

  //�����ϣ�����NewCode[]���ָ���൱��Jmp MyMessageBoxW
  //��Ȼ�Ѿ���ȡ����Jmp MyMessageBoxW
  //���ڸ��ǽ�Jmp MyMessageBoxWд��ԭAPI���ǰ5���ֽڵ�ʱ����
  //֪��Ϊʲô��5���ֽ���
  //Jmpָ���൱��0xe9,ռһ���ֽڵ��ڴ�ռ�
  //MyMessageBoxW��һ����ַ����ʵ��һ��������ռ4���ֽڵ��ڴ�ռ�
  //int n=0x123;   nռ4���ֽں�MyMessageBoxWռ4���ֽ���һ����
  //1+4=5��֪��Ϊʲô��5���ֽ��˰�
  HookOn(); 
}




void CHookMsgBoxDlg::OnBnClickedBtnStartHook()
{
	DWORD dwPid=::GetCurrentProcessId();
	hProcess=OpenProcess(PROCESS_ALL_ACCESS,0,dwPid); 

	GetApiEntrance();
	SetDlgItemText(IDC_STATIC_INFO,_T("Hook������"));
}


void CHookMsgBoxDlg::OnBnClickedBtnCallMsgBox()
{
	::MessageBoxW(m_hWnd,_T("����������MessageBoxW"),_T("Hello"),0);
}

//ֹͣHOOK
void CHookMsgBoxDlg::OnBnClickedBtnStopHook()
{
	HookOff();
	SetDlgItemText(IDC_STATIC_INFO,_T("Hookδ����"));
}
