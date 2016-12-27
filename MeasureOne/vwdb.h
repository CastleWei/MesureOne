//自定义调试类

#ifndef VWDB_H
#define VWDB_H

#include <QString>

namespace vwdb{
	bool initdb(void *_out);
	bool println(QString text);
}

#endif // VWDB_H

