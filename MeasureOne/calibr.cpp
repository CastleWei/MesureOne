#include "calibr.h"

Calibr::Calibr(MotionController &motion, ImageProcess &imgProc, ImageObject &imgObj)
	: motion(motion)
	, imgProc(imgProc)
	, imgObj(imgObj)
{

}

Calibr::~Calibr()
{

}

void Calibr::timerEvent(QTimerEvent *)
{
	takesamples();
}

void Calibr::takesamples()
{
	float curroff; //本次采样的偏移量
	int currpos = abs(motion.axes[X].pos - beginpos); //当前行进步数
	QMutexLocker locker(&imgObj.mutex);
	baseline = getTwoLines(imgObj, curroff);
	samples.append(QPointF(currpos, curroff)); //此时存的是步数和偏移量
}

float Calibr::getTwoLines(ImageObject &imgObj, float &off)
{
	QMutexLocker locker(&imgObj.mutex);
	//先创建以偏移量为自变量的直方图
	int len = imgObj.offsLen;
	int width = imgObj.src.cols;
	float *offs = imgObj.offs;

	// otsu法分开两列光刀
	QVector<float> hist(width, 0.0f);
	int N0 = 0, N1 = 0, N = 0; //阈值 以左、以右、总体 的像素个数。初始值：T=0时
	for (int i = 0; i < len; i++){
		if (offs[i] != 0){
			hist[(int)offs[i]]++; //统计个数
			N++;
		}
	}
	N1 = N;

	int bestT = 0; //最大方差对应阈值
	float maxG = 0; //最大方差
	int valSum0 = 0, valSum1 = 0; //灰度值总和
	for (int val = 0; val < width; val++)
		valSum1 += hist[val] * val;

	for (int T = 0; T < width; T++){
		int delta = hist[T];
		N0 += delta;
		N1 -= delta;
		valSum0 += delta * T;
		valSum1 -= delta * T;
		//新平均灰度
		float u0 = valSum0 / (float)N0;
		float u1 = valSum1 / (float)N1;
		//新的两边所占比例
		float w0 = N0 / (float)N;
		float w1 = N1 / (float)N;
		//新方差
		float G = w0 * w1 * (u0 - u1) * (u0 - u1);
		if (G > maxG){
			bestT = T;
			maxG = G;
		}
	}

	//重心法取中心
	int sum1 = 0, sum2 = 0;
	for (int j = 0; j < bestT; j++){
		uchar val = hist[j];
		sum1 += val*j;
		sum2 += val;
	}
	int l1 = sum2 == 0 ? 0 : (int)((float)sum1 / sum2 + 0.5f); //重心

	sum1 = 0, sum2 = 0;
	for (int j = bestT; j < width; j++){
		uchar val = hist[j];
		sum1 += val*j;
		sum2 += val;
	}
	int l2 = sum2 == 0 ? 0 : (int)((float)sum1 / sum2 + 0.5f); //重心
	
	off = l2 - l1; //输出偏移量
	return l1; //返回baseline
}

void Calibr::onA()
{
	this->a = motion.axes[X].pos;
	qDebug() << "pos a = "<<a;
}

void Calibr::onB()
{
	this->b = motion.axes[X].pos;
	qDebug() << "pos b = " << b;
}

void Calibr::onCaliGo()
{
	beginpos = motion.axes[X].pos;
	qDebug() << "beginpos = " << beginpos;

	takesamples();
	tmr = startTimer(sampleIntvl);
	motion.go(X, Positive);
}

void Calibr::onCaliStop()
{
	killTimer(tmr);
	motion.stop(AxisAll);
}

void Calibr::onCaliCalc(float realAB, float z0, float ze, int width)
{
	float mmperstep = abs(a - b) / realAB; //每一步的毫米数
	qDebug() << "mmperstep = " << mmperstep;
	for (int i = 0; i < samples.count(); i++)
		samples[i].setX(samples[i].x() * mmperstep); //步数换成毫米
	float dist = samples[samples.count() - 1].x(); //走过的距离
	qDebug() << "dist = " << dist;

	float kreal = (ze - z0) / dist; //标定块斜面斜率
	for (int i = 0; i < samples.count(); i++)
		samples[i].setX(samples[i].x() * kreal + z0); //换成高度

	samples.prepend(QPointF(0.0f, 0.0f)); //第一个为零点

	//插值
	lut = QVector<float>(width);
	for (int i = 0; i < samples.count()-1; i++){
		QPointF &p1 = samples[i], &p2 = samples[i + 1];
		//临近的两个点之间插值
		for (float yi = ceil(p1.y()); yi < p2.y(); yi++){
			lut[(int)yi] = (yi - p1.y()) * (p2.x()-p1.x()) / (p2.y()-p1.y()) + p1.x();
		}
	}
	int count = samples.count();
	QPointF &p1 = samples[count - 1], &p2 = samples[count - 2];
	//最后的两个点之间插值
	for (float yi = ceil(p1.y()); yi < p2.y(); yi++){
		lut[(int)yi] = (yi - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()) + p1.x();
	}

	textoutput(samples, "cali_samples.txt");

	QFile file("cali_lut.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Can't write the file!";
		QMessageBox::information(nullptr, CN("错误"), CN("保存文件失败"));
	}
	QTextStream tout(&file);
	for (int i = 0; i < lut.count(); i++){
		tout << i << '\t' << lut[i] << endl;
	}

}

bool textoutput(QList<QPointF> lst, const char *str){
	QFile file(str);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Can't write the file!";
		QMessageBox::information(nullptr, CN("错误"), CN("保存文件失败"));
	}
	QTextStream tout(&file);
	for (int i = 0; i < lst.count(); i++){
		QPointF &p = lst[i];
		tout << p.x() << '\t' << p.y() << endl;
	}
	return true;
}

