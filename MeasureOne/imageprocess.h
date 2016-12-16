#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <opencv2/opencv.hpp>
using namespace cv;

#include "common.h"

#include <QString>
#include <QStringList>
#include <QElapsedTimer>
#include <QHash>

//ͼ������󣬰������׶ε�ͼƬ�͹⵶���
struct ImageObject{
	cv::Mat src, img, dst;
	int offsLen = 0;
	float* offs;
};

//�������ĳ�����
class IMethod{
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
	virtual IMethod* create(QStringList cmd) = 0;

protected:
	virtual void call(ImageObject &img) = 0;
};

//ͼ�����࣬�����㷨��ˮ��
class ImageProcess
{
public:

	ImageProcess();
	~ImageProcess();

	void process(ImageObject &imgObj);
	bool parse(QString code);
	QString toString(bool isRunning);

private:
	QHash<QString, IMethod*> methods;
	QList<IMethod*> pipeline;
};

// **************************
// ����Ϊ�Զ��巽��
// **************************

//��ֵ��
class MaxValMethod :public IMethod{
	int thres = 150; //��ֵ�����ڴ�ֵ��Ϊû�й⵶
	void laserMax(Mat img, float *offs) {
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
	virtual QString key(){ return CN("��ֵ��"); }
	virtual QString toString(){ return key() + CN(" ��ֵ=") + NUM_STR(thres); }

	virtual IMethod* create(QStringList args){
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

//�е㷨
class MidPointMethod :public IMethod{
	int thres = 180; //�϶�Ϊ�⵶����ֵ��Ĭ��ֵ
	void midPoint(Mat img, uchar T, float *offs){
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
	virtual QString key(){ return CN("�е㷨"); }
	virtual QString toString(){ return key() + CN(" ��ֵ=") + NUM_STR(thres); }

	virtual IMethod* create(QStringList args){
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

//����Ӧ��ֵ��
class AdaptiveMethod :public IMethod{
	int th_h = 100; //�⵶����ֵ��Ĭ��ֵ
	int th_d = 50; //��ֵ������ delta��Ĭ��ֵ
	void adaptive(Mat img){
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
	virtual QString key(){ return CN("����Ӧ��ֵ��"); }
	virtual QString toString(){ return key() + CN(" ����ֵ=%1 ������ֵ=%2").arg(th_h).arg(th_d); }

	virtual IMethod* create(QStringList args){
		int len = args.length();
		AdaptiveMethod m; //��ʱ����

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
		return new AdaptiveMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		adaptive(imgObj.img);
	}

};

//���ķ�
class CentroidMethod :public IMethod{
	void centroid(Mat img, float *offs){
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
	virtual QString key(){ return CN("���ķ�"); }
	virtual QString toString(){ return key(); }

	virtual IMethod* create(QStringList args){
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

//OTSU��
class OtsuMethod :public IMethod{
	int thres = 50; //��ֵ����ȡ̫С
	uchar otsu(Mat img, float Gs[] = nullptr){
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
	void applyThreshold(Mat img, uchar T){
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
	virtual QString key(){ return CN("OTSU��"); }
	virtual QString toString(){ return key() + CN(" ��С��ֵ=") + NUM_STR(thres); }

	virtual IMethod* create(QStringList args){
		int len = args.length();
		OtsuMethod m; //��ʱ����

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
		return new OtsuMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		uchar T = otsu(imgObj.img);
		applyThreshold(imgObj.img, T);
	}
};

#endif // IMAGEPROCESS_H
