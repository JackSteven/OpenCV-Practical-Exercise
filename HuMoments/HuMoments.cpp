#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main()
{
	//�Ƿ����logת��
	bool showLogTransformedHuMoments = true;

	// Obtain filename ͼ���ַ
	string filename("./image/s0.png");

	// Read Image ��ͼ
	Mat im = imread(filename, IMREAD_GRAYSCALE);

	// Threshold image ��ֵ�ָ�
	threshold(im, im, 0, 255, THRESH_OTSU);

	// Calculate Moments �����
	//�ڶ�������True��ʾ��������ض��ᰴֵ1�Դ���Ҳ����˵�൱�ڶ�ͼ������˶�ֵ��������ֵΪ1
	Moments moment = moments(im, false);

	// Calculate Hu Moments ����Hu��
	double huMoments[7];
	HuMoments(moment, huMoments);

	// Print Hu Moments
	cout << filename << ": ";

	for (int i = 0; i < 7; i++)
	{
		if (showLogTransformedHuMoments)
		{
			// Log transform Hu Moments to make squash the range
			cout << -1 * copysign(1.0, huMoments[i]) * log10(abs(huMoments[i])) << " ";
		}
		else
		{
			// Hu Moments without log transform. 
			cout << huMoments[i] << " ";
		}

	}
	// One row per file
	cout << endl;

}