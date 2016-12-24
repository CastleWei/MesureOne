#ifndef SHOWWIDGET_H
#define SHOWWIDGET_H

#include <QLabel>
#include <QTimer>
#include <cameracontroller.h>

class ShowWidget : public QLabel
{
	Q_OBJECT

public:
	ShowWidget(QObject *parent);
	~ShowWidget();

	void startRefreshing(CameraController *cam);
	void stopRefreshing();

private:
	CameraController *cam;

	virtual void paintEvent(QPaintEvent * event);
	virtual void timerEvent(QTimerEvent * event);
};

#endif // SHOWWIDGET_H
