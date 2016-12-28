#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <opencv2/opencv.hpp>

#include "common.h"

#include <QString>
#include <QStringList>
#include <QElapsedTimer>
#include <QHash>

//ͼ������󣬰������׶ε�ͼƬ�͹⵶���
struct ImageObject{
	QMutex mutex;
	cv::Mat src, img, dst;
	int threshold = 128; //ȫͼ��ֵ
	int offsLen = 0;
	float *offs = nullptr; //todo:�ĳ�vector

	~ImageObject(){
		if (offs != nullptr) delete[] offs;
	}

	void newOffs(int offsLen){
		this->offsLen = offsLen;
		if (offs != nullptr) delete[] offs;
		this->offs = new float[offsLen]();
	}

	void clear(){
		src = cv::Mat();
		img = cv::Mat();
		dst = cv::Mat();
		offsLen = 0;
		if (offs != nullptr) delete[] offs;
		offs = nullptr;
	}
};

//�������ĳ�����
class AbstractMethod{
	QElapsedTimer tmr;

public:
	void process(ImageObject &img){
		tmr.start();
		call(img);
		dt = (int)tmr.elapsed();
	}

	int dt = 0;
	virtual QString key() = 0;
	virtual QString toString() = 0;
	virtual AbstractMethod* create(QStringList cmd) = 0;

protected:
	virtual void call(ImageObject &img) = 0;
};

//ͼ�����࣬�����㷨��ˮ��
class ImageProcess
{
public:
	QMutex mutex; //��ֹprocess��parse��ͻ

	ImageProcess();
	~ImageProcess();

	int process(ImageObject &imgObj);
	bool parse(QString code);
	QString toString(bool isRunning);

private:
	QHash<QString, AbstractMethod*> methods;
	QList<AbstractMethod*> pipeline;
};

// **************************
// ����Ϊ�Զ��巽��
// **************************

//��ֵ��ȡ�⵶
class MaxValMethod :public AbstractMethod{
	int thres = 150; //��ֵ�����ڴ�ֵ��Ϊû�й⵶
	void laserMax(cv::Mat img, float *offs) {
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int base = 0; //ĳ�л�ַ
		for (int i = 0; i < h; i++){

			int iMax = 0; //ÿ�����ֵ������ֵ����ʼֵΪ0
			uchar max = data[base + 0]; //��һ�����ص�ֵ

			//Ѱ��ÿ�����ֵ
			for (int j = 1; j < w; j++){
				if (data[base + j]>max){
					iMax = j;
					max = data[base + j];
				}
			}
			//���ȴ�����ֵ����Ϊ�ǹ⵶
			offs[i] = max > thres ? iMax : 0;

			base += step;
		}
	}

public:
	virtual QString key(){ return CN("��ֵ��ȡ�⵶"); }
	virtual QString toString(){ return key() + CN(" ��ֵ=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		MaxValMethod m; //��ʱ����

		//�������Ƿ����Ҫ��
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//�����µĴ���������
		return new MaxValMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		laserMax(imgObj.img, imgObj.offs);
	}
};

//�е㷨ȡ�⵶
class MidPointMethod :public AbstractMethod{
	int thres = 180; //�϶�Ϊ�⵶����ֵ��Ĭ��ֵ
	void midPoint(cv::Mat img, uchar T, float *offs){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		//uchar T = 180; //��ֵ

		int base = 0; //ĳ�л�ַ
		for (int i = 0; i < h; i++){
			//������
			int left = 0;
			while (data[base + left] < T && left < w) left++;

			//û�ҵ�������ֵ�ĵ㣬����û�й⵶
			if (left == w) {
				offs[i] = 0;
			}
			else{
				//�½���
				int right = w - 1;
				while (data[base + right] < T && right > 0) right--;
				offs[i] = (left + right) >> 1; //�е㣬���Զ�
			}
			base += step;
		}
	}

public:
	virtual QString key(){ return CN("�е㷨ȡ�⵶"); }
	virtual QString toString(){ return key() + CN(" ��ֵ=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		MidPointMethod m; //��ʱ����

		//�������Ƿ����Ҫ��
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//�����µĴ���������
		return new MidPointMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		midPoint(imgObj.img, thres, imgObj.offs);
	}

};

//���ķ�ȡ�⵶
class CentroidMethod :public AbstractMethod{
	void centroid(cv::Mat img, float *offs){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int base = 0; //ĳ�л�ַ
		for (int i = 0; i < h; i++){
			//���ӣ���ĸ
			int sum1 = 0, sum2 = 0;
			//���
			for (int j = 0; j < w; j++){
				uchar val = data[base + j];
				sum1 += val*j;
				sum2 += val;
			}

			// **һ��Ҫ�ǵ��жϷ�ĸΪ����������**
			int center = sum2 == 0 ? 0 : (int)((float)sum1 / sum2 + 0.5f); //����
			offs[i] = center;

			base += step; //������һ��
		}

	}

public:
	virtual QString key(){ return CN("���ķ�ȡ�⵶"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		CentroidMethod m; //��ʱ����

		//�������Ƿ����Ҫ��

		//�����µĴ���������
		return new CentroidMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		centroid(imgObj.img, imgObj.offs);
	}

};

//��Ե��ⷨȡ�⵶������
class GxEdgeDivideMethod :public AbstractMethod{
	int thres = 150; //�϶�Ϊ�⵶�ĸ���ֵ��Ĭ��ֵ
	void EdgeDivide(uchar *p, int w, int h, float *offs)
		//��Ե��⣬�ҳ�230���ϵĵ㣬�����߼����׵����仯�ĵ�
	{
		//QElapsedTimer sec;
		//sec.start();

		int base = 0;
		for (int i = 0; i < h; i++)
		{
			// 		int *diff = new int[w];		//�ݶ�����
			// 		for (int j = 0; j < w - 1; j++)
			// 		{
			// 			diff[j] = p[base + j + 1] - p[base + j];
			// 		}
			// 		diff[w - 1] = 0;
			int left = 0;
			int right = w - 1;
			int j = 0;
			for (j = 0; j<w; j++)			//����ȡ230���ϵĵ�Ϊ�⵶���ĵ�
			{
				if (p[base + j]>thres)
				{
					left = j - 1;
					break;
				}
			}
			for (j; j < w; j++)
			{
				if (p[base + j] < thres)
				{
					right = j;
					break;
				}
			}
			for (j = left; j > 1; j--)				//������Ѱ���ݶȽϴ�ĵ�
			{
				if (p[base + j + 1] + p[base + j - 1] - 2 * p[base + j] >= 0)
				{
					left = j;
					break;
				}
			}
			for (j = right; j < w - 1; j++)
			{
				if (p[base + j + 1] + p[base + j - 1] - 2 * p[base + j] <= 0)
				{
					right = j;
					break;
				}
			}
			if (left != 0 || right != w - 1)			//�й⵶�����ķ�
			{
				int count = 0;
				int sum = 0;
				int middle = 0;
				for (j = left; j <= right; j++)
				{
					sum += j*p[base + j];
					count += p[base + j];
				}
				middle = (sum + 0.5) / count;
				//for (j = 0; j < w; j++)
				//{
				//	p[base + j] = 0;
				//}
				//p[base + middle] = 255;

				//����ƫ��������
				offs[i] = middle;
			}
			else
			{
				offs[i] = 0;

				//δ�ҵ���Ĩ��
				//for (j = 0; j < w; j++)
				//{
				//	p[base + j] = 0;
				//}
			}
			// 		for (j = 0; j < w;j++)				//��ʾ��ȡ����
			// 		{
			// 			p[base + j] =0;
			// 		}
			// 		if (left!=0||right!=w-1)
			// 		{
			// 			for (j = left; j <= right;j++)
			// 			{
			// 				p[base + j] = 255;
			// 			}
			// 		}
			base += w;
		}
		//qDebug("%d", sec.elapsed());
	}

public:
	virtual QString key(){ return CN("��Ե���ȡ�⵶"); }
	virtual QString toString(){ return key() + CN(" ��ֵ=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		GxEdgeDivideMethod m; //��ʱ����

		//�������Ƿ����Ҫ��
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//�����µĴ���������
		return new GxEdgeDivideMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		cv::Mat &img = imgObj.img;
		EdgeDivide(img.data,img.cols,img.rows,imgObj.offs);
	}

};

//OTSU��ȡ��ֵ
class OtsuThres :public AbstractMethod{
	int thres = 50; //OTSUȡ������ֵ����̫С
	uchar otsu(cv::Mat img, float Gs[] = nullptr){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int hist[256] = { 0 }; //ֱ��ͼhistogram
		//��ֱ��ͼ
		int base = 0;
		for (int i = 0; i < h; i++){
			for (int j = 0; j < w; j++){
				uchar gray = data[base + j];
				hist[gray]++;
			}
			base += step;
		}

		uchar bestT = 0; //��󷽲��Ӧ��ֵ
		float maxG = 0; //��󷽲�
		int valSum0 = 0, valSum1 = 0; //�Ҷ�ֵ�ܺ�
		for (int val = 0; val < 256; val++)
			valSum1 += hist[val] * val;
		int N0 = 0, N1 = h*w, N = h*w; //��ֵ �������ҡ����� �����ظ�������ʼֵ��T=0ʱ

		for (uchar T = 0; T < 255; T++){
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
			if (Gs != nullptr) Gs[T] = G; //�м�����
			//��ֵ���̫С�����
			if (T < 50) continue;
			if (G > maxG){
				bestT = T;
				maxG = G;
			}
		}

		return bestT;
	}

public:
	virtual QString key(){ return CN("OTSUȡ��ֵ"); }
	virtual QString toString(){ return key() + CN(" ��С��ֵ=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		OtsuThres m; //��ʱ����

		//�������Ƿ����Ҫ��
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//�����µĴ���������
		return new OtsuThres(m);
	}

	virtual void call(ImageObject &imgObj){
		imgObj.threshold = otsu(imgObj.img);
	}
};

//����OTSU�ָ�
class OtsuByRowSegmn :public AbstractMethod{
	int thres = 50; //OTSUȡ������ֵ����̫С
	//����ȡOTSU����ֵ�����ú�
	void otsuByRow(cv::Mat img){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int base = 0; //ĳ�л�ַ
		for (int i = 0; i < h; i++){
			int hist[256] = { 0 }; //ֱ��ͼhistogram
			//��һ�е�ֱ��ͼ
			for (int j = 0; j < w; j++){
				uchar gray = data[base + j];
				hist[gray]++;
			}

			uchar bestT = 0; //��󷽲��Ӧ��ֵ
			float maxG = 0; //��󷽲�
			int valSum0 = 0, valSum1 = 0; //�Ҷ�ֵ�ܺ�
			for (int val = 0; val < 256; val++)
				valSum1 += hist[val] * val;
			int N0 = 0, N1 = w, N = w; //��ֵ �������ҡ����� �����ظ�������ʼֵ��T=0ʱ

			for (uchar T = 0; T < 255; T++){
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
				//��ֵ���̫С�����
				if (T < thres) continue;
				if (G >= maxG){
					bestT = T;
					maxG = G;
				}
			}
			//if (bestT == 50){
			//	printf("%d", bestT);
			//	bestT = 255;
			//}
			//�����Ҷȣ����̫С��˵��û�й⵶
			int max = 255;
			while (hist[max] == 0) max--;
			if (max < 80) bestT = 255;

			//��������С����ֵ�Ĳ���ȥ��
			for (int j = 0; j < w; j++){
				if (data[base + j] < bestT)
					data[base + j] = 0;
			}

			//**************************
			base += step;
		}

	}

public:
	virtual QString key(){ return CN("����OTSU�ָ�"); }
	virtual QString toString(){ return key() + CN(" ��С��ֵ=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		OtsuByRowSegmn m; //��ʱ����

		//�������Ƿ����Ҫ��
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//�����µĴ���������
		return new OtsuByRowSegmn(m);
	}

	virtual void call(ImageObject &imgObj){
		otsuByRow(imgObj.img);
	}
};

//����Ӧ��ֵ�ָ�
class AdaptiveSegmn :public AbstractMethod{
	int th_h = 100; //�⵶����ֵ��Ĭ��ֵ
	int th_d = 50; //��ֵ������ delta��Ĭ��ֵ
	void adaptive(cv::Mat img){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		// 		uchar th_h = 100; //�⵶����ֵ
		// 		uchar th_d = 50; //��ֵ������ delta

		int base = 0; //ĳ�л�ַ
		for (int i = 0; i < h; i++){

			int iMax = 0; //ÿ�����ֵ������ֵ����ʼֵΪ0
			uchar max = data[base + 0]; //��һ�����ص�ֵ

			//Ѱ��ÿ�����ֵ
			for (int j = 1; j < w; j++){
				if (data[base + j]>max){
					iMax = j;
					max = data[base + j];
				}
			}

			if (max < th_h){
				//δ�ﵽ��ֵ����Ϊ����û�й⵶
				for (int j = 0; j < w; j++) data[base + j] = 0;
			}
			else{
				int th_l = max - th_d; //�⵶����ֵ

				//�����ұ�Ե
				int rightEdge = iMax + 1;
				while (data[base + rightEdge] > th_l) rightEdge++;

				int leftEdge = iMax - 1;
				while (data[base + leftEdge] > th_l) leftEdge--;

				//�ú���������
				for (int j = 0; j <= leftEdge; j++) data[base + j] = 0;
				for (int j = rightEdge; j < w; j++) data[base + j] = 0;
			}

			base += step; //������һ��
		}
	}

public:
	virtual QString key(){ return CN("����Ӧ��ֵ�ָ�"); }
	virtual QString toString(){ return key() + CN(" ����ֵ=%1 ������ֵ=%2").arg(th_h).arg(th_d); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		AdaptiveSegmn m; //��ʱ����

		//�������Ƿ����Ҫ��
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.th_h = args[1].toInt(&ok);
				if (!ok || m.th_h<0 || m.th_h>255) return nullptr;

				if (len > 2){
					bool ok = false;
					m.th_d = args[2].toInt(&ok);

					int th_l = m.th_h - m.th_d;
					if (!ok || th_l<0 || th_l>255) return nullptr;
				}
			}
		}

		//�����µĴ���������
		return new AdaptiveSegmn(m);
	}

	virtual void call(ImageObject &imgObj){
		adaptive(imgObj.img);
	}

};

//��ֵ�ָ��ֵ�����úڣ�
class ThresholdSegmn :public AbstractMethod{
	void applyThreshold(cv::Mat img, uchar T){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int base = 0;
		for (int i = 0; i < h; i++){
			for (int j = 0; j < w; j++){
				if (data[base + j] < T) data[base + j] = 0;
			}
			base += step;
		}
	}

public:
	virtual QString key(){ return CN("��ֵ�ָ�"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//���ü�����
		//�����µĴ���������
		return new ThresholdSegmn();
	}

	virtual void call(ImageObject &imgObj){
		applyThreshold(imgObj.img, imgObj.threshold);
	}
};

//���⵶�������
class MixLaserResult :public AbstractMethod{
	//�ѹ⵶����ú��߻���ԭͼ��
	void mix(cv::Mat &src, float *offs){
		int h = src.rows;
		int w = src.cols;
		int step = src.step1();
		uchar *data = src.data;

		int base = 0; //ĳ�л�ַ
		for (int i = 0; i < h; i++){
			if (offs[i]>0){
				//���е�i�����ص��׵�ַ
				uchar *bgr = data + base + (int)round(offs[i]) * 3;
				//��Ϊ��ɫ
				bgr[0] = 255;
				bgr[1] = 0;
				bgr[2] = 0;
			}
			base += step;
		}
	}

public:
	virtual QString key(){ return CN("���⵶"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//���ü�����
		//�����µĴ���������
		return new MixLaserResult();
	}

	virtual void call(ImageObject &imgObj){
		cv::cvtColor(imgObj.src, imgObj.dst, cv::COLOR_BGR2RGB);
		mix(imgObj.dst, imgObj.offs);
	}
};

//��ʾԭͼ�������
class SrcImgResult :public AbstractMethod{

public:
	virtual QString key(){ return CN("��ʾԭͼ"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//���ü�����
		//�����µĴ���������
		return new SrcImgResult();
	}

	virtual void call(ImageObject &imgObj){
		//imgObj.dst = imgObj.src; //��ָ��ԭͼ�������� //=>Ҫת����ɫ��������
		cv::cvtColor(imgObj.src, imgObj.dst, cv::COLOR_BGR2RGB);
	}
};

//��ʾ�Ҷ�ͼ�������
class GrayImgResult :public AbstractMethod{

public:
	virtual QString key(){ return CN("��ʾ�Ҷ�ͼ"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//���ü�����
		//�����µĴ���������
		return new GrayImgResult();
	}

	virtual void call(ImageObject &imgObj){
		imgObj.dst = imgObj.img; //��ָ��Ҷ�ͼ��������
		//TODO: bug:һ��ʼ����"��ʾ�Ҷ�ͼ\n ����Ŵ�"���ͻ�ҵ�����֪��Ϊʲô
	}
};

//����Ŵ󣬳����
class ZoomResult :public AbstractMethod{

public:
	virtual QString key(){ return CN("����Ŵ�"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//���ü�����
		//�����µĴ���������
		return new ZoomResult();
	}

	virtual void call(ImageObject &imgObj){
		if (imgObj.dst.cols < 1000)
			resize(imgObj.dst, imgObj.dst, cv::Size(), 2, 2, cv::INTER_NEAREST);
	}
};


#endif // IMAGEPROCESS_H
