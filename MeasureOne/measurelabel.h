#ifndef MEASURELABEL_H
#define MEASURELABEL_H

#include <QLabel>
#include "imageprocess.h"
#include "calidata.h"

class MeasureLabel : public QLabel
{
	Q_OBJECT

public:
	MeasureLabel(QObject *parent);
	~MeasureLabel();

	void init(ImageObject *imgObj, CaliData *cali);

private:
	ImageObject *imgObj = nullptr;
	CaliData *cali = nullptr;
	float factor = 1.0f; //缩放比例，每毫米对应多少像素

	virtual void mousePressEvent(QMouseEvent * event);
	virtual void paintEvent(QPaintEvent * event);
};

#endif // MEASURELABEL_H
