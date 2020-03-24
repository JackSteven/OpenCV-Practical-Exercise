#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;


int main1()
{
	String img_path = "circle.png";
	Mat src, gray, thr;

	src = imread(img_path);

	// convert image to grayscale ��ȡ�Ҷ�ͼ
	cvtColor(src, gray, COLOR_BGR2GRAY);

	// convert grayscale to binary image ��ֵ��
	threshold(gray, thr, 0, 255, THRESH_OTSU);

	// find moments of the image ��ȡ��ֵͼ��أ�true��ʾͼ���ֵ����
	Moments m = moments(thr, true);
	Point p(m.m10 / m.m00, m.m01 / m.m00);

	// coordinates of centroid ��������
	cout << Mat(p) << endl;

	// show the image with a point mark at the centroid ��������
	circle(src, p, 5, Scalar(128, 0, 0), -1);
	imshow("show", src);
	waitKey(0);
	return 0;
}