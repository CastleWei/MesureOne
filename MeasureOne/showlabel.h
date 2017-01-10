#ifndef SHOWWIDGET_H
#define SHOWWIDGET_H

#include <QLabel>
#include <QTimer>
#include <cameracontroller.h>

class ShowLabel : public QLabel
{
	Q_OBJECT

public:
	ShowLabel(QObject *parent);
	~ShowLabel();

	//void startRefreshing(CameraController *cam);
	//void stopRefreshing();

private:
	//CameraController *cam;

	virtual void paintEvent(QPaintEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	//virtual void timerEvent(QTimerEvent * event);
};

#endif // SHOWWIDGET_H
