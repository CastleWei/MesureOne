#include "cameracontroller.h"

CameraController::CameraController(QObject *parent, ImageProcess &imgProc)
	: QThread(parent), imgProc(imgProc)
{

}

CameraController::~CameraController()
{
	this->end();
}

void CameraController::start()
{
	if (isOn){
		isOn = false;
		wait();
	}

	this->isOn = true;
	QThread::start();
}

void CameraController::end()
{
	this->isOn = false;
	this->wait();
	this->dt = 0;
}

void CameraController::run()
{
	imgObj.clear();
	if (srcType == fromPic)
		runPic();
	else
		runVideoCam();

	emit finishCollecting(); //进程结束后发出结束信号
}

void CameraController::runPic()
{
	imgObj.src = cv::imread(path.toStdString());
	if (imgObj.src.empty()){
		qDebug() << CN("打开图像失败");
		return;
	}
	imgObj.newOffs(imgObj.src.rows);
	qDebug() << imgObj.offsLen;
	dt = imgProc.process(imgObj);
	emit imageCollected();
}

void CameraController::runVideoCam()
{
	cv::VideoCapture cap;
	bool ok;
	if (srcType == fromCam)
		ok = cap.open(camNum);
	else
		ok = cap.open(path.toStdString());
	if (!ok){
		qDebug() << "cannot open camera or video file";
		return;
	}

	cv::Mat &src = imgObj.src;
	cv::Mat &img = imgObj.img;
	cv::Mat &dst = imgObj.dst;

	//第一次读取，初始化光刀偏移量数组
	{
		QMutexLocker(&imgObj.mutex);
		cap >> src;
		if (src.empty()) return;
		imgObj.newOffs(src.rows);
		qDebug() << imgObj.offsLen;
	}

	elpsTmr.start();

	while (isOn){
		//调整进度
		//if (srcType == fromVideo){
		//	camMutex.lock();
		//	if (jump){
		//		cap.set(CV_CAP_PROP_POS_AVI_RATIO, newpos); //！！！不能用
		//		jump = false;
		//	}
		//	camMutex.unlock();
		//	//curpos = cap.get(CV_CAP_PROP_POS_AVI_RATIO); //！！！！不能用
		//	//到不了100%，不知道为什么
		//	curpos = cap.get(CV_CAP_PROP_POS_FRAMES) / cap.get(CV_CAP_PROP_FRAME_COUNT);
		//	qDebug() << curpos;
		//}
		
		//读取每帧图片并处理
		{
			QMutexLocker(&imgObj.mutex);
			if (!cap.read(src)) break;
			//cvtColor(src, img, COLOR_BGR2GRAY); 在imgProc.process里面做了
		}

		imgProc.process(imgObj);

		//后续处理
		dt = (int)elpsTmr.restart();

		emit imageCollected();

		if (willRest) msleep(msecRest); //休息

		while (!isPlaying && isOn) msleep(200); //如果按了暂停，自旋等待
	}
	qDebug() << CN("视频采集线程关闭");
}
