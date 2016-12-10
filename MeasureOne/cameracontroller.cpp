#include "cameracontroller.h"

CameraController::CameraController(QObject *parent)
	: QThread(parent)
{

}

CameraController::~CameraController()
{
	this->isOn = false;
	if (pRes != NULL) delete[] pRes;
	cap.release();
}

void CameraController::setThres(int val)
{
	if (this->thresType == Manual) thres = val;
}

QImage CameraController::getImage()
{
	//mutex->lock();

	//cv::Mat mat;
	//cv::cvtColor(srcImg, mat, cv::COLOR_GRAY2RGB);
	//QImage img(mat.data, mat.cols, mat.rows, QImage::Format_RGB32);
	//mutex->unlock();
	////将计算结果光刀显示为红线
	//uchar* p = img.bits();
	//int step = img.bytesPerLine();
	//for (int i = 0; i < img.height(); i++){
	//	if (pRes[i] != 0){
	//		int n = static_cast<int>(pRes[i] + 0.5f); //四舍五入
	//		p[n << 2] = 255; //每个像素四字节，n<<2即n*4，R在第一字节
	//	}
	//	p += step;
	//}
	mutex.lock();
	cv::cvtColor(dstImg, retImg, cv::COLOR_GRAY2RGB);
	mutex.unlock();

	QImage img(retImg.data, retImg.cols, retImg.rows, 
		retImg.channels()*retImg.cols, QImage::Format_RGB888);

	return 	img;
}

QImage CameraController::getIntermediateImage()
{
	QImage img;

	return img;
}

void CameraController::start()
{
	this->isOn = true;
	QThread::start();
}

void CameraController::end()
{
	this->isOn = false;
}

void CameraController::run()
{
	cap.open(1);
	mutex.lock();
	cap >> srcGrayImg;
	mutex.unlock();

	//用于存放结果的数组：每行偏移量
	pRes = new float[srcGrayImg.rows];

	while (isOn){
		mutex.lock();
		cap >> srcRgbImg;
		cvtColor(srcRgbImg, srcGrayImg, cv::COLOR_BGR2GRAY);
		mutex.unlock();

		proccess();
		isReady = true;
		dt = elpsTmr.restart();
		msleep(15); //sleep的单位是秒！！！必须要用msleep！！
	}
}

void CameraController::proccess()
{
	QMutexLocker locker(&mutex);
	switch (filterType)
	{
	case NoFilter:
		dstImg = srcGrayImg.clone();
		break;
	case Average:
		cv::blur(srcGrayImg, dstImg, cv::Size(ksize, ksize));
		break;
	case Gaussian:
		cv::GaussianBlur(srcGrayImg, dstImg, cv::Size(ksize, ksize), 0, 0);
		break;
	case Median:
		cv::medianBlur(srcGrayImg, dstImg, ksize);
		break;
	default:
		break;
	}

	switch (thresType)
	{
	case MaxValue:
		//maxValueMethod();
		break;
	case Otsu:
		otsuMethod();
		break;
	case AutoThres:
		autoThresMethod();
		break;
	default:
		break;
	}
}

void CameraController::maxValueMethod()
{
	uchar *p = dstImg.data;
	int offset = 0;
	int step = dstImg.step1(); //一行的长度
	for (int i = 0; i < dstImg.rows; i++){
		int iMax = 0;
		for (int j = 1; j < dstImg.cols; j++){
			if (p[offset + j]>p[offset + iMax])
				iMax = j;
		}
		pRes[i] = iMax;
		offset += step;
	}
}

void CameraController::otsuMethod()
{

}

void CameraController::autoThresMethod()
{

}
