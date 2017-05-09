#if 0

#include <highgui.h>
#include <cv.h>
//#include "cl.h"
#include <fstream>
#include <d3d9.h>

//#include <IMFX_plugin.h>

#define POINT_NUM 9

using namespace std;

/*��ʼ���ṹ���4����*/
#if defined(WIN32) || defined(WIN64)
typedef struct tagImagePoint
{
	float x;
	float y;
}IMAGE_POINT_S;
/*��ʼ���ṹ���4����,��˳ʱ�뷽�����4���㣬����˳ʱ�뷽��Ҳ���ԣ��ڲ�������*//*ÿ��ͨ����4���㲻һ����ͬ*/
typedef struct tagPointInfo
{
	IMAGE_POINT_S imagePoints[4];
}POINT_INFO_S;
#endif

/*���ĸ������Ϣ�ʹ��ڵĿ����Ϣ*/
struct RotateParam
{
	long Angle;
	long wWith;
	long wHeight;
	POINT_INFO_S sPointY_INFOR;
	POINT_INFO_S sPointUV_INFOR;
	//IDirect3DDevice9Ex* pD3DDevice;
};
long getMatY(long *iDstImgWidth, long *iDstImgHeight, POINT_INFO_S *pPointY,float *pDataY);

long getMatUV(long *iDstImgWidth, long *iDstImgHeight, POINT_INFO_S *pPointUV,float *pDataUV);

void sortPoint(POINT_INFO_S *pPoint);

#endif