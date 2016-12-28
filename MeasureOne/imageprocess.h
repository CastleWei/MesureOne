#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <opencv2/opencv.hpp>

#include "common.h"

#include <QString>
#include <QStringList>
#include <QElapsedTimer>
#include <QHash>

//图像处理对象，包含各阶段的图片和光刀结果
struct ImageObject{
	QMutex mutex;
	cv::Mat src, img, dst;
	int threshold = 128; //全图阈值
	int offsLen = 0;
	float *offs = nullptr; //todo:改成vector

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

//处理方法的抽象类
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

//图像处理类，管理算法流水线
class ImageProcess
{
public:
	QMutex mutex; //防止process和parse冲突

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
// 以下为自定义方法
// **************************

//极值法取光刀
class MaxValMethod :public AbstractMethod{
	int thres = 150; //阈值，低于此值认为没有光刀
	void laserMax(cv::Mat img, float *offs) {
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
	virtual QString key(){ return CN("极值法取光刀"); }
	virtual QString toString(){ return key() + CN(" 阈值=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
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

//中点法取光刀
class MidPointMethod :public AbstractMethod{
	int thres = 180; //认定为光刀的阈值，默认值
	void midPoint(cv::Mat img, uchar T, float *offs){
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
	virtual QString key(){ return CN("中点法取光刀"); }
	virtual QString toString(){ return key() + CN(" 阈值=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
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

//重心法取光刀
class CentroidMethod :public AbstractMethod{
	void centroid(cv::Mat img, float *offs){
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
	virtual QString key(){ return CN("重心法取光刀"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
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

//边缘检测法取光刀（高旭）
class GxEdgeDivideMethod :public AbstractMethod{
	int thres = 150; //认定为光刀的高阈值，默认值
	void EdgeDivide(uchar *p, int w, int h, float *offs)
		//边缘检测，找出230以上的点，向两边检测二阶导数变化的点
	{
		//QElapsedTimer sec;
		//sec.start();

		int base = 0;
		for (int i = 0; i < h; i++)
		{
			// 		int *diff = new int[w];		//梯度数组
			// 		for (int j = 0; j < w - 1; j++)
			// 		{
			// 			diff[j] = p[base + j + 1] - p[base + j];
			// 		}
			// 		diff[w - 1] = 0;
			int left = 0;
			int right = w - 1;
			int j = 0;
			for (j = 0; j<w; j++)			//首先取230以上的点为光刀中心点
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
			for (j = left; j > 1; j--)				//向两边寻找梯度较大的点
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
			if (left != 0 || right != w - 1)			//有光刀则重心法
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

				//计入偏移量数组
				offs[i] = middle;
			}
			else
			{
				offs[i] = 0;

				//未找到则抹黑
				//for (j = 0; j < w; j++)
				//{
				//	p[base + j] = 0;
				//}
			}
			// 		for (j = 0; j < w;j++)				//显示提取区域
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
	virtual QString key(){ return CN("边缘检测取光刀"); }
	virtual QString toString(){ return key() + CN(" 阈值=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		GxEdgeDivideMethod m; //临时对象

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
		return new GxEdgeDivideMethod(m);
	}

	virtual void call(ImageObject &imgObj){
		cv::Mat &img = imgObj.img;
		EdgeDivide(img.data,img.cols,img.rows,imgObj.offs);
	}

};

//OTSU法取阈值
class OtsuThres :public AbstractMethod{
	int thres = 50; //OTSU取到的阈值不能太小
	uchar otsu(cv::Mat img, float Gs[] = nullptr){
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

public:
	virtual QString key(){ return CN("OTSU取阈值"); }
	virtual QString toString(){ return key() + CN(" 最小阈值=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		OtsuThres m; //临时对象

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
		return new OtsuThres(m);
	}

	virtual void call(ImageObject &imgObj){
		imgObj.threshold = otsu(imgObj.img);
	}
};

//按行OTSU分割
class OtsuByRowSegmn :public AbstractMethod{
	int thres = 50; //OTSU取到的阈值不能太小
	//按行取OTSU，阈值以下置黑
	void otsuByRow(cv::Mat img){
		int h = img.rows;
		int w = img.cols;
		int step = img.step1();
		uchar *data = img.data;

		int base = 0; //某行基址
		for (int i = 0; i < h; i++){
			int hist[256] = { 0 }; //直方图histogram
			//求一行的直方图
			for (int j = 0; j < w; j++){
				uchar gray = data[base + j];
				hist[gray]++;
			}

			uchar bestT = 0; //最大方差对应阈值
			float maxG = 0; //最大方差
			int valSum0 = 0, valSum1 = 0; //灰度值总和
			for (int val = 0; val < 256; val++)
				valSum1 += hist[val] * val;
			int N0 = 0, N1 = w, N = w; //阈值 以左、以右、总体 的像素个数。初始值：T=0时

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
				//阈值如果太小则忽略
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
			//找最大灰度，如果太小，说明没有光刀
			int max = 255;
			while (hist[max] == 0) max--;
			if (max < 80) bestT = 255;

			//将该行中小于阈值的部分去除
			for (int j = 0; j < w; j++){
				if (data[base + j] < bestT)
					data[base + j] = 0;
			}

			//**************************
			base += step;
		}

	}

public:
	virtual QString key(){ return CN("按行OTSU分割"); }
	virtual QString toString(){ return key() + CN(" 最小阈值=") + NUM_STR(thres); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		OtsuByRowSegmn m; //临时对象

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
		return new OtsuByRowSegmn(m);
	}

	virtual void call(ImageObject &imgObj){
		otsuByRow(imgObj.img);
	}
};

//自适应阈值分割
class AdaptiveSegmn :public AbstractMethod{
	int th_h = 100; //光刀高阈值，默认值
	int th_d = 50; //阈值浮动量 delta，默认值
	void adaptive(cv::Mat img){
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
	virtual QString key(){ return CN("自适应阈值分割"); }
	virtual QString toString(){ return key() + CN(" 高阈值=%1 浮动阈值=%2").arg(th_h).arg(th_d); }

	virtual AbstractMethod* create(QStringList args){
		int len = args.length();
		AdaptiveSegmn m; //临时对象

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
		return new AdaptiveSegmn(m);
	}

	virtual void call(ImageObject &imgObj){
		adaptive(imgObj.img);
	}

};

//阈值分割（阈值以下置黑）
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
	virtual QString key(){ return CN("阈值分割"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//不用检查参数
		//返回新的处理方法对象
		return new ThresholdSegmn();
	}

	virtual void call(ImageObject &imgObj){
		applyThreshold(imgObj.img, imgObj.threshold);
	}
};

//画光刀，出结果
class MixLaserResult :public AbstractMethod{
	//把光刀结果用红线画在原图上
	void mix(cv::Mat &src, float *offs){
		int h = src.rows;
		int w = src.cols;
		int step = src.step1();
		uchar *data = src.data;

		int base = 0; //某行基址
		for (int i = 0; i < h; i++){
			if (offs[i]>0){
				//该行第i个像素的首地址
				uchar *bgr = data + base + (int)round(offs[i]) * 3;
				//设为红色
				bgr[0] = 255;
				bgr[1] = 0;
				bgr[2] = 0;
			}
			base += step;
		}
	}

public:
	virtual QString key(){ return CN("画光刀"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//不用检查参数
		//返回新的处理方法对象
		return new MixLaserResult();
	}

	virtual void call(ImageObject &imgObj){
		cv::cvtColor(imgObj.src, imgObj.dst, cv::COLOR_BGR2RGB);
		mix(imgObj.dst, imgObj.offs);
	}
};

//显示原图，出结果
class SrcImgResult :public AbstractMethod{

public:
	virtual QString key(){ return CN("显示原图"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//不用检查参数
		//返回新的处理方法对象
		return new SrcImgResult();
	}

	virtual void call(ImageObject &imgObj){
		//imgObj.dst = imgObj.src; //仅指向原图，不复制 //=>要转换颜色，否则发蓝
		cv::cvtColor(imgObj.src, imgObj.dst, cv::COLOR_BGR2RGB);
	}
};

//显示灰度图，出结果
class GrayImgResult :public AbstractMethod{

public:
	virtual QString key(){ return CN("显示灰度图"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//不用检查参数
		//返回新的处理方法对象
		return new GrayImgResult();
	}

	virtual void call(ImageObject &imgObj){
		imgObj.dst = imgObj.img; //仅指向灰度图，不复制
		//TODO: bug:一开始就用"显示灰度图\n 按需放大"，就会挂掉，不知道为什么
	}
};

//按需放大，出结果
class ZoomResult :public AbstractMethod{

public:
	virtual QString key(){ return CN("按需放大"); }
	virtual QString toString(){ return key(); }

	virtual AbstractMethod* create(QStringList args){
		//不用检查参数
		//返回新的处理方法对象
		return new ZoomResult();
	}

	virtual void call(ImageObject &imgObj){
		if (imgObj.dst.cols < 1000)
			resize(imgObj.dst, imgObj.dst, cv::Size(), 2, 2, cv::INTER_NEAREST);
	}
};


#endif // IMAGEPROCESS_H
