#include "showwidget.h"
#include <QPainter>
#include <QLabel>

ShowWidget::ShowWidget(QObject *parent)
	: QLabel((QWidget*)parent)
{
}

ShowWidget::~ShowWidget()
{

}

void ShowWidget::startRefreshing(CameraController *cam)
{
	this->cam = cam;
	//int tmr = startTimer(40);
}

void ShowWidget::stopRefreshing()
{
	//killTimer(0);
}

void ShowWidget::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	//QImage img = cam->getImage();
	//QImage mImg = cam->getIntermediateImage();
	//if (!img.isNull() && mImg.isNull()){
	//	this->setMinimumSize(img.width() + mImg.width(), img.height());
	//	painter.drawImage(0, 0, img);
	//	painter.drawImage(img.width(), 0, mImg);
	//}
}

void ShowWidget::timerEvent(QTimerEvent * event)
{
	update();
}
