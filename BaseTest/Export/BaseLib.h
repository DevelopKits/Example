// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� EVENTHOOK_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// EVENTHOOK_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef BASELIB_EXPORTS
#define BASELIB_EXPORTS __declspec(dllexport)
#else
#define BASELIB_EXPORTS __declspec(dllimport)
#endif

class IBaseLib
{
public:
	virtual int Add(int a, int b)= 0;
	virtual int Sub(int a, int b)= 0;
	virtual void Release() = 0;
};

extern "C" BASELIB_EXPORTS IBaseLib* CreateBaseLib();
extern "C" BASELIB_EXPORTS void DestroyBaseLib(IBaseLib* pIBaseLib);