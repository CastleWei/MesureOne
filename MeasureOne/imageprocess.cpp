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
	//清理容器中new出来的对象
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
		QString cmd = cmdline[i].simplified(); //去除多余空格
		if (cmd == "" || cmd.startsWith("//")) continue; //注释或者空行

		QStringList args = cmd.split(' ');
		QString name = args[0];

		IMethod* p = methods.value(name, nullptr); //读取名字对应的方法对象
		if (p == nullptr) break;

		IMethod* pnew = p->create(args); //创建含对应参数的方法对象
		if (pnew == nullptr) break;

		tempList.append(pnew);
	}
	if (i == cmdline.length()){
		//正常读取完毕
		pipeline = tempList;
		return true;
	}
	else{
		//失败，释放new出来的对象
		qDebug() << QString("imgproc parsing failed at:  ") + cmdline[i];//输出错误行
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

