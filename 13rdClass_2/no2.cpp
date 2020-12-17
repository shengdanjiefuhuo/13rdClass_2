#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void calcGaussianModel(vector<Mat> backMats, Mat& meanMat, Mat& varMat);//计算模型函数，计算均值和方差
void GaussianThreshold(int wVar, void* userdata);                       //利用均值mat和方差mat，计算差分
void Gauss_Ksize(int, void*);                                           //高斯滤波Ksize回调函数
Mat meanMat;//均值mat
Mat varMat;//方差mat
Mat dst;//差分结果
Mat frame;
int Var_Weight = 30; //方差权重
int KsizeValue = 5;  //Ksize滑动条初始位置

int main()
{
	VideoCapture cap(0);//打开摄像头
	//如果视频打开失败
	if (!cap.isOpened())
	{
		cout << "不能打开视频" << endl;
		return -1;
		destroyAllWindows();
	}

	//用来计算背景模型的图像
	vector<Mat> backMats;

	int nframe = 200;	//用来建立背景模型的帧数量
	int cnt = 0;	    //计数值

	while (1)
	{
		cap >> frame;
		cvtColor(frame, frame, COLOR_BGR2GRAY);

		if (frame.empty())
		{
			cout << "不能读取视频帧" << endl;
			return -1;
			destroyAllWindows();
		}

		//存储前面的nframe帧
		if (cnt < nframe)
		{
			backMats.push_back(frame);
			if (cnt == 0)
			{
				cout << "--- 正在读取帧 --- " << endl;
			}
			else if (cnt % 10 == 0)
				cout << "-" << endl;
		}
		else if (cnt == nframe)
		{
			//利用前面的nframe帧，计算高斯模型
			cout << "计算高斯模型" << endl;
			meanMat.create(frame.size(), CV_8UC1);
			varMat.create(frame.size(), CV_32FC1);
			//调用计算高斯模型函数
			calcGaussianModel(backMats, meanMat, varMat);
		}
		else
		{
			//计算背景差分
			dst.create(frame.size(), CV_8UC1);
			//利用均值mat和方差mat，计算差分
			namedWindow("dst");
			//创建滑动条
			createTrackbar("threshohd", "dst", &Var_Weight, 100, GaussianThreshold, &frame);
			createTrackbar("内核尺寸", "dst", &KsizeValue, 15, Gauss_Ksize);
			//调用回调函数
			Gauss_Ksize(0, 0);
			GaussianThreshold(Var_Weight, &frame);

			imshow("dst", dst);
			imshow("frame", frame);               //显示当前帧
			imshow("background", meanMat);        //显示背景
		}
		//延时30ms
		//等待键盘相应，按下ESC键退出
		if (waitKey(30) == 27)
		{
			destroyAllWindows();
			break;
		}
		cnt++;
	}
	return 0;
}

//计算模型函数，计算均值和方差
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
			int nframe = backMats.size();//帧个数
			//求均值
			for (int i = 0; i < nframe; i++)
			{
				sum += backMats[i].at<uchar>(row, col);
			}
			meanMat.at<uchar>(row, col) = sum / nframe;

			//求方差
			for (int j = 0; j < nframe; j++)
			{
				var += pow((backMats[j].at<uchar>(row, col) - meanMat.at<uchar>(row, col)), 2);
			}
			varMat.at<float>(row, col) = var / nframe;
		}
	}
}



//回调函数
//利用均值mat和方差mat，计算差分
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

void Gauss_Ksize(int, void*)       //高斯滤波Ksize回调函数
{
	int length = KsizeValue * 2 + 1;
	GaussianBlur(frame, frame, Size(length, length), 5.5, 5.5);
}