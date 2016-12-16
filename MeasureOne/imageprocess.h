#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <opencv2/opencv.hpp>
using namespace cv;

#include "common.h"

#include <QString>
#include <QStringList>
#include <QElapsedTimer>
#include <QHash>

//图像处理对象，包含各阶段的图片和光刀结果
struct ImageObject{
	cv::Mat src, img, dst;
	int offsLen = 0;
	float* offs;
};

//处理方法的抽象类
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

//图像处理类，管理算法流水线
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
// 以下为自定义方法
// **************************

//极值法
class MaxValMethod :public IMethod{
	int thres = 150; //阈值，低于此值认为没有光刀
	void laserMax(Mat img, float *offs) {
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int base = 0; //某行基址
		for (int i = 0; i < h; i++){

			int iMax = 0; //每行最大值的索引值，初始值为0
			uchar max = data[base + 0]; //第一个像素的值

			//寻找每行最大值
			for (int j = 1; j < w; j++){
				if (data[base + j]>max){
					iMax = j;
					max = data[base + j];
				}
			}
			//亮度大于阈值才认为是光刀
			offs[i] = max > thres ? iMax : 0;

			base += step;
		}
	}

public:
	virtual QString key(){ return CN("极值法"); }
	virtual QString toString(){ return key() + CN(" 阈值=") + NUM_STR(thres); }

	virtual IMethod* create(QStringList args){
		int len = args.length();
		MaxValMethod m; //临时对象

		//检查参数是否符合要求
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//返回新的处理方法对象
		return new MaxValMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		laserMax(imgObj.img, imgObj.offs);
	}
};

//中点法
class MidPointMethod :public IMethod{
	int thres = 180; //认定为光刀的阈值，默认值
	void midPoint(Mat img, uchar T, float *offs){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		//uchar T = 180; //阈值

		int base = 0; //某行基址
		for (int i = 0; i < h; i++){
			//上升沿
			int left = 0;
			while (data[base + left] < T && left < w) left++;

			//没找到大于阈值的点，这行没有光刀
			if (left == w) {
				offs[i] = 0;
			}
			else{
				//下降沿
				int right = w - 1;
				while (data[base + right] < T && right > 0) right--;
				offs[i] = (left + right) >> 1; //中点，除以二
			}
			base += step;
		}
	}

public:
	virtual QString key(){ return CN("中点法"); }
	virtual QString toString(){ return key() + CN(" 阈值=") + NUM_STR(thres); }

	virtual IMethod* create(QStringList args){
		int len = args.length();
		MidPointMethod m; //临时对象

		//检查参数是否符合要求
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//返回新的处理方法对象
		return new MidPointMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		midPoint(imgObj.img, thres, imgObj.offs);
	}

};

//自适应阈值法
class AdaptiveMethod :public IMethod{
	int th_h = 100; //光刀高阈值，默认值
	int th_d = 50; //阈值浮动量 delta，默认值
	void adaptive(Mat img){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		// 		uchar th_h = 100; //光刀高阈值
		// 		uchar th_d = 50; //阈值浮动量 delta

		int base = 0; //某行基址
		for (int i = 0; i < h; i++){

			int iMax = 0; //每行最大值的索引值，初始值为0
			uchar max = data[base + 0]; //第一个像素的值

			//寻找每行最大值
			for (int j = 1; j < w; j++){
				if (data[base + j]>max){
					iMax = j;
					max = data[base + j];
				}
			}

			if (max < th_h){
				//未达到阈值，视为该行没有光刀
				for (int j = 0; j < w; j++) data[base + j] = 0;
			}
			else{
				int th_l = max - th_d; //光刀低阈值

				//找左右边缘
				int rightEdge = iMax + 1;
				while (data[base + rightEdge] > th_l) rightEdge++;

				int leftEdge = iMax - 1;
				while (data[base + leftEdge] > th_l) leftEdge--;

				//置黑其余像素
				for (int j = 0; j <= leftEdge; j++) data[base + j] = 0;
				for (int j = rightEdge; j < w; j++) data[base + j] = 0;
			}

			base += step; //跳到下一行
		}
	}

public:
	virtual QString key(){ return CN("自适应阈值法"); }
	virtual QString toString(){ return key() + CN(" 高阈值=%1 浮动阈值=%2").arg(th_h).arg(th_d); }

	virtual IMethod* create(QStringList args){
		int len = args.length();
		AdaptiveMethod m; //临时对象

		//检查参数是否符合要求
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

		//返回新的处理方法对象
		return new AdaptiveMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		adaptive(imgObj.img);
	}

};

//重心法
class CentroidMethod :public IMethod{
	void centroid(Mat img, float *offs){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int base = 0; //某行基址
		for (int i = 0; i < h; i++){
			//分子，分母
			int sum1 = 0, sum2 = 0;
			//求和
			for (int j = 0; j < w; j++){
				uchar val = data[base + j];
				sum1 += val*j;
				sum2 += val;
			}

			// **一定要记得判断分母为零的情况！！**
			int center = sum2 == 0 ? 0 : (int)((float)sum1 / sum2 + 0.5f); //重心
			offs[i] = center;

			base += step; //跳到下一行
		}

	}

public:
	virtual QString key(){ return CN("重心法"); }
	virtual QString toString(){ return key(); }

	virtual IMethod* create(QStringList args){
		int len = args.length();
		CentroidMethod m; //临时对象

		//检查参数是否符合要求

		//返回新的处理方法对象
		return new CentroidMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		centroid(imgObj.img, imgObj.offs);
	}

};

//OTSU法
class OtsuMethod :public IMethod{
	int thres = 50; //阈值不能取太小
	uchar otsu(Mat img, float Gs[] = nullptr){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int hist[256] = { 0 }; //直方图histogram
		//求直方图
		int base = 0;
		for (int i = 0; i < h; i++){
			for (int j = 0; j < w; j++){
				uchar gray = data[base + j];
				hist[gray]++;
			}
			base += step;
		}

		uchar bestT = 0; //最大方差对应阈值
		float maxG = 0; //最大方差
		int valSum0 = 0, valSum1 = 0; //灰度值总和
		for (int val = 0; val < 256; val++)
			valSum1 += hist[val] * val;
		int N0 = 0, N1 = h*w, N = h*w; //阈值 以左、以右、总体 的像素个数。初始值：T=0时

		for (uchar T = 0; T < 255; T++){
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
			if (Gs != nullptr) Gs[T] = G; //中间数据
			//阈值如果太小则忽略
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
	virtual QString key(){ return CN("OTSU法"); }
	virtual QString toString(){ return key() + CN(" 最小阈值=") + NUM_STR(thres); }

	virtual IMethod* create(QStringList args){
		int len = args.length();
		OtsuMethod m; //临时对象

		//检查参数是否符合要求
		if (len > 0){
			//if (args[0] != key()) return nullptr;

			if (len > 1){
				bool ok = false;
				m.thres = args[1].toInt(&ok);
				if (!ok || m.thres<0 || m.thres>255) return nullptr;
			}
		}

		//返回新的处理方法对象
		return new OtsuMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		uchar T = otsu(imgObj.img);
		applyThreshold(imgObj.img, T);
	}
};

#endif // IMAGEPROCESS_H
