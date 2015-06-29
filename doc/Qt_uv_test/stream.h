/****************************************************************************

Creator: liyangyao@gmail.com
Date: 2015/4/20

****************************************************************************/

#ifndef STREAM_H
#define STREAM_H

#include "uvpp.h"
#include "loop.h"
#include "handle.h"

namespace uv{

struct WriteRequest
{
public:
    explicit WriteRequest(const QByteArray &ba, const RequestCallback &cb):
         m_byteArray(ba),
         m_writeCallback(cb)
    {
        m_buf.base = m_byteArray.data();
        m_buf.len = m_byteArray.length();
        m_req.data = this;
    }

    uv_write_t *get()
    {
        return &m_req;
    }

    const uv_buf_t *buf()
    {
        return &m_buf;
    }

    const RequestCallback& writeCallback()
    {
        return m_writeCallback;
    }

private:
    uv_write_t m_req;
    uv_buf_t m_buf;
    QByteArray m_byteArray;
    RequestCallback m_writeCallback;
    DISABLE_COPY(WriteRequest)
};

template <typename HANDLE_T>
class Stream: public Handle<HANDLE_T>
{
public:
    explicit Stream():
        Handle<HANDLE_T>()
    {
    }

    int accept(Stream *client)
    {
        uv_accept(get<uv_stream_t>(), client->get<uv_stream_t>());
    }

    int read_start()
    {
        return uv_read_start(get<uv_stream_t>(), on_alloc_cb, on_read_cb);
    }

    int read_stop()
    {
        return uv_read_stop(get<uv_stream_t>());
    }

    int write(const QByteArray &ba, const RequestCallback &cb)
    {
        WriteRequest *wr = new WriteRequest(ba, cb);
        return uv_write(wr->get(), get<uv_stream_t>(), wr->buf(), 1, on_write_cb);
    }

    void readyRead(QByteArray data)
    {
        qDebug()<<"readyRead:"<<data;
        shutdown();
    }

    int shutdown()
    {
        uv_shutdown_t* req = new uv_shutdown_t;
        return uv_shutdown(req, get<uv_stream_t>(), on_shutdown_cb);
    }

    int listen()
    {
        //todo:: callback
        return uv_listen(get<uv_stream_t>(), 128, on_connection_cb);
    }

private:
    static void on_write_cb(uv_write_t* req, int status)
    {
        WriteRequest *wr = reinterpret_cast<WriteRequest *>(req->data);
        if (wr->writeCallback())
        {
            wr->writeCallback()(status);
        }
        delete wr;
    }

    static void on_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
    {
        static char BUFFER[65535];
        buf->base = BUFFER;
        buf->len = 65535;
    }

    static void on_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
    {
        //nread is > 0 if there is data available, 0 if libuv is done reading for now, or < 0 on error.
        qDebug()<<"nread="<<nread;
        Stream *_this = reinterpret_cast<Stream *>(stream->data);
        if (nread>0)
        {
            QByteArray ba(buf->base, nread);
            _this->readyRead(ba);
        }
        else if (nread<0)
        {
            _this->close();
        }
    }

    static void on_shutdown_cb(uv_shutdown_t* req, int status)
    {
        delete req;
        qDebug()<<"shutdown status="<<status;
    }

    static void on_connection_cb(uv_stream_t* server, int status)
    {

    }
    DISABLE_COPY(Stream)

};
}

#endif // STREAM_H