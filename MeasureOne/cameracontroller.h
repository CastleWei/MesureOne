#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <opencv.hpp>

//滤波算法
enum FilterType{
	NoFilter,
	Average,
	Gaussian,
	Median,
};

//阈值算法
enum ThresType{
	Manual,
	MaxValue,
	Otsu,
	AutoThres
};

//！！！！目前暂时不用区分
//中心提取算法
enum CenterType{
	NoCenter,
	Centroid, //重心法
	Midpoint  //中点 (a+b)/2
};

class CameraController : public QThread
{
	Q_OBJECT

public:
	CameraController(QObject *parent);
	~CameraController();

	void start();
	void end();

	FilterType filterType = NoFilter;
	ThresType thresType = MaxValue;
	qint64 dt;
	uint ksize = 5;
	int thres = 128; //阈值
	bool isReady = false;

	void setThres(int val);
	QImage getImage();
	QImage getIntermediateImage();

private:
	bool isOn = false;
	bool mixed = true;
	float *pRes = NULL; //存放结果：每行偏移量
	QElapsedTimer elpsTmr;
	QMutex mutex;
	cv::VideoCapture cap;
	cv::Mat srcRgbImg;
	cv::Mat srcGrayImg;
	cv::Mat dstImg;
	cv::Mat retImg; //返回给paintEvent的mat
	
	void run();
	void proccess();
	
	void maxValueMethod();
	void otsuMethod();
	void autoThresMethod();
};

#endif // CAMERACONTROLLER_H
