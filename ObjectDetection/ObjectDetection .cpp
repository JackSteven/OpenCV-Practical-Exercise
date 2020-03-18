#include <fstream>
#include <sstream>
#include <iostream>
#include <time.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

;
using namespace cv;
using namespace dnn;
using namespace std;

// Initialize the parameters ��ʼ����
 // Confidence threshold ���Ŷ���ֵ
float confThreshold = 0.5;
// Non-maximum suppression threshold �Ǽ�����������ֵ
float nmsThreshold = 0.4;
//���ͼ����
int inpWidth = 416;
int inpHeight = 416;
//������
vector<string> classes;

// Remove the bounding boxes with low confidence using non-maxima suppression
// ���ڷǼ���������ȥ�������Ŷȵļ���
void postprocess(Mat& frame, const vector<Mat>& out);

// ��Ԥ���
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

// ��ȡ��������
vector<String> getOutputsNames(const Net& net);

int main()
{
	// Give the configuration and weight files for the model ģ�Ͳ����ļ�
	String modelConfiguration = "yolov3.cfg";
	String modelWeights = "yolov3.weights";
	// Load names of classes ��ȡ��������
	string classesFile = "coco.names";
	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line))
	{
		classes.push_back(line);
	}

	VideoCapture cap("run.mp4");

	// Load the network ��������
	Net net = readNetFromDarknet(modelConfiguration, modelWeights);
	net.setPreferableBackend(DNN_BACKEND_OPENCV);
	//����ʹ��CPU
	net.setPreferableTarget(DNN_TARGET_CPU);

	// Open a video file or an image file or a camera stream.
	string str, outputFile;
	Mat blob;
	clock_t start, finish;

	while (waitKey(1) < 0) {
		//��ͼ
		//Mat frame = imread("bird.jpg");
		Mat frame;
		cap.read(frame);
		//������������ǲ�������ͼ��
		//resize(frame, frame, Size(300, 300));

		start = clock();
		// Create a 4D blob from a frame. ��������������ͼ��
		blobFromImage(frame, blob, 1 / 255.0, Size(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);

		//Sets the input to the network �������
		net.setInput(blob);

		// Runs the forward pass to get output of the output layers ��ȡ�������
		vector<Mat> outs;
		net.forward(outs, getOutputsNames(net));

		// Remove the bounding boxes with low confidence
		postprocess(frame, outs);
		finish = clock();

		cout << "time is " << double(finish - start) / CLOCKS_PER_SEC << endl;
		// Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes)
		//���ǰ�򴫲���ʱ��
		vector<double> layersTimes;
		double freq = getTickFrequency() / 1000;
		double t = net.getPerfProfile(layersTimes) / freq;
		string label = format("Inference time for a frame : %.2f ms", t);
		putText(frame, label, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));

		imshow("result", frame);
	}
	
	//����ͼ��
	//imwrite("result.jpg", frame);
	waitKey(0);

	return 0;
}

/**
 * @brief Remove the bounding boxes with low confidence using non-maxima suppression ���ڷǼ���������ȥ���߿�
 *
 * @param frame ��Ƶͼ��
 * @param outs �������
 */
void postprocess(Mat& frame, const vector<Mat>& outs)
{
	//�����
	vector<int> classIds;
	//���Ŷ�
	vector<float> confidences;
	vector<Rect> boxes;

	//�������е������
	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		//ɨ��������������ı߽�������ֻ�������и����Ŷȷ����ı߽�򡣽�������ǩָ��Ϊ��÷���ߵ��ࡣ
		//��ȡ��
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score ��ȡ���ŶȺ�λ�ò���
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			//����������Ŷ���ֵ
			if (confidence > confThreshold)
			{
				//��ȡ����
				int centerX = (int)(data[0] * frame.cols);
				int centerY = (int)(data[1] * frame.rows);
				int width = (int)(data[2] * frame.cols);
				int height = (int)(data[3] * frame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(Rect(left, top, width, height));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	//����Ǽ��������ƽ���������ŶȴӴ�С���
	vector<int> indices;
	//�Ǽ���������
	NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
	//��ͼ
	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		Rect box = boxes[idx];
		//�࣬���Ŷ�
		drawPred(classIds[idx], confidences[idx], box.x, box.y,
			box.x + box.width, box.y + box.height, frame);
	}
}

/**
 * @brief Draw the predicted bounding box ����
 *
 * @param classId ���
 * @param conf ���Ŷ�
 * @param left
 * @param top
 * @param right
 * @param bottom
 * @param frame
 */
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
{
	//Draw a rectangle displaying the bounding box
	rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

	//Get the label for the class name and its confidence
	string label = format("%.2f", conf);
	if (!classes.empty())
	{
		CV_Assert(classId < (int)classes.size());
		label = classes[classId] + ":" + label;
	}

	//Display the label at the top of the bounding box ��ÿ�������ϽǱ��ϱ�ǩ
	int baseLine;
	Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	top = max(top, labelSize.height);
	rectangle(frame, Point(left, top - round(1.5*labelSize.height)), Point(left + round(1.5*labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
	putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
}

// Get the names of the output layers ��ȡ�����
/**
 * @brief Get the Outputs Names object
 *
 * @param net
 * @return vector<String>
 */
vector<String> getOutputsNames(const Net& net)
{
	//���
	static vector<String> names;
	if (names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		vector<int> outLayers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		vector<String> layersNames = net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
		{
			names[i] = layersNames[outLayers[i] - 1];
		}
	}
	return names;
}