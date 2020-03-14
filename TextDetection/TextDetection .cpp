#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;

//����
void decode(const Mat &scores, const Mat &geometry, float scoreThresh,
	std::vector<RotatedRect> &detections, std::vector<float> &confidences);

/**
 * @brief
 *
 * @param srcImg ���ͼ��
 * @param inpWidth ���ѧϰͼ�������
 * @param inpHeight ���ѧϰͼ�������
 * @param confThreshold ���Ŷ�
 * @param nmsThreshold �Ǽ���ֵ�����㷨��ֵ
 * @param net
 * @return Mat
 */
Mat text_detect(Mat srcImg, int inpWidth, int inpHeight, float confThreshold, float nmsThreshold, Net net)
{
	//���
	std::vector<Mat> output;
	std::vector<String> outputLayers(2);
	outputLayers[0] = "feature_fusion/Conv_7/Sigmoid";
	outputLayers[1] = "feature_fusion/concat_3";

	//���ͼ��
	Mat frame, blob;
	frame = srcImg.clone();
	//��ȡ���ѧϰģ�͵�����
	blobFromImage(frame, blob, 1.0, Size(inpWidth, inpHeight), Scalar(123.68, 116.78, 103.94), true, false);
	net.setInput(blob);
	//������
	net.forward(output, outputLayers);

	//���Ŷ�
	Mat scores = output[0];
	//λ�ò���
	Mat geometry = output[1];

	// Decode predicted bounding boxes�� �Լ�����н��룬��ȡ�ı���λ�÷���
	//�ı���λ�ò���
	std::vector<RotatedRect> boxes;
	//�ı������Ŷ�
	std::vector<float> confidences;
	decode(scores, geometry, confThreshold, boxes, confidences);

	// Apply non-maximum suppression procedure�� Ӧ�÷Ǽ����������㷨
	//����Ҫ����ı���
	std::vector<int> indices;
	NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

	// Render detections. ���Ԥ��
	//���ű���
	Point2f ratio((float)frame.cols / inpWidth, (float)frame.rows / inpHeight);
	for (size_t i = 0; i < indices.size(); ++i)
	{
		RotatedRect &box = boxes[indices[i]];

		Point2f vertices[4];
		box.points(vertices);
		//��ԭ�����
		for (int j = 0; j < 4; ++j)
		{
			vertices[j].x *= ratio.x;
			vertices[j].y *= ratio.y;
		}
		//����
		for (int j = 0; j < 4; ++j)
		{
			line(frame, vertices[j], vertices[(j + 1) % 4], Scalar(0, 255, 0), 2, LINE_AA);
		}
	}

	// Put efficiency information. ʱ��
	std::vector<double> layersTimes;
	double freq = getTickFrequency() / 1000;
	double t = net.getPerfProfile(layersTimes) / freq;
	std::string label = format("Inference time: %.2f ms", t);
	putText(frame, label, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0));

	return frame;
}

//ģ�͵�ַ
auto model = "frozen_east_text_detection.pb";
//���ͼ��
auto detect_image = "patient.jpg";
//�����ߴ�
auto inpWidth = 320;
auto inpHeight = 320;
//���Ŷ���ֵ
auto confThreshold = 0.5;
//�Ǽ���ֵ�����㷨��ֵ
auto nmsThreshold = 0.4;

int main()
{
	//��ȡģ��
	Net net = readNet(model);
	//��ȡ���ͼ��
	Mat srcImg = imread(detect_image);
	if (!srcImg.empty())
	{
		cout << "read image success!" << endl;
	}
	Mat resultImg = text_detect(srcImg, inpWidth, inpHeight, confThreshold, nmsThreshold, net);
	imshow("result", resultImg);
	waitKey();
	return 0;
}

/**
 * @brief �����⵽���ı��������Ϣ
 *
 * @param scores ���Ŷ�
 * @param geometry λ����Ϣ
 * @param scoreThresh ���Ŷ���ֵ
 * @param detections λ��
 * @param confidences �������
 */
void decode(const Mat &scores, const Mat &geometry, float scoreThresh, std::vector<RotatedRect> &detections, std::vector<float> &confidences)
{
	detections.clear();
	//�ж��ǲ��Ƿ�����ȡҪ��
	CV_Assert(scores.dims == 4);
	CV_Assert(geometry.dims == 4);
	CV_Assert(scores.size[0] == 1);
	CV_Assert(geometry.size[0] == 1);
	CV_Assert(scores.size[1] == 1);
	CV_Assert(geometry.size[1] == 5);
	CV_Assert(scores.size[2] == geometry.size[2]);
	CV_Assert(scores.size[3] == geometry.size[3]);

	const int height = scores.size[2];
	const int width = scores.size[3];
	for (int y = 0; y < height; ++y)
	{
		//ʶ�����
		const float *scoresData = scores.ptr<float>(0, 0, y);
		//�ı�������
		const float *x0_data = geometry.ptr<float>(0, 0, y);
		const float *x1_data = geometry.ptr<float>(0, 1, y);
		const float *x2_data = geometry.ptr<float>(0, 2, y);
		const float *x3_data = geometry.ptr<float>(0, 3, y);
		//�ı���Ƕ�
		const float *anglesData = geometry.ptr<float>(0, 4, y);
		//�������м�⵽�ļ���
		for (int x = 0; x < width; ++x)
		{
			float score = scoresData[x];
			//������ֵ���Ըü���
			if (score < scoreThresh)
			{
				continue;
			}

			// Decode a prediction.
			// Multiple by 4 because feature maps are 4 time less than input image.
			float offsetX = x * 4.0f, offsetY = y * 4.0f;
			//�Ƕȼ���������Ҽ���
			float angle = anglesData[x];
			float cosA = std::cos(angle);
			float sinA = std::sin(angle);
			float h = x0_data[x] + x2_data[x];
			float w = x1_data[x] + x3_data[x];

			Point2f offset(offsetX + cosA * x1_data[x] + sinA * x2_data[x], offsetY - sinA * x1_data[x] + cosA * x2_data[x]);
			Point2f p1 = Point2f(-sinA * h, -cosA * h) + offset;
			Point2f p3 = Point2f(-cosA * w, sinA * w) + offset;
			//��ת���Σ��ֱ��������ĵ����꣬ͼ���ߣ��Ƕ�
			RotatedRect r(0.5f * (p1 + p3), Size2f(w, h), -angle * 180.0f / (float)CV_PI);
			//�������
			detections.push_back(r);
			//�����������Ŷ�
			confidences.push_back(score);
		}
	}
}