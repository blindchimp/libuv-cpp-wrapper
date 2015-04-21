/****************************************************************************

Creator: liyangyao@gmail.com
Date: 2015/4/20

****************************************************************************/

#ifndef UVQT_H
#define UVQT_H

#include <QObject>
#include <memory>
#include <functional>
#include <libuv/include/uv.h>
#include <QDebug>

using std::shared_ptr;
using std::unique_ptr;
using std::function;

namespace{
typedef function<void()> Callback;
typedef function<void(bool connected)> ConnectCallback;
}

#endif // UVQT_H
