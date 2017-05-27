// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 EVENTHOOK_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// EVENTHOOK_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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