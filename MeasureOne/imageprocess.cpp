#include "imageprocess.h"
#include <QDebug>

ImageProcess::ImageProcess()
{
	IMethod *p;
	p = new MaxValMethod();
	methods[p->key()] = p;
	p = new MaxValMethod();
	methods["maxval"] = p;
	p = new MidPointMethod();
	methods[p->key()] = p;
	p = new MidPointMethod();
	methods["midpoint"] = p;
	p = new AdaptiveMethod();
	methods[p->key()] = p;
	p = new AdaptiveMethod();
	methods["adaptive"] = p;
	p = new CentroidMethod();
	methods[p->key()] = p;
	p = new CentroidMethod();
	methods["centroid"] = p;
	p = new OtsuMethod();
	methods[p->key()] = p;
	p = new OtsuMethod();
	methods[CN("otsu")] = p;
}

ImageProcess::~ImageProcess()
{
	//����������new�����Ķ���
	foreach(IMethod* p, pipeline) delete p;
	foreach(IMethod* p, methods.values()) delete p;
}

void ImageProcess::process(ImageObject &imgObj)
{
	foreach(IMethod* method, pipeline)
		method->process(imgObj);
}

bool ImageProcess::parse(QString code)
{
	QStringList cmdline = code.split('\n');
	QList<IMethod*> tempList;

	int i;
	for (i = 0; i < cmdline.length(); i++){
		QString cmd = cmdline[i].simplified(); //ȥ������ո�
		if (cmd == "" || cmd.startsWith("//")) continue; //ע�ͻ��߿���

		QStringList args = cmd.split(' ');
		QString name = args[0];

		IMethod* p = methods.value(name, nullptr); //��ȡ���ֶ�Ӧ�ķ�������
		if (p == nullptr) break;

		IMethod* pnew = p->create(args); //��������Ӧ�����ķ�������
		if (pnew == nullptr) break;

		tempList.append(pnew);
	}
	if (i == cmdline.length()){
		//������ȡ���
		pipeline = tempList;
		return true;
	}
	else{
		//ʧ�ܣ��ͷ�new�����Ķ���
		qDebug() << QString("imgproc parsing failed at:  ") + cmdline[i];//���������
		foreach(IMethod* m, tempList) delete m;
		return false;
	}
}

QString ImageProcess::toString(bool isRunning)
{
	QString str;
	foreach(IMethod *method, pipeline){
		QString line;
		if (isRunning)
			line = QString().sprintf("%3dms\t%1\n", method->dt).arg(method->toString());
		else
			line = method->toString() + "\n";
		str.append(line);
	}
	return str;
}

