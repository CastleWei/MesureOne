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

	emit finishCollecting(); //���̽����󷢳������ź�
}

void CameraController::runPic()
{
	imgObj.src = cv::imread(path.toStdString());
	if (imgObj.src.empty()){
		qDebug() << CN("��ͼ��ʧ��");
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

	//��һ�ζ�ȡ����ʼ���⵶ƫ��������
	{
		QMutexLocker(&imgObj.mutex);
		cap >> src;
		if (src.empty()) return;
		imgObj.newOffs(src.rows);
		qDebug() << imgObj.offsLen;
	}

	elpsTmr.start();

	while (isOn){
		//��������
		//if (srcType == fromVideo){
		//	camMutex.lock();
		//	if (jump){
		//		cap.set(CV_CAP_PROP_POS_AVI_RATIO, newpos); //������������
		//		jump = false;
		//	}
		//	camMutex.unlock();
		//	//curpos = cap.get(CV_CAP_PROP_POS_AVI_RATIO); //��������������
		//	//������100%����֪��Ϊʲô
		//	curpos = cap.get(CV_CAP_PROP_POS_FRAMES) / cap.get(CV_CAP_PROP_FRAME_COUNT);
		//	qDebug() << curpos;
		//}
		
		//��ȡÿ֡ͼƬ������
		{
			QMutexLocker(&imgObj.mutex);
			if (!cap.read(src)) break;
			//cvtColor(src, img, COLOR_BGR2GRAY); ��imgProc.process��������
		}

		imgProc.process(imgObj);

		//��������
		dt = (int)elpsTmr.restart();

		emit imageCollected();

		if (willRest) msleep(msecRest); //��Ϣ

		while (!isPlaying && isOn) msleep(200); //���������ͣ�������ȴ�
	}
	qDebug() << CN("��Ƶ�ɼ��̹߳ر�");
}
