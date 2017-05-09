#if 0
#include "image.h"

//��ȡY����
long getMatY(long *iDstImgWidth, long *iDstImgHeight, POINT_INFO_S *pPointY, float *pDataY)
{
	//��ά�����µĵ㣬����Ϊ����
	CvPoint2D32f srcTri[4], dstTri[4];
	CvMat* warp_mat = cvCreateMat( 3, 3, CV_32FC1 );
	//float pData[9];

	//���򲢻�ȡ��Ӿ����ϡ�����������
	//IMAGE_POINT rectPoint[2];
	sortPoint(pPointY);/*һ����˳ʱ��*/

	//����������任
	dstTri[0].x = pPointY->imagePoints[0].x;
	dstTri[0].y = pPointY->imagePoints[0].y;
	dstTri[1].x = pPointY->imagePoints[1].x;
	dstTri[1].y = pPointY->imagePoints[1].y;
	dstTri[2].x = pPointY->imagePoints[2].x;
	dstTri[2].y = pPointY->imagePoints[2].y;
	dstTri[3].x = pPointY->imagePoints[3].x;
	dstTri[3].y = pPointY->imagePoints[3].y;

	///*GPU��*/
	////����������任

	////Ŀ��ͼ���С,˳ʱ��1,2,3,4��
	//srcTri[0].x = 0;
	//srcTri[0].y = 0;
	//srcTri[1].x = *iDstImgWidth;
	//srcTri[1].y = 0;
	//srcTri[2].x = *iDstImgWidth;
	//srcTri[2].y = *iDstImgHeight;
	//srcTri[3].x = 0;
	//srcTri[3].y = *iDstImgHeight;

	/*cPU��*/
	//����������任

	//Ŀ��ͼ���С,˳ʱ��1,2,3,4��
	srcTri[0].x = 0;
	srcTri[0].y = 0;
	srcTri[1].x = *iDstImgWidth;
	srcTri[1].y = 0;
	srcTri[2].x = *iDstImgWidth;
	srcTri[2].y = *iDstImgHeight;
	srcTri[3].x = 0;
	srcTri[3].y = *iDstImgHeight;

	//��ȡ����
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

//��ȡUV����
long getMatUV(long *iDstImgWidth, long *iDstImgHeight, POINT_INFO_S *pPointUV,float *pDataUV)
{
	//��ά�����µĵ㣬����Ϊ����
	CvPoint2D32f srcTri[4], dstTri[4];
	CvMat* warp_mat = cvCreateMat( 3, 3, CV_32FC1 );

	//���򲢻�ȡ��Ӿ����ϡ�����������
	sortPoint(pPointUV);/*һ����˳ʱ��*/
	//����������任
	dstTri[0].x = pPointUV->imagePoints[0].x;
	dstTri[0].y = pPointUV->imagePoints[0].y;
	dstTri[1].x = pPointUV->imagePoints[1].x;
	dstTri[1].y = pPointUV->imagePoints[1].y;
	dstTri[2].x = pPointUV->imagePoints[2].x;
	dstTri[2].y = pPointUV->imagePoints[2].y;
	dstTri[3].x = pPointUV->imagePoints[3].x;
	dstTri[3].y = pPointUV->imagePoints[3].y;

	//Ŀ��ͼ���Сcpu��
	srcTri[0].x = 0;
	srcTri[0].y = 0;
	srcTri[1].x = *iDstImgWidth/2;
	srcTri[1].y = 0;
	srcTri[2].x = *iDstImgWidth/2;
	srcTri[2].y = *iDstImgHeight/2;
	srcTri[3].x = 0;
	srcTri[3].y = *iDstImgHeight/2;

	//Ŀ��ͼ���СGPU��
	//srcTri[0].x = 0;
	//srcTri[0].y = 0;
	//srcTri[1].x = *iDstImgWidth/2;
	//srcTri[1].y = 0;
	//srcTri[2].x = *iDstImgWidth/2;
	//srcTri[2].y = *iDstImgHeight/2;
	//srcTri[3].x = 0;
	//srcTri[3].y = *iDstImgHeight/2;

	//��ȡ����
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

//�ĸ��㰴��˳ʱ�����򲢻�ȡ��Ӿ����ϡ�����������
void sortPoint(POINT_INFO_S *pPoint )
{
	IMAGE_POINT_S tmpPoint;

	//4���㰴��x��С��������
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
	//��ȡx��С���ֵ
	//rectPoint[0].x = pPoint[0].x;
	//rectPoint[1].x = pPoint[3].x;

	//4���㰴��y��С��������
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

	//��ȡy��С���ֵ
	//rectPoint[0].y = pPoint[0].y;
	//rectPoint[1].y = pPoint[3].y;

	//y����������Ƚ�x��С��С��Ϊ��4����
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