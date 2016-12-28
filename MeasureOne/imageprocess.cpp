#include "imageprocess.h"
#include <QDebug>
#include <QMessageBox>

ImageProcess::ImageProcess()
{
	AbstractMethod *p;
	p = new MaxValMethod();
	methods[p->key()] = p;
	p = new MaxValMethod();
	methods["maxval"] = p;
	p = new MidPointMethod();
	methods[p->key()] = p;
	p = new MidPointMethod();
	methods["midpoint"] = p;
	p = new CentroidMethod();
	methods[p->key()] = p;
	p = new CentroidMethod();
	methods["centroid"] = p;
	p = new GxEdgeDivideMethod();
	methods[p->key()] = p;
	p = new OtsuThres();
	methods[p->key()] = p;
	p = new OtsuThres();
	methods["otsu"] = p;
	p = new OtsuByRowSegmn();
	methods[p->key()] = p;
	p = new AdaptiveSegmn();
	methods[p->key()] = p;
	p = new AdaptiveSegmn();
	methods["adaptive"] = p;
	p = new ThresholdSegmn();
	methods[p->key()] = p;
	p = new MixLaserResult();
	methods[p->key()] = p;
	p = new SrcImgResult();
	methods[p->key()] = p;
	p = new GrayImgResult();
	methods[p->key()] = p;
	p = new ZoomResult();
	methods[p->key()] = p;
}

ImageProcess::~ImageProcess()
{
	//����������new�����Ķ���
	foreach(AbstractMethod* p, pipeline) delete p;
	foreach(AbstractMethod* p, methods.values()) delete p;
}

int ImageProcess::process(ImageObject &imgObj)
{
	QMutexLocker locker(&imgObj.mutex);
	QMutexLocker locker2(&this->mutex);

	cvtColor(imgObj.src, imgObj.img, cv::COLOR_BGR2GRAY);

	QElapsedTimer tmr;
	tmr.start();

	foreach(AbstractMethod* method, pipeline)
		method->process(imgObj);

	return (int)tmr.elapsed();
}

bool ImageProcess::parse(QString code)
{
	QStringList cmdline = code.split('\n');
	QList<AbstractMethod*> tempList;

	int i;
	for (i = 0; i < cmdline.length(); i++){
		QString cmd = cmdline[i].simplified(); //ȥ������ո�
		if (cmd == "" || cmd.startsWith("//")) continue; //ע�ͻ��߿���

		QStringList args = cmd.split(' ');
		QString name = args[0];

		AbstractMethod* p = methods.value(name, nullptr); //��ȡ���ֶ�Ӧ�ķ�������
		if (p == nullptr) break;

		AbstractMethod* pnew = p->create(args); //��������Ӧ�����ķ�������
		if (pnew == nullptr) break;

		tempList.append(pnew);
	}
	if (i == cmdline.length()){
		//������ȡ���
		QMutexLocker locker(&this->mutex);
		foreach(AbstractMethod* m, pipeline) delete m;
		pipeline = tempList;
		return true;
	}
	else{
		//ʧ�ܣ��ͷ�new�����Ķ���
		//���������
		QMessageBox::information(nullptr, CN("�������"), CN("���������޷�������\n") + cmdline[i]);
		foreach(AbstractMethod* m, tempList) delete m;
		return false;
	}
}

QString ImageProcess::toString(bool isRunning)
{
	QString str;
	foreach(AbstractMethod *method, pipeline){
		QString line;
		if (isRunning)
			line = QString().sprintf("%3dms\t%1\n", method->dt).arg(method->toString());
		else
			line = method->toString() + "\n";
		str.append(line);
	}
	return str;
}

