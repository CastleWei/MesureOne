#include "showlabel.h"
#include "vwdb.h"
#include <QPainter>
#include <QLabel>
#include <QMouseEvent>

ShowLabel::ShowLabel(QObject *parent)
	: QLabel((QWidget*)parent)
{
}

ShowLabel::~ShowLabel()
{

}

void ShowLabel::update(ImageObject *imgObj)
{
	this->imgObj = imgObj;
	needUpdate = true;
	QLabel::update();
}

void ShowLabel::showImg(ImageObject &imgObj)
{
	QMutexLocker locker(&imgObj.mutex);
	this->setPixmap(QPixmap::fromImage(getQimg(imgObj)));
}

//******！！！！！必须在mutex保护下使用
QImage ShowLabel::getQimg(ImageObject &imgObj)
{
	cv::Mat mat;

	if (!imgObj.dst.empty())
		mat = imgObj.dst;
	else if (!imgObj.img.empty())
		mat = imgObj.img;
	else
		mat = imgObj.src;

	int n = mat.elemSize(); //每点的字节数
	QImage::Format format;
	if (n == 3)
		format = QImage::Format_RGB888;
	else if (n == 1)
		format = QImage::Format_Grayscale8;
	else{
		vwdb::println("qimage format error, elemsize=");
		return QImage();
	}

	return QImage(mat.data, mat.cols, mat.rows, mat.cols*n, format);

}

void ShowLabel::paintEvent(QPaintEvent * event)
{
	if (imgObj != nullptr && needUpdate){
		showImg(*imgObj);
		needUpdate = false;
	}
	QLabel::paintEvent(event);
}
