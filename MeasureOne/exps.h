//*******************************
//本文件记录本项目中所有的实测经验
//*******************************


//***<<<  0. 格式  >>>***

//内容

//***<<< 以上  0. 格式  >>>***



//***<<<  5. cvtColor()耗时  >>>***

//结论：1280*960 的图片，不同mat间转换2ms，同一个mat转换9ms

//cv::Mat mat, mat1, mat2;
//mat = cv::imread("wgs.bmp");
//mat1 = cv::imread("1.jpg");
//mat2 = cv::imread("wgs.bmp");
//
//QElapsedTimer tmr;
//tmr.start();
//cvtColor(mat1, mat, cv::COLOR_BGR2RGB);
//cvtColor(mat2, mat, cv::COLOR_BGR2RGB);
//cvtColor(mat1, mat, cv::COLOR_BGR2RGB);
//cvtColor(mat2, mat, cv::COLOR_BGR2RGB);
////cvtColor(mat, mat, cv::COLOR_RGB2BGR);
////cvtColor(mat, mat, cv::COLOR_BGR2RGB);
////cvtColor(mat, mat, cv::COLOR_RGB2BGR);
//qint64 dt = tmr.elapsed();

//QMessageBox::information(this, "", NUM_STR(dt) + "ms");

//***<<< 以上  5. cvtColor()耗时  >>>***



//***<<<  4. mat.copyTo()耗时  >>>***

//结果：1280*960 的图片copy一次2ms

//cv::Mat mat, mat1, mat2;
//mat = cv::imread("wgs.bmp");
//mat1 = cv::imread("1.jpg");
//mat2 = cv::imread("wgs.bmp");
//
//QElapsedTimer tmr;
//tmr.start();
//mat1.copyTo(mat);
//mat2.copyTo(mat);
//qint64 dt = tmr.elapsed();
//
//statusBar()->showMessage(QString::number(dt));

//***<<< 以上  4. mat.copyTo()耗时  >>>***



//***<<<  3. 测试QThread::usleep的准确度  >>>***

//结果：usleep(1)时，min ~ average ~ max 如下
//1.01 ms ~ 1.84 ms ~ 2.48ms

//msleep()个位数时都有一毫秒左右的误差

// QElapsedTimer tmr;
// quint64 tsum = 0, tmin = 10000000, tmax = 0;
// int n = 1000;
// for (int i = 0; i < n; i++){
// 	tmr.start();
// 	QThread::usleep(1);
// 	quint64 dt = tmr.nsecsElapsed();
// 
// 	if (tmin>dt) tmin = dt;
// 	if (tmax < dt) tmax = dt;
// 	tsum += dt;
// }
// QMessageBox::information(this, "",

//***<<< 以上  3. 测试QThread::usleep的准确度  >>>***



//***<<<  2. mat.elemSize的作用  >>>***

//返回每个像素占多少字节

// 	cv::Mat mat;
// 	mat = cv::imread("wgs.bmp");
// 	int n1 = mat.elemSize();
// 	cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
// 	int n2 = mat.elemSize();
// 	statusBar()->showMessage(QString().sprintf("bgr:%d, gray:%d", n1, n2)); //bgr:3, gray:1

//***<<< 以上  2. elemSize的作用  >>>***



//***<<<  1. 测试painter.drawImage()和setPixmap()的效率  >>>***

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

//***<<< 以上  1. 测试painter.drawImage()和setPixmap()的效率  >>>***

