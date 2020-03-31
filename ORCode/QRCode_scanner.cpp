#include <iostream>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

/**
 * @brief ������ʾ��⵽��QR����Χ�Ŀ�
 *
 * @param im
 * @param bbox
 */
void display(Mat &im, Mat &bbox)
{
	int n = bbox.rows;
	for (int i = 0; i < n; i++)
	{
		line(im, Point2i(bbox.at<float>(i, 0), bbox.at<float>(i, 1)),
			Point2i(bbox.at<float>((i + 1) % n, 0), bbox.at<float>((i + 1) % n, 1)), Scalar(255, 0, 0), 3);
	}
	imshow("Result", im);
}

int main()
{
	// Read image
	Mat inputImage = imread("demo.jpg");

	//QR�����
	QRCodeDetector qrDecoder = QRCodeDetector::QRCodeDetector();

	//��ά��߿����꣬��ȡ�����Ķ�ά��
	Mat bbox, rectifiedImage;

	//����ά��
	std::string data = qrDecoder.detectAndDecode(inputImage, bbox, rectifiedImage);

	//��ȡ��ά���е���������
	if (data.length() > 0)
	{
		cout << "Decoded Data : " << data << endl;
		display(inputImage, bbox);
		rectifiedImage.convertTo(rectifiedImage, CV_8UC3);
		//չʾ��ά��
		imshow("Rectified QRCode", rectifiedImage);

		waitKey(0);
	}
	else
	{
		cout << "QR Code not detected" << endl;
	}
	return 0;
}