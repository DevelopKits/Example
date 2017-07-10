// HookAllMsgBox.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "HookAllMsgBox.h"
#include <tchar.h>

HooKAllMsgBox g_HoolAllMsgBox;

void HookOn();
void HookOff();

//���������ڹ����,�������̹߳������±���;
#pragma data_seg(".SHARED")
HINSTANCE g_hInstDll = NULL; //��dll��ʵ�����;
HHOOK g_hMouse = NULL; //��깳�Ӿ��;
#pragma data_seg()
#pragma comment(linker, "/section:.SHARED,rws")

HWND g_hWnd = NULL;
HANDLE hProcess = NULL;
BOOL bIsInjected = FALSE;

//ԭ��������
typedef int (WINAPI *MsgBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
typedef int (WINAPI *MsgBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
MsgBoxA oldMsgBoxA = NULL; //���ڱ���ԭ������ַ
MsgBoxW oldMsgBoxW = NULL; //���ڱ���ԭ������ַ
FARPROC pfMsgBoxA = NULL;//ָ��ԭ������ַ��Զָ��
FARPROC pfMsgBoxW = NULL;//ָ��ԭ������ַ��Զָ��
BYTE OldCodeA[5]; //�ϵ�ϵͳAPI��ڴ���
BYTE NewCodeA[5]; //Ҫ��ת��API���� (jmp xxxx)
BYTE OldCodeW[5]; //�ϵ�ϵͳAPI��ڴ���
BYTE NewCodeW[5]; //Ҫ��ת��API���� (jmp xxxx)

int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
int WINAPI MyMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstDll = hModule;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//��깳�ӹ��̣�Ŀ���Ǽ��ر�dll��ʹ�����ĳ���
//��깳�ӵ����ã��������ĳ���򴰿���ʱ����ͻ�����������dll
LRESULT CALLBACK HookMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//�������Ϣʱ�����䷢��������
	if (nCode == HC_ACTION)
	{
		::PostMessage(g_hWnd, WM_MYMOUSE, wParam, (LPARAM)(((PMOUSEHOOKSTRUCT)lParam)->hwnd));
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
int WINAPI UninitHook()
{
	HookOff();//�ǵûָ�ԭAPI��ڹ�
	//��������øú���ʱ���ָ���ֻ��������ԭAPI����ڣ�
	//���������API��ڻ�û�б��ָ����������Ǳ��봦��
	//dll�˳����̣����ں���ExitInstance()�У����ûָ�
	//API��ڵĺ���HookOff(),ֻ�����������������ٴε���
	//ԭAPIʱ���Ų��ᷢ������ม�
	//������HOOK���г����ĳ��ϵͳAPIʱ��ǧ��Ҫע����
	//ExitInstance()�е���HookOff(),Ѫ�Ľ�ѵ����
	if (g_hMouse)
	{
		UnhookWindowsHookEx(g_hMouse);
		FreeLibrary(g_hInstDll);
		g_hMouse = NULL;
	}
	return 1;
}


 int WINAPI InitHook(HWND hWnd)
{
	g_hWnd = hWnd;
	g_hMouse = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)HookMouseProc, g_hInstDll, NULL);
	if (!g_hMouse)
	{
		UninitHook();
		return -1;
	}
	return 1;
}

// �������ӵĺ���
void HookOn()
{
	DWORD dwTemp = 0, dwOldProtect, dwRet = 0, dwWrite;

	//�޸�ԭAPI���ǰ����ֽ�Ϊjmp xxxxxxxx
	//Debug�汾���������޸�ʧ�ܣ�����Release�汾�ɹ�
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, PAGE_READWRITE, &dwOldProtect);
	dwRet = WriteProcessMemory(hProcess, pfMsgBoxA, NewCodeA, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		
	}
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, dwOldProtect, &dwTemp);

	//�޸�ԭAPI���ǰ����ֽ�Ϊjmp xxxxxxxx
	//Debug�汾���������޸�ʧ�ܣ�����Release�汾�ɹ�
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, PAGE_READWRITE, &dwOldProtect);
	dwRet = WriteProcessMemory(hProcess, pfMsgBoxW, NewCodeW, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		
	}
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, dwOldProtect, &dwTemp);
}

//�رչ��ӵĺ���
void HookOff()//������������add()����ڴ���ָ�
{

	DWORD dwTemp = 0, dwOldProtect = 0, dwRet = 0, dwWrite = 0;

	//�ָ�ԭAPI���
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, PAGE_READWRITE, &dwOldProtect);
	dwRet = WriteProcessMemory(hProcess, pfMsgBoxA, OldCodeA, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		//TRACE("����д��ʧ��");
	}
	VirtualProtectEx(hProcess, pfMsgBoxA, 5, dwOldProtect, &dwTemp);

	//�ָ�ԭAPI���
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, PAGE_READWRITE, &dwOldProtect);
	WriteProcessMemory(hProcess, pfMsgBoxW, OldCodeW, 5, &dwWrite);
	if (0 == dwRet || 0 == dwWrite)
	{
		//TRACE("����д��ʧ��");
	}
	VirtualProtectEx(hProcess, pfMsgBoxW, 5, dwOldProtect, &dwTemp);
}

HooKAllMsgBox::HooKAllMsgBox()
{
	DWORD dwPid = ::GetCurrentProcessId();
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);

	if (!bIsInjected)
	{
		bIsInjected = TRUE;//��ֻ֤����1��
		//��ȡ����
		HMODULE hmod = ::LoadLibraryA(("User32.dll"));
		oldMsgBoxA = (MsgBoxA)::GetProcAddress(hmod, "MessageBoxA");
		pfMsgBoxA = (FARPROC)oldMsgBoxA;
		oldMsgBoxW = (MsgBoxW)::GetProcAddress(hmod, "MessageBoxW");
		pfMsgBoxW = (FARPROC)oldMsgBoxW;

		if (pfMsgBoxA == NULL)
		{
			MessageBoxA(NULL, ("cannot get MessageBoxA()"), ("error"), 0);
			return;
		}
		if (pfMsgBoxW == NULL)
		{
			MessageBoxA(NULL, ("cannot get MessageBoxW()"), ("error"), 0);
			return;
		}

		// ��ԭAPI�е���ڴ��뱣����OldCodeA[]��OldCodeW[]
		_asm
		{
			lea edi, OldCodeA
				mov esi, pfMsgBoxA
				cld
				movsd
				movsb
		}
		_asm
		{
			lea edi, OldCodeW
				mov esi, pfMsgBoxW
				cld
				movsd
				movsb
		}

		NewCodeA[0] = 0xe9;//ʵ����0xe9���൱��jmpָ��
		NewCodeW[0] = 0xe9;//ʵ����0xe9���൱��jmpָ��

		//��ȡ���ǵ�API�ĵ�ַ
		_asm
		{
			lea eax, MyMessageBoxA
				mov ebx, pfMsgBoxA
				sub eax, ebx
				sub eax, 5
				mov dword ptr[NewCodeA + 1], eax
		}
		_asm
		{
			lea eax, MyMessageBoxW
				mov ebx, pfMsgBoxW
				sub eax, ebx
				sub eax, 5
				mov dword ptr[NewCodeW + 1], eax
		}

		//�����ϣ�����NewCode[]���ָ���൱��Jmp Myadd
		HookOn(); //���Կ���������
	}
}

HooKAllMsgBox::~HooKAllMsgBox()
{
	//dll�˳�ʱ���ǵûָ�ԭAPI��ڹ���Ѫ�Ľ�ѵѽ��
	//�����ǹ����г����API����dll�˳�û�лָ�ԭAPI���ʱ��
	//��ô�����������ٴε��ø�APIʱ���ᷢ��������Ϊ���ǵ�
	//dll�����Ѿ��˳��ˣ�ԭ��API��ڱ��޸ĵ�ǰ����ֽڻ���
	//ָ������dll�е��Լ�����ĺ�����ַ������dll�˳����õ�ַ
	//��ȻҲ�Ͳ������ˣ�������ø��Լ�ʱ����Ȼ�ᷢ�������ˡ�
	HookOff();
}

// ���ǵļ�API����MyMessageBoxA
int WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	int nRet = 0;

	HookOff();//����ԭ����֮ǰ���ǵ��Ȼָ�HOOKѽ����Ȼ�ǵ��ò�����
	//������ָ�HOOK���͵���ԭ�������������ѭ��
	//�Ͼ����õĻ������ǵĺ������Ӷ���ɶ�ջ��������������

	nRet = ::MessageBoxA(hWnd, "������MessageBoxA��HOOK�˰�", lpCaption, uType);
	nRet = ::MessageBoxA(hWnd, lpText, lpCaption, uType);
	HookOn();//������ԭ�����󣬼ǵü�������HOOK����Ȼ�´λ�HOOK����Ŷ�� 

	return nRet;
}

//���ǵļ�API����MyMessageBoxW
int WINAPI MyMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	int nRet = 0;

	HookOff();//����ԭ����֮ǰ���ǵ��Ȼָ�HOOKѽ����Ȼ�ǵ��ò�����
	//������ָ�HOOK���͵���ԭ�������������ѭ��
	//�Ͼ����õĻ������ǵĺ������Ӷ���ɶ�ջ��������������

	nRet = ::MessageBoxW(hWnd, _T("����,MessageBoxW��HOOK�˰�"), lpCaption, uType);
	nRet = ::MessageBoxW(hWnd, lpText, lpCaption, uType);
	HookOn();//������ԭ�����󣬼ǵü�������HOOK����Ȼ�´λ�HOOK����Ŷ�� 

	return nRet;
}
