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
	float curroff; //���β�����ƫ����
	int currpos = abs(motion.axes[X].pos - beginpos); //��ǰ�н�����
	QMutexLocker locker(&imgObj.mutex);
	baseline = getTwoLines(imgObj, curroff);
	samples.append(QPointF(currpos, curroff)); //��ʱ����ǲ�����ƫ����
}

float Calibr::getTwoLines(ImageObject &imgObj, float &off)
{
	QMutexLocker locker(&imgObj.mutex);
	//�ȴ�����ƫ����Ϊ�Ա�����ֱ��ͼ
	int len = imgObj.offsLen;
	int width = imgObj.src.cols;
	float *offs = imgObj.offs;

	// otsu���ֿ����й⵶
	QVector<float> hist(width, 0.0f);
	int N0 = 0, N1 = 0, N = 0; //��ֵ �������ҡ����� �����ظ�������ʼֵ��T=0ʱ
	for (int i = 0; i < len; i++){
		if (offs[i] != 0){
			hist[(int)offs[i]]++; //ͳ�Ƹ���
			N++;
		}
	}
	N1 = N;

	int bestT = 0; //��󷽲��Ӧ��ֵ
	float maxG = 0; //��󷽲�
	int valSum0 = 0, valSum1 = 0; //�Ҷ�ֵ�ܺ�
	for (int val = 0; val < width; val++)
		valSum1 += hist[val] * val;

	for (int T = 0; T < width; T++){
		int delta = hist[T];
		N0 += delta;
		N1 -= delta;
		valSum0 += delta * T;
		valSum1 -= delta * T;
		//��ƽ���Ҷ�
		float u0 = valSum0 / (float)N0;
		float u1 = valSum1 / (float)N1;
		//�µ�������ռ����
		float w0 = N0 / (float)N;
		float w1 = N1 / (float)N;
		//�·���
		float G = w0 * w1 * (u0 - u1) * (u0 - u1);
		if (G > maxG){
			bestT = T;
			maxG = G;
		}
	}

	//���ķ�ȡ����
	int sum1 = 0, sum2 = 0;
	for (int j = 0; j < bestT; j++){
		uchar val = hist[j];
		sum1 += val*j;
		sum2 += val;
	}
	int l1 = sum2 == 0 ? 0 : (int)((float)sum1 / sum2 + 0.5f); //����

	sum1 = 0, sum2 = 0;
	for (int j = bestT; j < width; j++){
		uchar val = hist[j];
		sum1 += val*j;
		sum2 += val;
	}
	int l2 = sum2 == 0 ? 0 : (int)((float)sum1 / sum2 + 0.5f); //����
	
	off = l2 - l1; //���ƫ����
	return l1; //����baseline
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
	float mmperstep = abs(a - b) / realAB; //ÿһ���ĺ�����
	qDebug() << "mmperstep = " << mmperstep;
	for (int i = 0; i < samples.count(); i++)
		samples[i].setX(samples[i].x() * mmperstep); //�������ɺ���
	float dist = samples[samples.count() - 1].x(); //�߹��ľ���
	qDebug() << "dist = " << dist;

	float kreal = (ze - z0) / dist; //�궨��б��б��
	for (int i = 0; i < samples.count(); i++)
		samples[i].setX(samples[i].x() * kreal + z0); //���ɸ߶�

	samples.prepend(QPointF(0.0f, 0.0f)); //��һ��Ϊ���

	//��ֵ
	lut = QVector<float>(width);
	for (int i = 0; i < samples.count()-1; i++){
		QPointF &p1 = samples[i], &p2 = samples[i + 1];
		//�ٽ���������֮���ֵ
		for (float yi = ceil(p1.y()); yi < p2.y(); yi++){
			lut[(int)yi] = (yi - p1.y()) * (p2.x()-p1.x()) / (p2.y()-p1.y()) + p1.x();
		}
	}
	int count = samples.count();
	QPointF &p1 = samples[count - 1], &p2 = samples[count - 2];
	//����������֮���ֵ
	for (float yi = ceil(p1.y()); yi < p2.y(); yi++){
		lut[(int)yi] = (yi - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()) + p1.x();
	}

	textoutput(samples, "cali_samples.txt");

	QFile file("cali_lut.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Can't write the file!";
		QMessageBox::information(nullptr, CN("����"), CN("�����ļ�ʧ��"));
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
		QMessageBox::information(nullptr, CN("����"), CN("�����ļ�ʧ��"));
	}
	QTextStream tout(&file);
	for (int i = 0; i < lst.count(); i++){
		QPointF &p = lst[i];
		tout << p.x() << '\t' << p.y() << endl;
	}
	return true;
}

