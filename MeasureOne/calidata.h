#pragma once

#include <QPointF>

class CaliData
{


public:
	float dx = 0.32f, dy = 0.16f, dz = 0.16f;

	CaliData()
	{
	}

	~CaliData()
	{
	}

	//标定一条光刀
	QPointF caliLine(int i, float off){
		return QPointF(i*dx, off*dz);
	}
};

