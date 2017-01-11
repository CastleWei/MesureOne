#include "measurelabel.h"
#include "vwdb.h"

#include <QMouseEvent>
#include <QPainter>
#include <QBrush>
#include <QMutexLocker>

MeasureLabel::MeasureLabel(QObject *parent)
: QLabel((QWidget*)parent)
{

}

MeasureLabel::~MeasureLabel()
{

}

void MeasureLabel::init(ImageObject *imgObj, CaliData *cali)
{
	this->imgObj = imgObj;
	this->cali = cali;
}

void MeasureLabel::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton){
		QPoint pt = event->pos();
		vwdb::println(QString().sprintf("%d,%d", pt.x(), pt.y()));
		double x, y;
		x = pt.x() / factor;
		y = (this->height() - pt.y()) / factor;
		QString str = CN("该点坐标为: (%1 mm, %2 mm)").arg(x).arg(y);
		vwdb::showstat(str);
		vwdb::msgbox(str);
	}
}

void MeasureLabel::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.fillRect(painter.window(), Qt::black);
	painter.setPen(QPen(Qt::green, 1));

	if (cali == nullptr || imgObj == nullptr || imgObj->offs == nullptr) return;

	//**** TODO: bug: 动态采集时，切换到测量页会崩溃，一开始就在测量页不会，预计是showlbl刷新时的问题

	QMutexLocker locker(&imgObj->mutex);

	int w = this->width();
	int h = this->height();
	int size = imgObj->offsLen;
	QPointF pe = cali->caliLine(size - 1, imgObj->offs[size - 1]); //最后一个点
	factor = w / pe.x(); //比例

	//此法略慢，7~8ms
	//QPointF p0(0.0f, 0.0f); //连线时的上一点
	//for (int i = 0; i < size; i++){
	//	QPointF p = cali->caliLine(i, imgObj->offs[i]); //标定后的坐标
	//	p *= dx; //缩放到和界面等宽
	//	p.setY(h - p.y()); //以下面为基线，上下翻转
	//	painter.drawLine(p0, p);
	//	p0 = p;
	//}

	//此法较快，4~6ms
	QPointF *vec = new QPointF[size];
	for (int i = 0; i < size; i++){
		QPointF &p = vec[i];
		p = cali->caliLine(i, imgObj->offs[i]); //标定后的坐标
		p *= factor; //缩放到和界面等宽
		p.setY(h - p.y()); //以下面为基线，上下翻转
	}
	painter.drawPolyline(vec, size);
	delete[] vec;
}
