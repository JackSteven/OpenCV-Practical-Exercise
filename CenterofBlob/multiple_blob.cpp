#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

RNG rng(12345);

void find_moments(Mat src);

int main()
{
	String img_path = "multiple.png";
	/// Load source image, convert it to gray
	Mat src, gray;
	src = imread(img_path);

	cvtColor(src, gray, COLOR_BGR2GRAY);

	//��ʾԭͼ
	namedWindow("Source", WINDOW_AUTOSIZE);
	imshow("Source", src);

	// call function to find_moments Ѱ���ĺ���
	find_moments(gray);

	waitKey(0);
	return(0);
}

void find_moments(Mat gray)
{
	Mat canny_output;
	//���������ĵ㼯��
	vector<vector<Point> > contours;
	//��������������
	vector<Vec4i> hierarchy;

	/// Detect edges using canny ��Ե������ȡ����
	Canny(gray, canny_output, 50, 150, 3);
	// Find contours Ѱ������ RETR_TREE��ʾ��ȡ��������
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Get the moments ͼ���
	vector<Moments> mu(contours.size());
	//��ȡÿ�������ľ�
	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	///  Get the centroid of figures. �����ʵ�
	vector<Point2f> mc(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	/// Draw contours
	//������
	Mat drawing(canny_output.size(), CV_8UC3, Scalar(255, 255, 255));

	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(167, 151, 0);
		//������
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
		//������
		circle(drawing, mc[i], 4, color, -1, 7, 0);
	}

	/// Show the resultant image
	namedWindow("Contours", WINDOW_AUTOSIZE);
	imshow("Contours", drawing);
	waitKey(0);
}