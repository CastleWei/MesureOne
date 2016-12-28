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
	//清理容器中new出来的对象
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
		QString cmd = cmdline[i].simplified(); //去除多余空格
		if (cmd == "" || cmd.startsWith("//")) continue; //注释或者空行

		QStringList args = cmd.split(' ');
		QString name = args[0];

		AbstractMethod* p = methods.value(name, nullptr); //读取名字对应的方法对象
		if (p == nullptr) break;

		AbstractMethod* pnew = p->create(args); //创建含对应参数的方法对象
		if (pnew == nullptr) break;

		tempList.append(pnew);
	}
	if (i == cmdline.length()){
		//正常读取完毕
		QMutexLocker locker(&this->mutex);
		foreach(AbstractMethod* m, pipeline) delete m;
		pipeline = tempList;
		return true;
	}
	else{
		//失败，释放new出来的对象
		//输出错误行
		QMessageBox::information(nullptr, CN("命令错误"), CN("该行命令无法解析：\n") + cmdline[i]);
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

