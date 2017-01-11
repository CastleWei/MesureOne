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
	void update(ImageObject *imgObj);

private:
	bool needUpdate = false;
	ImageObject *imgObj = nullptr;

	void showImg(ImageObject &imgObj);
	QImage getQimg(ImageObject &imgObj);

	virtual void paintEvent(QPaintEvent * event);
};

#endif // SHOWWIDGET_H
