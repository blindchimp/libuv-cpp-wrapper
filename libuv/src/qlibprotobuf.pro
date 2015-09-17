TEMPLATE = lib
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ./src

CONFIG += staticlib





HEADERS += $$PWD/heap-inl.h
HEADERS += $$PWD/queue.h
HEADERS += $$PWD/uv-common.h
SOURCES += $$PWD/fs-poll.c
SOURCES += $$PWD/inet.c
SOURCES += $$PWD/threadpool.c
SOURCES += $$PWD/uv-common.c
SOURCES += $$PWD/version.c

HEADERS += $$PWD/win/atomicops-inl.h
HEADERS += $$PWD/win/handle-inl.h
HEADERS += $$PWD/win/internal.h
HEADERS += $$PWD/win/req-inl.h
HEADERS += $$PWD/win/stream-inl.h
HEADERS += $$PWD/win/winapi.h
HEADERS += $$PWD/win/winsock.h
SOURCES += $$PWD/win/async.c
SOURCES += $$PWD/win/core.c
SOURCES += $$PWD/win/dl.c
SOURCES += $$PWD/win/error.c
SOURCES += $$PWD/win/fs-event.c
SOURCES += $$PWD/win/fs.c
SOURCES += $$PWD/win/getaddrinfo.c
SOURCES += $$PWD/win/getnameinfo.c
SOURCES += $$PWD/win/handle.c
SOURCES += $$PWD/win/loop-watcher.c
SOURCES += $$PWD/win/pipe.c
SOURCES += $$PWD/win/poll.c
SOURCES += $$PWD/win/process-stdio.c
SOURCES += $$PWD/win/process.c
SOURCES += $$PWD/win/req.c
SOURCES += $$PWD/win/signal.c
SOURCES += $$PWD/win/stream.c
SOURCES += $$PWD/win/tcp.c
SOURCES += $$PWD/win/thread.c
SOURCES += $$PWD/win/timer.c
SOURCES += $$PWD/win/tty.c
SOURCES += $$PWD/win/udp.c
SOURCES += $$PWD/win/util.c
SOURCES += $$PWD/win/winapi.c
SOURCES += $$PWD/win/winsock.c


HEADERS += $$PWD/unix/atomic-ops.h
HEADERS += $$PWD/unix/internal.h
HEADERS += $$PWD/unix/linux-syscalls.h
HEADERS += $$PWD/unix/spinlock.h
SOURCES += $$PWD/unix/aix.c
SOURCES += $$PWD/unix/android-ifaddrs.c
SOURCES += $$PWD/unix/async.c
SOURCES += $$PWD/unix/core.c
SOURCES += $$PWD/unix/darwin-proctitle.c
SOURCES += $$PWD/unix/darwin.c
SOURCES += $$PWD/unix/dl.c
SOURCES += $$PWD/unix/freebsd.c
SOURCES += $$PWD/unix/fs.c
SOURCES += $$PWD/unix/fsevents.c
SOURCES += $$PWD/unix/getaddrinfo.c
SOURCES += $$PWD/unix/getnameinfo.c
SOURCES += $$PWD/unix/kqueue.c
SOURCES += $$PWD/unix/linux-core.c
SOURCES += $$PWD/unix/linux-inotify.c
SOURCES += $$PWD/unix/linux-syscalls.c
SOURCES += $$PWD/unix/loop-watcher.c
SOURCES += $$PWD/unix/loop.c
SOURCES += $$PWD/unix/netbsd.c
SOURCES += $$PWD/unix/openbsd.c
SOURCES += $$PWD/unix/pipe.c
SOURCES += $$PWD/unix/poll.c
SOURCES += $$PWD/unix/process.c
SOURCES += $$PWD/unix/proctitle.c
SOURCES += $$PWD/unix/pthread-fixes.c
SOURCES += $$PWD/unix/signal.c
SOURCES += $$PWD/unix/stream.c
SOURCES += $$PWD/unix/sunos.c
SOURCES += $$PWD/unix/tcp.c
SOURCES += $$PWD/unix/thread.c
SOURCES += $$PWD/unix/timer.c
SOURCES += $$PWD/unix/tty.c
SOURCES += $$PWD/unix/udp.c

