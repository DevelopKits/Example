// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SWARTZCLIENT_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SWARTZCLIENT_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef SWARTZCLIENT_EXPORTS
#define SWARTZCLIENT_API __declspec(dllexport)
#else
#define SWARTZCLIENT_API __declspec(dllimport)
#endif

// �����Ǵ� SwartzClient.dll ������
class SWARTZCLIENT_API CSwartzClient {
public:
	CSwartzClient(void);
	// TODO:  �ڴ�������ķ�����
};

extern SWARTZCLIENT_API int nSwartzClient;

SWARTZCLIENT_API int fnSwartzClient(void);
