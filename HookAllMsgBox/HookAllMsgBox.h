// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� HOOKALLMSGBOX_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// HOOKALLMSGBOX_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef HOOKALLMSGBOX_EXPORTS
#define HOOKALLMSGBOX_API __declspec(dllexport)
#else
#define HOOKALLMSGBOX_API __declspec(dllimport)
#endif

#define WM_MYMOUSE WM_USER + 306 //�Զ�����Ϣ�����ں�������ͨ��

class HooKAllMsgBox
{
	public:
		HooKAllMsgBox();
		~HooKAllMsgBox();

};

//extern "C" HOOKALLMSGBOX_API int __stdcall InitHook(HWND hWnd);
//extern "C" HOOKALLMSGBOX_API int __stdcall UninitHook();