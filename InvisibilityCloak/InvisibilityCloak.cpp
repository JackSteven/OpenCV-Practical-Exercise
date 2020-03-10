#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main()
{
	//����Ƶ
	VideoCapture cap("video/input.mp4");

	// �����Ƶ�Ƿ��
	if (!cap.isOpened())
	{
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	Mat background;
	//����ǰ30֡
	for (int i = 0; i < 30; i++)
	{
		cap >> background;
	}
	//��Y�ᷭתͼ��
	flip(background, background, 1);
	//�첼��251֡�ų��֣�����ǰ250֡
	for (int i = 0; i < 220; i++)
	{
		Mat frame_slip;
		cap >> frame_slip;
		continue;
	}


	//ͼ���ȡ
	while (1)
	{
		//���֡
		Mat frame;

		// Capture frame-by-frame
		cap >> frame;

		// If the frame is empty, break immediately
		if (frame.empty())
		{
			break;
		}
		//hsvͼ��
		Mat hsv;
		flip(frame, frame, 1);
		cvtColor(frame, hsv, COLOR_BGR2HSV);

		//��ɫ����1����ɫ����2
		Mat mask1, mask2;
		//��ɫ����
		Mat mask_red;
		//��������
		Mat mask_background;
		//������ɫ
		//��ֵͼ�����к�ɫ0��ʾ�޺�ɫ����ɫ1��ʾ��ɫ����
		inRange(hsv, Scalar(0, 120, 70), Scalar(10, 255, 255), mask1);
		inRange(hsv, Scalar(170, 120, 70), Scalar(180, 255, 255), mask2);
		mask_red = mask1 + mask2;


		//ȥ������
		Mat kernel = Mat::ones(3, 3, CV_32F);
		morphologyEx(mask_red, mask_red, cv::MORPH_OPEN, kernel);
		morphologyEx(mask_red, mask_red, cv::MORPH_DILATE, kernel);


		//��mask_red��0��1�������õ���������Χ��
		bitwise_not(mask_red, mask_background);
		Mat res1, res2, final_output;
		//�ӵ�ǰ֡�ٳ���������res1,�첼����Ϳ�ɺ�ɫ��
		bitwise_and(frame, frame, res1, mask_background);
		//�ӱ���֡��ȡ�첼���򸲸ǵı���res2
		bitwise_and(background, background, res2, mask_red);

		addWeighted(res1, 1, res2, 1, 0, final_output);
		//չʾͼ��
		imshow("Magic !!!", res1);
		// Press  ESC on keyboard to exit
		char c = (char)waitKey(1);
		if (c == 27)
		{
			break;
		}
	}

	return 0;
}