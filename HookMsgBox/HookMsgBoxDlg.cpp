// HookMsgBoxDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HookMsgBox.h"
#include "HookMsgBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CHookMsgBoxDlg 对话框




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


// CHookMsgBoxDlg 消息处理程序

BOOL CHookMsgBoxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHookMsgBoxDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CHookMsgBoxDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//原函数类型定义
typedef int (WINAPI* MsgBoxW)(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType);
MsgBoxW OldMsgBoxW=NULL;//指向原函数的指针
FARPROC pfOldMsgBoxW;  //指向函数的远指针
BYTE OldCode[5]; //原系统API入口代码
BYTE NewCode[5]; //原系统API新的入口代码 (jmp xxxxxxxx)

HANDLE hProcess=NULL;//本程序进程句柄
HINSTANCE hInst=NULL;//API所在的dll文件句柄

void HookOn();
void HookOff();
int WINAPI MyMessageBoxW(HWND hWnd,LPCWSTR lpText,LPCWSTR lpCaption,UINT uType)
{
	TRACE(lpText);
	HookOff();//调用原函数之前，记得先恢复HOOK呀，不然是调用不到的
			  //如果不恢复HOOK，就调用原函数，会造成死循环
			  //毕竟调用的还是我们的函数，从而造成堆栈溢出，程序崩溃。

	int nRet=::MessageBoxW(hWnd,_T("哈哈，MessageBoxW被HOOK了"),lpCaption,uType);

	HookOn();//调用完原函数后，记得继续开启HOOK，不然下次会HOOK不到。 

	return nRet;
}



//开启钩子的函数
void HookOn() 
{ 
 ASSERT(hProcess!=NULL);

 DWORD dwTemp=0;
 DWORD dwOldProtect;
 
 //修改API函数入口前5个字节为jmp xxxxxx
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,PAGE_READWRITE,&dwOldProtect); 
 WriteProcessMemory(hProcess,pfOldMsgBoxW,NewCode,5,0);
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,dwOldProtect,&dwTemp);

}

//关闭钩子的函数
void HookOff()
{ 
 ASSERT(hProcess!=NULL);

 DWORD dwTemp=0;
 DWORD dwOldProtect;

 //恢复API函数入口前5个字节
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,PAGE_READWRITE,&dwOldProtect); 
 WriteProcessMemory(hProcess,pfOldMsgBoxW,OldCode,5,0); 
 VirtualProtectEx(hProcess,pfOldMsgBoxW,5,dwOldProtect,&dwTemp);  
}

//获取API函数入口前5个字节
//旧入口前5个字节保存在前面定义的字节数组BYTE OldCode[5]
//新入口前5个字节保存在前面定义的字节数组BYTE NewCode[5]
void GetApiEntrance()
{
 
  //获取原API入口地址
  HMODULE hmod=::LoadLibrary(_T("User32.dll"));
  OldMsgBoxW=(MsgBoxW)::GetProcAddress(hmod,"MessageBoxW");
  pfOldMsgBoxW=(FARPROC)OldMsgBoxW;
  
  if (pfOldMsgBoxW==NULL)
  {
	MessageBox(NULL,_T("获取原API入口地址出错"),_T("error!"),0);
	return;
  }

  // 将原API的入口前5个字节代码保存到OldCode[]
  _asm 
  { 
   lea edi,OldCode		//获取OldCode数组的地址,放到edi
   mov esi,pfOldMsgBoxW //获取原API入口地址，放到esi
   cld	  //方向标志位，为以下两条指令做准备
   movsd //复制原API入口前4个字节到OldCode数组
   movsb //复制原API入口第5个字节到OldCode数组
  }


  NewCode[0]=0xe9;//实际上0xe9就相当于jmp指令

  //获取MyMessageBoxW的相对地址,为Jmp做准备
  //int nAddr= UserFunAddr – SysFunAddr - （我们定制的这条指令的大小）;
  //Jmp nAddr;
  //（我们定制的这条指令的大小）, 这里是5，5个字节嘛
  _asm 
  { 
   lea eax,MyMessageBoxW //获取我们的MyMessageBoxW函数地址
   mov ebx,pfOldMsgBoxW  //原系统API函数地址
   sub eax,ebx			 //int nAddr= UserFunAddr – SysFunAddr
   sub eax,5			 //nAddr=nAddr-5
   mov dword ptr [NewCode+1],eax //将算出的地址nAddr保存到NewCode后面4个字节
								 //注：一个函数地址占4个字节
  } 
 

  //填充完毕，现在NewCode[]里的指令相当于Jmp MyMessageBoxW
  //既然已经获取到了Jmp MyMessageBoxW
  //现在该是将Jmp MyMessageBoxW写入原API入口前5个字节的时候了
  //知道为什么是5个字节吗？
  //Jmp指令相当于0xe9,占一个字节的内存空间
  //MyMessageBoxW是一个地址，其实是一个整数，占4个字节的内存空间
  //int n=0x123;   n占4个字节和MyMessageBoxW占4个字节是一样的
  //1+4=5，知道为什么是5个字节了吧
  HookOn(); 
}




void CHookMsgBoxDlg::OnBnClickedBtnStartHook()
{
	DWORD dwPid=::GetCurrentProcessId();
	hProcess=OpenProcess(PROCESS_ALL_ACCESS,0,dwPid); 

	GetApiEntrance();
	SetDlgItemText(IDC_STATIC_INFO,_T("Hook已启动"));
}


void CHookMsgBoxDlg::OnBnClickedBtnCallMsgBox()
{
	::MessageBoxW(m_hWnd,_T("这是正常的MessageBoxW"),_T("Hello"),0);
}

//停止HOOK
void CHookMsgBoxDlg::OnBnClickedBtnStopHook()
{
	HookOff();
	SetDlgItemText(IDC_STATIC_INFO,_T("Hook未启动"));
}
