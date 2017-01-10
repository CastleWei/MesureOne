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

//void ShowLabel::startRefreshing(CameraController *cam)
//{
//	this->cam = cam;
//	//int tmr = startTimer(40);
//}
//
//void ShowLabel::stopRefreshing()
//{
//	//killTimer(0);
//}

void ShowLabel::paintEvent(QPaintEvent * event)
{
	QLabel::paintEvent(event);
	//QPainter painter(this);
	//QImage img = cam->getImage();
	//QImage mImg = cam->getIntermediateImage();
	//if (!img.isNull() && mImg.isNull()){
	//	this->setMinimumSize(img.width() + mImg.width(), img.height());
	//	painter.drawImage(0, 0, img);
	//	painter.drawImage(img.width(), 0, mImg);
	//}
}

void ShowLabel::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton){
		QPoint pt = event->pos();
		vwdb::println(QString().sprintf("%d,%d", pt.x(), pt.y()));
	}
}

//void ShowLabel::timerEvent(QTimerEvent * event)
//{
//	update();
//}
