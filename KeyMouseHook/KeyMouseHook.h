// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� KEYMOUSEHOOK_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// KEYMOUSEHOOK_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef KEYMOUSEHOOK_EXPORTS
#define KEYMOUSEHOOK_API __declspec(dllexport)
#else
#define KEYMOUSEHOOK_API __declspec(dllimport)
#endif

#define WM_MYKEY WM_USER + 305 //�Զ�����Ϣ�����ں�������ͨ��
#define WM_MYMOUSE WM_USER + 306 //�Զ�����Ϣ�����ں�������ͨ��

KEYMOUSEHOOK_API int InitHook(HWND hWnd);
KEYMOUSEHOOK_API int UninitHook();
