#if 0
#include "image.h"

//获取Y矩阵
long getMatY(long *iDstImgWidth, long *iDstImgHeight, POINT_INFO_S *pPointY, float *pDataY)
{
	//二维坐标下的点，类型为浮点
	CvPoint2D32f srcTri[4], dstTri[4];
	CvMat* warp_mat = cvCreateMat( 3, 3, CV_32FC1 );
	//float pData[9];

	//排序并获取外接矩左上、右下两个点
	//IMAGE_POINT rectPoint[2];
	sortPoint(pPointY);/*一定是顺时针*/

	//计算矩阵仿射变换
	dstTri[0].x = pPointY->imagePoints[0].x;
	dstTri[0].y = pPointY->imagePoints[0].y;
	dstTri[1].x = pPointY->imagePoints[1].x;
	dstTri[1].y = pPointY->imagePoints[1].y;
	dstTri[2].x = pPointY->imagePoints[2].x;
	dstTri[2].y = pPointY->imagePoints[2].y;
	dstTri[3].x = pPointY->imagePoints[3].x;
	dstTri[3].y = pPointY->imagePoints[3].y;

	///*GPU下*/
	////计算矩阵仿射变换

	////目标图像大小,顺时针1,2,3,4点
	//srcTri[0].x = 0;
	//srcTri[0].y = 0;
	//srcTri[1].x = *iDstImgWidth;
	//srcTri[1].y = 0;
	//srcTri[2].x = *iDstImgWidth;
	//srcTri[2].y = *iDstImgHeight;
	//srcTri[3].x = 0;
	//srcTri[3].y = *iDstImgHeight;

	/*cPU下*/
	//计算矩阵仿射变换

	//目标图像大小,顺时针1,2,3,4点
	srcTri[0].x = 0;
	srcTri[0].y = 0;
	srcTri[1].x = *iDstImgWidth;
	srcTri[1].y = 0;
	srcTri[2].x = *iDstImgWidth;
	srcTri[2].y = *iDstImgHeight;
	srcTri[3].x = 0;
	srcTri[3].y = *iDstImgHeight;

	//获取矩阵
	cvGetPerspectiveTransform( srcTri, dstTri, warp_mat );

	memcpy(pDataY,warp_mat->data.fl,POINT_NUM*sizeof(float));
	//pData = (float*)warp_mat->data.ptr;
	//for(int i=0;i<9;i++)
	//{
	//	pDataY[i]=(float)warp_mat->data.fl[i];
	//}
	cvReleaseMat(&warp_mat);

	return 0;
}

//获取UV矩阵
long getMatUV(long *iDstImgWidth, long *iDstImgHeight, POINT_INFO_S *pPointUV,float *pDataUV)
{
	//二维坐标下的点，类型为浮点
	CvPoint2D32f srcTri[4], dstTri[4];
	CvMat* warp_mat = cvCreateMat( 3, 3, CV_32FC1 );

	//排序并获取外接矩左上、右下两个点
	sortPoint(pPointUV);/*一定是顺时针*/
	//计算矩阵仿射变换
	dstTri[0].x = pPointUV->imagePoints[0].x;
	dstTri[0].y = pPointUV->imagePoints[0].y;
	dstTri[1].x = pPointUV->imagePoints[1].x;
	dstTri[1].y = pPointUV->imagePoints[1].y;
	dstTri[2].x = pPointUV->imagePoints[2].x;
	dstTri[2].y = pPointUV->imagePoints[2].y;
	dstTri[3].x = pPointUV->imagePoints[3].x;
	dstTri[3].y = pPointUV->imagePoints[3].y;

	//目标图像大小cpu下
	srcTri[0].x = 0;
	srcTri[0].y = 0;
	srcTri[1].x = *iDstImgWidth/2;
	srcTri[1].y = 0;
	srcTri[2].x = *iDstImgWidth/2;
	srcTri[2].y = *iDstImgHeight/2;
	srcTri[3].x = 0;
	srcTri[3].y = *iDstImgHeight/2;

	//目标图像大小GPU下
	//srcTri[0].x = 0;
	//srcTri[0].y = 0;
	//srcTri[1].x = *iDstImgWidth/2;
	//srcTri[1].y = 0;
	//srcTri[2].x = *iDstImgWidth/2;
	//srcTri[2].y = *iDstImgHeight/2;
	//srcTri[3].x = 0;
	//srcTri[3].y = *iDstImgHeight/2;

	//获取矩阵
	cvGetPerspectiveTransform( srcTri, dstTri, warp_mat );

	memcpy(pDataUV,warp_mat->data.fl,POINT_NUM*sizeof(float));
	//for(int i=0;i<9;i++)
	//{
	//	pDataUV[i]=(float)warp_mat->data.ptr[i];
	//}
	//pData = (float*)warp_mat->data.ptr;

	cvReleaseMat( &warp_mat );

	return 0;
}

//四个点按照顺时针排序并获取外接矩左上、右下两个点
void sortPoint(POINT_INFO_S *pPoint )
{
	IMAGE_POINT_S tmpPoint;

	//4个点按照x从小到大排序
	for (int i = 0; i < 3; i++)
	{
		for (int j = i; j < 4; j++)
		{
			if (pPoint->imagePoints[i].x > pPoint->imagePoints[j].x)
			{
				tmpPoint = pPoint->imagePoints[i];
				pPoint->imagePoints[i] =pPoint->imagePoints[j];
				pPoint->imagePoints[j] = tmpPoint;
			}
		}
	}
	//获取x最小最大值
	//rectPoint[0].x = pPoint[0].x;
	//rectPoint[1].x = pPoint[3].x;

	//4个点按照y从小到大排序
	for (int i = 0; i < 3; i++)
	{
		for (int j = i; j < 4; j++)
		{
			if (pPoint->imagePoints[i].y > pPoint->imagePoints[j].y)
			{
				tmpPoint = pPoint->imagePoints[i];
				pPoint->imagePoints[i] = pPoint->imagePoints[j];
				pPoint->imagePoints[j] = tmpPoint;
			}
		}
	}

	//获取y最小最大值
	//rectPoint[0].y = pPoint[0].y;
	//rectPoint[1].y = pPoint[3].y;

	//y最大的两个点比较x大小，小的为第4个点
	if (pPoint->imagePoints[2].x < pPoint->imagePoints[3].x)
	{
		tmpPoint = pPoint->imagePoints[3];
		pPoint->imagePoints[3] = pPoint->imagePoints[2];
		pPoint->imagePoints[2] = tmpPoint;
	}

	if (pPoint->imagePoints[1].x < pPoint->imagePoints[0].x)
	{
		tmpPoint = pPoint->imagePoints[0];
		pPoint->imagePoints[0] = pPoint->imagePoints[1];
		pPoint->imagePoints[1] = tmpPoint;
	}
	return;
}

#endif