//*******************************
//���ļ���¼����Ŀ�����е�ʵ�⾭��
//*******************************


//***<<<  0. ��ʽ  >>>***

//����

//***<<< ����  0. ��ʽ  >>>***



//***<<<  2. mat.elemSize������  >>>***

// 	cv::Mat mat;
// 	mat = cv::imread("wgs.bmp");
// 	int n1 = mat.elemSize();
// 	cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
// 	int n2 = mat.elemSize();
// 	statusBar()->showMessage(QString().sprintf("bgr:%d, gray:%d", n1, n2));

//***<<< ����  2. elemSize������  >>>***



//***<<<  1. ����painter.drawImage()��setPixmap()��Ч��  >>>***

/*
��λms		1��ͼ	4��ͼ
RGB		pix		12		48
draw	31		126
GRAY	pix		39		158
draw	17		70

���ۣ�	painter.drwaImage()�Ҷ�ͼ�죻
setPixmap() RGB�죻
�������
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

//***<<< ����  1. ����painter.drawImage()��setPixmap()��Ч��  >>>***

