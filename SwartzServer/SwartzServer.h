// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SWARTZSERVER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SWARTZSERVER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef SWARTZSERVER_EXPORTS
#define SWARTZSERVER_API __declspec(dllexport)
#else
#define SWARTZSERVER_API __declspec(dllimport)
#endif

// �����Ǵ� SwartzServer.dll ������
class SWARTZSERVER_API CSwartzServer {
public:
	CSwartzServer(void);
	// TODO:  �ڴ�������ķ�����
};

extern SWARTZSERVER_API int nSwartzServer;

SWARTZSERVER_API int fnSwartzServer(void);
