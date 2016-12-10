#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <opencv.hpp>

//�˲��㷨
enum FilterType{
	NoFilter,
	Average,
	Gaussian,
	Median,
};

//��ֵ�㷨
enum ThresType{
	Manual,
	MaxValue,
	Otsu,
	AutoThres
};

//��������Ŀǰ��ʱ��������
//������ȡ�㷨
enum CenterType{
	NoCenter,
	Centroid, //���ķ�
	Midpoint  //�е� (a+b)/2
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
	int thres = 128; //��ֵ
	bool isReady = false;

	void setThres(int val);
	QImage getImage();
	QImage getIntermediateImage();

private:
	bool isOn = false;
	bool mixed = true;
	float *pRes = NULL; //��Ž����ÿ��ƫ����
	QElapsedTimer elpsTmr;
	QMutex mutex;
	cv::VideoCapture cap;
	cv::Mat srcRgbImg;
	cv::Mat srcGrayImg;
	cv::Mat dstImg;
	cv::Mat retImg; //���ظ�paintEvent��mat
	
	void run();
	void proccess();
	
	void maxValueMethod();
	void otsuMethod();
	void autoThresMethod();
};

#endif // CAMERACONTROLLER_H
