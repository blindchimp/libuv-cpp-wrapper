/****************************************************************************

Creator: liyangyao@gmail.com
Date: 2015/8/17

****************************************************************************/

#ifndef UVPP_STREAM_H
#define UVPP_STREAM_H

#include "uvpp_define.h"
#include "uvpp_handle.h"
#include "uvpp_loop.h"

namespace uvpp{
template <typename HANDLE_T>
class Stream: public Handle<HANDLE_T>
{
public:
    explicit Stream():
        Handle<HANDLE_T>()
    {
        handle()->data = this;
    }

    bool listen(const NewConnectionCallback &onNewConnection)
    {
        m_onNewConnection = onNewConnection;
        return uv_listen(handle<uv_stream_t>(), 128, stream_connection_cb)==0;
    }

    bool accept(Stream *client)
    {
        return uv_accept(handle<uv_stream_t>(), client->handle<uv_stream_t>())>=0;
    }

    //Start read data from the connected endpoint.
    int read_start(const ReadCallback &onRead)
    {
        m_onRead = onRead;
        return uv_read_start(handle<uv_stream_t>(), stream_alloc_cb, stream_read_cb);
    }

    bool read_stop()
    {
        return uv_read_stop(handle<uv_stream_t>())==0;
    }

    struct stream_write_ctx
    {
        uv_write_t req;
        QByteArray buffer;
        WriteCallback callback;
    };

    //Write data on the stream
    bool write(const QByteArray &data, const WriteCallback &cb = nullptr)
    {
        stream_write_ctx *req = new stream_write_ctx;
        req->buffer = data;
        req->callback = cb;
        uv_buf_t buf;
        buf = uv_buf_init(req->buffer.data(), req->buffer.size());

        int err = uv_write(&req->req, handle<uv_stream_t>(), &buf, 1, stream_write_cb);
        if (err<0)
        {
            delete req;
            return false;
        }
        return true;
    }

    //Shutdown the write side of this Stream.
    int shutdown()
    {
        uv_shutdown_t* req = new uv_shutdown_t;
        int err =  uv_shutdown(req, handle<uv_stream_t>(), stream_shutdown_cb);
        if (err<0)
        {
            delete req;
            return false;
        }
        return true;
    }

    //Indicates if stream is readable.
    bool readable()
    {
        return uv_is_readable(handle<uv_stream_t>());
    }

    //Indicates if stream is writable.
    bool writable()
    {
        return uv_is_writable(handle<uv_stream_t>());
    }   

    void onStreamClosed(const StreamClosedCallback &cb)
    {
        m_onStreamClosed = cb;
    }

private:
    DISABLE_COPY(Stream)
    static void stream_write_cb(uv_write_t* req, int status)
    {
        stream_write_ctx *ctx = (stream_write_ctx *)req;
        if (ctx->callback)
        {
            ctx->callback(status);
        }
        delete ctx;
    }

    static void stream_alloc_cb(uv_handle_t* handle, size_t /*suggested_size*/, uv_buf_t* buf)
    {
        Loop* loop = (Loop *)handle->loop->data;

        if (loop->buffer.in_use)
        {
            buf->base = nullptr;
            buf->len = 0;
        }
        else{
            buf->base = loop->buffer.slab;
            buf->len = sizeof loop->buffer.slab;
            loop->buffer.in_use = true;
        }
    }

    static void stream_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
    {
        //nread is > 0 if there is data available, 0 if libuv is done reading for now, or < 0 on error.
        Stream *_this = reinterpret_cast<Stream *>(stream->data);
        if (nread>0)
        {
            QByteArray ba(buf->base, nread);
            if (_this->m_onRead)
            {
                _this->m_onRead(ba);
            }
        }
        else if (nread<0)
        {
            _this->read_stop();
            if (_this->m_onStreamClosed)
            {
                _this->m_onStreamClosed();
            }
        }
        Loop* loop = (Loop *)stream->loop->data;
        loop->buffer.in_use = false;
    }

    static void stream_shutdown_cb(uv_shutdown_t* req, int /*status*/)
    {
        delete req;
    }

    static void stream_connection_cb(uv_stream_t* server, int /*status*/)
    {
        Stream *_this = reinterpret_cast<Stream *>(server->data);
        if (_this->m_onNewConnection)
        {
            _this->m_onNewConnection();
        }
    }

    NewConnectionCallback m_onNewConnection;
    StreamClosedCallback m_onStreamClosed;
    ReadCallback m_onRead;
};
}

#endif // UVPP_STREAM_H