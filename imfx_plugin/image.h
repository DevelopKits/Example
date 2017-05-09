#if 0

#include <highgui.h>
#include <cv.h>
//#include "cl.h"
#include <fstream>
#include <d3d9.h>

//#include <IMFX_plugin.h>

#define POINT_NUM 9

using namespace std;

/*初始化结构体的4个点*/
#if defined(WIN32) || defined(WIN64)
typedef struct tagImagePoint
{
	float x;
	float y;
}IMAGE_POINT_S;
/*初始化结构体的4个点,按顺时针方向给出4个点，不按顺时针方向也可以，内部已排序*//*每个通道的4个点不一定相同*/
typedef struct tagPointInfo
{
	IMAGE_POINT_S imagePoints[4];
}POINT_INFO_S;
#endif

/*放四个点的信息和窗口的宽高信息*/
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