/****************************************************************************

Creator: liyangyao@gmail.com
Date: 2015/8/18

****************************************************************************/

#ifndef UVPP_PIPE_H
#define UVPP_PIPE_H

#include "uvpp_define.h"
#include "uvpp_handle.h"
#include "uvpp_loop.h"
#include "uvpp_stream.h"

namespace uvpp{

class Pipe: public Stream<uv_pipe_t>
{
public:
    explicit Pipe(Loop *loop, bool ipc):
        Stream<uv_pipe_t>()
    {
        uv_pipe_init(loop->handle(), handle(), ipc);
    }

    ~Pipe()
    {

    }

    //Bind to the specified Pipe name.
    int bind(const char *name)
    {
        return uv_pipe_bind(handle(), name);
    }


    //Start connecion to the remote Pipe.
    bool connect(const char *name, const ConnectCallback &onConnect)
    {
        m_onConnect = onConnect;
        uv_connect_t *req = new uv_connect_t;
        req->data = this;

        uv_pipe_connect(req, handle(), name, pipe_connect_cb);
        return true;
    }


private:
    DISABLE_COPY(Pipe)
    static void pipe_connect_cb(uv_connect_t* req, int status)
    {
        bool connected = status==0;
        Pipe *_this = reinterpret_cast<Pipe *>(req->data);
        if (_this->m_onConnect)
        {
            _this->m_onConnect(connected);
        }
        delete req;
    }
    ConnectCallback m_onConnect;
};
}

#endif // UVPP_PIPE_H