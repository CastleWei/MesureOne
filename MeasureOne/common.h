//自定义的公用内容

#ifndef COMMON_H
#define COMMON_H

#include <QString>

////中文字符串
#define CN(str) QString::fromLocal8Bit(str)
////数字转字符串
#define NUM_STR(str) QString::number(str)

#endif // COMMON_H

//测试painter.drawImage()和setPixmap()的效率

/*
	单位ms		1张图	4张图
RGB		pix		12		48
		draw	31		126
GRAY	pix		39		158
		draw	17		70

结论：	painter.drwaImage()灰度图快；
		setPixmap() RGB快；
		相差三倍
*/

//void MeasureOne::OnTest()
//{
//	cv::Mat img = cv::imread("wgs.bmp");
//	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
//	QImage qimg(img.data, img.cols, img.rows, img.cols * 3, QImage::Format_RGB888);
//
//	cv::Mat img2 = cv::imread("1.jpg");
//	cv::cvtColor(img2, img2, cv::COLOR_BGR2RGB);
//	QImage qimg2(img2.data, img2.cols, img2.rows, img2.cols * 3, QImage::Format_RGB888);
//
//	//cv::Mat img = cv::imread("wgs.bmp");
//	//cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
//	//QImage qimg(img.data, img.cols, img.rows, img.cols, QImage::Format_Grayscale8);
//
//	//cv::Mat img2 = cv::imread("1.jpg");
//	//cv::cvtColor(img2, img2, cv::COLOR_BGR2GRAY);
//	//QImage qimg2(img2.data, img2.cols, img2.rows, img2.cols, QImage::Format_Grayscale8);
//
//	this->setMinimumSize(qimg.width()/* + mImg.width()*/, qimg.height());
//
//	QElapsedTimer tmr;
//
//	//tmr.start();
//	//QPainter painter(this);
//	//painter.drawImage(0, 0, qimg2);
//	//qint64 dt1 = tmr.elapsed();
//	ui.lblShow->pix = true;
//
//	tmr.start();
//	// 	ui.lblShow->setPixmap(QPixmap::fromImage(qimg));
//	// 	ui.lblShow->setPixmap(QPixmap::fromImage(qimg2));
//	// 	ui.lblShow->setPixmap(QPixmap::fromImage(qimg));
//	ui.lblShow->setPixmap(QPixmap::fromImage(qimg2));
//	qint64 dt2 = tmr.elapsed();

//	statusBar()->showMessage("pix dt=" + NUM_STR(dt2));
//}

//void ShowWidget::paintEvent(QPaintEvent * event)
//{
//	if (pix){
//		QLabel::paintEvent(event);
//		return;
//	}
//
//	cv::Mat img = cv::imread("wgs.bmp");
//	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
//	QImage qimg(img.data, img.cols, img.rows, img.cols * 3, QImage::Format_RGB888);
//
//	cv::Mat img2 = cv::imread("1.jpg");
//	cv::cvtColor(img2, img2, cv::COLOR_BGR2RGB);
//	QImage qimg2(img2.data, img2.cols, img2.rows, img2.cols * 3, QImage::Format_RGB888);
//
//	this->setMinimumSize(qimg.width()/* + mImg.width()*/, qimg.height());
//
//	QElapsedTimer tmr;
//
//	tmr.start();
//	QPainter painter(this);
//	// 	painter.drawImage(0, 0, qimg);
//	// 	painter.drawImage(0, 0, qimg2);
//	// 	painter.drawImage(0, 0, qimg);
//	painter.drawImage(0, 0, qimg2);
//	qint64 dt1 = tmr.elapsed();
//
//	str.sprintf("painter event dt = %d", dt1);
//
//}

