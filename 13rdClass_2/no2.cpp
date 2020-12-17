#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void calcGaussianModel(vector<Mat> backMats, Mat& meanMat, Mat& varMat);//����ģ�ͺ����������ֵ�ͷ���
void GaussianThreshold(int wVar, void* userdata);                       //���þ�ֵmat�ͷ���mat��������
void Gauss_Ksize(int, void*);                                           //��˹�˲�Ksize�ص�����
Mat meanMat;//��ֵmat
Mat varMat;//����mat
Mat dst;//��ֽ��
Mat frame;
int Var_Weight = 30; //����Ȩ��
int KsizeValue = 5;  //Ksize��������ʼλ��

int main()
{
	VideoCapture cap(0);//������ͷ
	//�����Ƶ��ʧ��
	if (!cap.isOpened())
	{
		cout << "���ܴ���Ƶ" << endl;
		return -1;
		destroyAllWindows();
	}

	//�������㱳��ģ�͵�ͼ��
	vector<Mat> backMats;

	int nframe = 200;	//������������ģ�͵�֡����
	int cnt = 0;	    //����ֵ

	while (1)
	{
		cap >> frame;
		cvtColor(frame, frame, COLOR_BGR2GRAY);

		if (frame.empty())
		{
			cout << "���ܶ�ȡ��Ƶ֡" << endl;
			return -1;
			destroyAllWindows();
		}

		//�洢ǰ���nframe֡
		if (cnt < nframe)
		{
			backMats.push_back(frame);
			if (cnt == 0)
			{
				cout << "--- ���ڶ�ȡ֡ --- " << endl;
			}
			else if (cnt % 10 == 0)
				cout << "-" << endl;
		}
		else if (cnt == nframe)
		{
			//����ǰ���nframe֡�������˹ģ��
			cout << "�����˹ģ��" << endl;
			meanMat.create(frame.size(), CV_8UC1);
			varMat.create(frame.size(), CV_32FC1);
			//���ü����˹ģ�ͺ���
			calcGaussianModel(backMats, meanMat, varMat);
		}
		else
		{
			//���㱳�����
			dst.create(frame.size(), CV_8UC1);
			//���þ�ֵmat�ͷ���mat��������
			namedWindow("dst");
			//����������
			createTrackbar("threshohd", "dst", &Var_Weight, 100, GaussianThreshold, &frame);
			createTrackbar("�ں˳ߴ�", "dst", &KsizeValue, 15, Gauss_Ksize);
			//���ûص�����
			Gauss_Ksize(0, 0);
			GaussianThreshold(Var_Weight, &frame);

			imshow("dst", dst);
			imshow("frame", frame);               //��ʾ��ǰ֡
			imshow("background", meanMat);        //��ʾ����
		}
		//��ʱ30ms
		//�ȴ�������Ӧ������ESC���˳�
		if (waitKey(30) == 27)
		{
			destroyAllWindows();
			break;
		}
		cnt++;
	}
	return 0;
}

//����ģ�ͺ����������ֵ�ͷ���
void calcGaussianModel(vector<Mat> backMats, Mat& meanMat, Mat& varMat)
{
	int height = backMats[0].rows;
	int width = backMats[0].cols;

	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			int sum = 0;
			float var = 0;
			int nframe = backMats.size();//֡����
			//���ֵ
			for (int i = 0; i < nframe; i++)
			{
				sum += backMats[i].at<uchar>(row, col);
			}
			meanMat.at<uchar>(row, col) = sum / nframe;

			//�󷽲�
			for (int j = 0; j < nframe; j++)
			{
				var += pow((backMats[j].at<uchar>(row, col) - meanMat.at<uchar>(row, col)), 2);
			}
			varMat.at<float>(row, col) = var / nframe;
		}
	}
}



//�ص�����
//���þ�ֵmat�ͷ���mat��������
void GaussianThreshold(int wVar, void* userdata)
{
	float weight = (float)wVar * 0.1;
	Mat src = *(Mat*)userdata;
	int height = src.rows;
	int width = src.cols;

	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			int diff = abs(src.at<uchar>(row, col) - meanMat.at<uchar>(row, col));
			int th = weight * varMat.at<float>(row, col);
			if (diff > th)
			{
				dst.at<uchar>(row, col) = 255;
			}
			else
			{
				dst.at<uchar>(row, col) = 0;
			}
		}
	}

}

void Gauss_Ksize(int, void*)       //��˹�˲�Ksize�ص�����
{
	int length = KsizeValue * 2 + 1;
	GaussianBlur(frame, frame, Size(length, length), 5.5, 5.5);
}