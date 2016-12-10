#include "showwidget.h"
#include <QPainter>
#include <QDebug>
#include <QTimerEvent>

ShowWidget::ShowWidget(QObject *parent)
	: QWidget((QWidget*)parent)
{

}

ShowWidget::~ShowWidget()
{

}

void ShowWidget::startRefreshing(CameraController *cam)
{
	this->cam = cam;
	iTmrRefresh = startTimer(100);
}

void ShowWidget::stopRefreshing()
{
	killTimer(iTmrRefresh);
}

void ShowWidget::paintEvent(QPaintEvent * event)
{
	if (cam == NULL) return;

	if (cam->isReady){
		QPainter painter(this);
		QImage img = cam->getImage();
		//QImage mImg = cam->getIntermediateImage();

		this->setMinimumSize(img.width()/* + mImg.width()*/, img.height());
		painter.drawImage(0, 0, img);
		//painter.drawImage(img.width(), 0, mImg);
	}
}

void ShowWidget::timerEvent(QTimerEvent * event)
{
	if (event->timerId() == iTmrRefresh){
		update();
	}
}
