//自定义的公用内容

#ifndef COMMON_H
#define COMMON_H

#include <QString>

////中文字符串
#define CN(str) QString::fromLocal8Bit(str)
////数字转字符串
#define NUM_STR(str) QString::number(str)

#endif // COMMON_H
